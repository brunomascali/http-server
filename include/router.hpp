#pragma once

#include <unordered_map>
#include <string>
#include <functional>
#include <http.hpp>

struct pair_hash {
    template <class T1, class T2>
    std::size_t operator() (const std::pair<T1, T2> &pair) const {
        return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
    }
};

template<typename State>
class Router {
    using Handler = std::function<Http::Response(Http::Request, State&)>;
    std::unordered_map<std::pair<Http::RequestMethod, std::string>, Handler, pair_hash> m_routes;

public:
    Router() = default;

    void add_route(const Http::RequestMethod& method, const std::string& path, const Handler& handler) {
        m_routes[std::make_pair(method, path)] = handler;
    }
    
    Http::Response handle_request(const Http::Request& request, State& state) const {
        auto it = m_routes.find(std::make_pair(request.get_method(), request.get_url()));
        if (it != m_routes.end()) {
            return it->second(request, state);
        }
        return Http::Response(404, std::string("Not Found"));
    }
};