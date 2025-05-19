// #include "router.hpp"

// template <typename State>
// void Router<State>::add_route(const RequestMethod &method, const std::string &path, const Handler &handler)
// {
//     m_routes[std::make_pair(method, path)] = handler;
// }

// template <typename State>
// Response Router<State>::handle_request(const Request &request, State &state)
// {
//     auto it = m_routes.find(std::make_pair(request.m_method, request.m_url));
//     if (it != m_routes.end()) {
//         return it->second(request, state);
//     }
//     return Response(404, "Not Found");
// }
