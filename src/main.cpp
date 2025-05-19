#include <server.hpp>
#include <router.hpp>
#include <json/json.hpp>
#include <http.hpp>

int main(int argc, char *argv[])
{
    int port = 8080;
    if (argc > 1)
    {
        port = atoi(argv[1]);
    }

    using State = int;
    Server<State>()
        .port(port)
        .backlog(20)
        .threads(2)
        .add_route(Http::RequestMethod::GET, "/counter",
                   [](Http::Request request, State &state)
                   {
                       nlohmann::json json;
                       json["counter"] = state;
                       return Http::Response(200, json);
                   })
        .add_route(Http::RequestMethod::POST, "/increment",
                   [](Http::Request request, State &state)
                   {
                       state++;
                       nlohmann::json json;
                       json["counter"] = state;
                       return Http::Response(200, json);
                   })
        .add_route(Http::RequestMethod::POST, "/decrement",
                   [](Http::Request request, State &state)
                   {
                       state--;
                       auto json = nlohmann::json::object();
                       json["counter"] = state;
                       return Http::Response(200, json);
                   })
        .build()
        .run();
}
