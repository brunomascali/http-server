# C++ HTTP Server

A modern thread-safe toy HTTP server written in C++

## Features

- Thread-safe request handling
- Builder pattern for easy server configuration
- Type-safe state management
- Simple routing system
- Support for common HTTP methods (GET, POST, PUT, DELETE)

## Building

```bash
# Create and enter build directory
mkdir build
cd build

# Configure and build
cmake ..
make

# Or use the build script
../build.sh
```

## Usage

The server can be started with an optional port number (defaults to 8080):

```bash
./HttpServer [port]
```

### Example Code

```cpp
#include <server.hpp>
#include <router.hpp>
#include <http.hpp>
#include <json/json.hpp>

int main() {
    using State = int;  // Define your state type

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
```
### State Management

The server is templated on a state type, allowing you to use any type as the server's state:

```cpp
// Using a simple integer
using State = int;

// Using a custom struct
struct AppState {
    int counter;
    std::string name;
    std::vector<std::string> items;
};
using State = AppState;
```

The state is thread-safe and shared between all worker threads. Access to the state is protected by mutexes.

## License

MIT License