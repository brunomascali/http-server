#include <http.hpp>
#include <string>
#include <picohttpparser/picohttpparser.h>

Http::Request::Request(std::string request)
{
    const char *method, *path;
    int pret, minor_version;
    struct phr_header headers[100];
    size_t method_len, path_len, num_headers;

    num_headers = sizeof(headers) / sizeof(headers[0]);
    pret = phr_parse_request(request.c_str(), request.size(), &method, &method_len, &path, &path_len,
                             &minor_version, headers, &num_headers, 0);

    m_method = m_method_map.at(std::string(method, method_len));
    m_url = std::string(path, path_len);
    m_version = "HTTP/" + std::to_string(minor_version);

    for (size_t i = 0; i != num_headers; ++i)
    {
        m_headers[std::string(headers[i].name, headers[i].name_len)] = std::string(headers[i].value, headers[i].value_len);
    }

    const char *body_start = request.c_str() + pret;
    m_body = std::string(body_start);

    if (m_headers.count("Content-Length") > 0)
    {
        size_t content_length = std::stoul(m_headers["Content-Length"]);
        m_body = m_body.substr(0, content_length);
    }
}