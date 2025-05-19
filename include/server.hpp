#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <router.hpp>
#include <iostream>
#include <cerrno>
#include <cstring>
#include <thread>
#include <vector>
#include <http.hpp>
#include <socket.hpp>

struct Request
{
    Http::Request m_request;
    Socket m_client_socket;

    Request() : m_request(), m_client_socket() {}
    Request(Http::Request request, Socket client_socket)
        : m_request(request), m_client_socket(client_socket)
    {
    }
};

struct NoState
{
};

template <typename State>
class Server
{
private:
    Socket m_server_socket;
    uint16_t m_port = 8080;
    uint16_t m_backlog = 20;
    sockaddr_in m_server_address;

    Router<State> m_router;
    State m_state = State();

    std::mutex m_state_mutex;
    std::mutex m_requests_mutex;
    std::queue<Request> m_requests_queue;
    std::condition_variable m_request_condvar;

    uint8_t num_threads = 1;
    std::vector<std::thread> m_workers;
    bool m_running = false;

    void launch_worker_threads(int num_threads)
    {
        std::cout << "Launching " << num_threads << " num_threads\n";
        m_running = true;
        m_workers.resize(num_threads);

        for (std::thread &thread : m_workers)
        {
            if (num_threads > 1)
                thread = std::thread(&Server<State>::handle_request_multi_thread, this);
            else
                thread = std::thread(&Server<State>::handle_request_single_thread, this);
        }
    }

public:
    Server<State>() = default;
    ~Server<State>()
    {
        m_running = false;
        m_request_condvar.notify_all();
        for (auto &worker : m_workers)
        {
            if (worker.joinable())
            {
                worker.join();
            }
        }
        m_server_socket.close();
    }

    Server<State> &port(int port)
    {
        this->m_port = port;
        return *this;
    }

    Server<State> &backlog(int backlog)
    {
        this->m_backlog = backlog;
        return *this;
    }

    Server<State> &router(Router<State> router)
    {
        this->m_router = router;
        return *this;
    }

    Server<State> &threads(uint8_t num_threads)
    {
        this->num_threads = num_threads;
        return *this;
    }

    Server<State> &state(State state)
    {
        this->m_state = state;
        return *this;
    }

    // using Handler = std::function<Http::Response(Http::Request, State&)>;
    Server<State> &add_route(const Http::RequestMethod &method, const std::string &path,
                             const std::function<Http::Response(Http::Request, State&)> &handler)
    {
        this->m_router.add_route(method, path, handler);
        return *this;
    }

    Server<State> &build()
    {
        m_server_socket = Socket(socket(AF_INET, SOCK_STREAM, 0));
        if (!m_server_socket.is_valid())
        {
            std::cerr << "Socket creation failed: " << std::strerror(errno) << std::endl;
            throw std::runtime_error("Failed to create socket");
        }

        m_server_address.sin_family = AF_INET;
        m_server_address.sin_addr.s_addr = INADDR_ANY;
        m_server_address.sin_port = htons(m_port);

        try
        {
            m_server_socket.bind(m_server_address);
            m_server_socket.listen(m_backlog);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error: " << e.what() << std::endl;
            throw std::runtime_error("Failed to bind or listen on socket");
        }

        std::cout << "Bind and listen successful" << std::endl;
        return *this;
    }

    void run()
    {
        if (num_threads == 0)
            throw std::invalid_argument("At least 1 thread is required to run the server");

        launch_worker_threads(num_threads);

        while (true)
        {
            try
            {
                Socket client_socket = m_server_socket.accept(m_server_address);
                Http::Request request = client_socket.recv_http_request();

                if (num_threads > 1)
                {
                    std::unique_lock<std::mutex> lk(m_requests_mutex);
                    m_requests_queue.push(Request(request, client_socket));
                    m_request_condvar.notify_one();
                }
                else
                {
                    m_requests_queue.push(Request(request, client_socket));
                }
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error: " << e.what() << std::endl;
                continue;
            }
        }
    }

    void handle_request_single_thread()
    {
        while (m_running)
        {
            if (m_requests_queue.empty())
                continue;

            Request request = m_requests_queue.front();
            m_requests_queue.pop();
            Http::Response response = m_router.handle_request(request.m_request, m_state);
            request.m_client_socket.send(response.to_string());
            request.m_client_socket.close();
        }
    }

    void handle_request_multi_thread()
    {
        while (m_running)
        {
            Request request{};
            Http::Response response{};
            {
                std::unique_lock<std::mutex> lk(m_requests_mutex);
                m_request_condvar.wait(lk, [this]()
                                       { return !m_running || !m_requests_queue.empty(); });
                request = m_requests_queue.front();
                m_requests_queue.pop();
            }

            {
                std::unique_lock<std::mutex> lk(m_state_mutex);
                response = m_router.handle_request(request.m_request, m_state);
            }

            request.m_client_socket.send(response.to_string());
            request.m_client_socket.close();
        }
    }
};