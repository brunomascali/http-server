#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

class Socket
{
    int m_socket_fd;

public:
    Socket() : m_socket_fd(-1) {}
    Socket(int socket_fd) : m_socket_fd(socket_fd) {}

    bool is_valid() const { return m_socket_fd != -1; }

    int get_socket_fd() const { return m_socket_fd; }

    void bind(const sockaddr_in &address)
    {
        if (::bind(m_socket_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
        {
            throw std::runtime_error("Failed to bind socket");
        }
    }

    void listen(int backlog)
    {
        if (::listen(m_socket_fd, backlog) < 0)
        {
            throw std::runtime_error("Failed to listen on socket");
        }
    }

    Socket accept(sockaddr_in &client_address)
    {
        socklen_t client_len = sizeof(client_address);
        return Socket(::accept(m_socket_fd, (struct sockaddr *)&client_address, &client_len));
    }
    
    Http::Request recv_http_request() {
        std::string buffer_str;
        char buffer[1024];
        
        ssize_t bytes_read;
        while ((bytes_read = recv(m_socket_fd, buffer, sizeof(buffer), 0)) > 0)
        {
            buffer_str.append(buffer, bytes_read);
            if (static_cast<size_t>(bytes_read) < sizeof(buffer)) {
                break;
            }
        }
        
        if (bytes_read < 0)
        {
            std::cerr << "Error reading from socket: " << std::strerror(errno) << std::endl;
            throw std::runtime_error("Error reading from socket");
        }
        
        if (buffer_str.empty())
        {
            throw std::runtime_error("Client disconnected without sending data");
        }
        
        return Http::Request(buffer_str);
    }

    void send(const std::string &message)
    {
        ssize_t bytes_sent = ::send(m_socket_fd, message.c_str(), message.size(), 0);
        if (bytes_sent < 0)
        {
            throw std::runtime_error("Failed to send message");
        }
    }

    void close()
    {
        if (::close(m_socket_fd) < 0)
        {
            throw std::runtime_error("Failed to close socket");
        }
    }
};