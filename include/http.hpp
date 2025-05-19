#pragma once

#include <unordered_map>
#include <string>
#include <json/json.hpp>

namespace Http
{
    enum class RequestMethod
    {
        GET,
        POST,
        PUT,
        DELETE,
        OPTIONS,
        HEAD,
    };

    class Request
    {
        RequestMethod m_method;
        std::string m_url;
        std::string m_version;
        std::unordered_map<std::string, std::string> m_headers;
        static inline const std::unordered_map<std::string, RequestMethod> m_method_map = {
            {"GET", RequestMethod::GET},
            {"POST", RequestMethod::POST},
            {"PUT", RequestMethod::PUT},
            {"DELETE", RequestMethod::DELETE},
            {"OPTIONS", RequestMethod::OPTIONS},
            {"HEAD", RequestMethod::HEAD},
        };
        std::string m_body;

    public:

        Request() : m_method(RequestMethod::GET), m_url(""), m_version(""), m_headers(), m_body("") {}
        explicit Request(std::string request);

        RequestMethod get_method() const { return m_method; }
        std::string get_url() const { return m_url; }
        std::string get_version() const { return m_version; }
        std::unordered_map<std::string, std::string> get_headers() const { return m_headers; }
        std::string get_body() const { return m_body; }
    };

    class StatusLine
    {
        std::string m_version = "HTTP/1.1";
        int m_status_code = 200;
        static inline const std::unordered_map<int, std::string> m_reason_phrase_map = {
            {200, "OK"},
            {404, "Not Found"},
        };

    public:
        StatusLine() : m_status_code(200) {}
        StatusLine(int status_code) : m_status_code(status_code) {}

        std::string to_string() const
        {
            return m_version + " " + std::to_string(m_status_code) + " " + m_reason_phrase_map.at(m_status_code);
        }
    };

    class Response
    {
        StatusLine m_status_line;
        std::unordered_map<std::string, std::string> m_headers;
        std::string m_body;

    public:
        Response() : m_status_line(200) {}
        Response(int status_code, std::string body) : m_status_line(status_code), m_body(body) {
            m_headers["Content-Type"] = "text/html";
        }
        Response(int status_code, nlohmann::json body) : m_status_line(status_code), m_body(body.dump()) {
            m_headers["Content-Type"] = "application/json";
        }
        explicit Response(int status_code) : m_status_line(status_code) {}

        std::string to_string() const
        {
            std::string headers_str;
            for (const auto &header : m_headers)
            {
                headers_str += header.first + ": " + header.second + "\r\n";
            }
            headers_str += "Content-Length: " + std::to_string(m_body.size()) + "\r\n";
            return m_status_line.to_string() + "\r\n" + headers_str + "\r\n" + m_body;
        }
    };
}