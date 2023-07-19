#pragma once

#include <filesystem>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

namespace http_handler{

namespace fs = std::filesystem;
namespace sys = boost::system;
namespace beast = boost::beast;
namespace http = beast::http;
using StringRequest = http::request<http::string_body>;     // Запрос, тело которого представлено в виде строки
using StringResponse = http::response<http::string_body>;   // Ответ, тело которого представлено в виде строки
using FileResponse = http::response<http::file_body>;
using EmptyResponse = http::response<http::empty_body>;
using FileRequestResult = std::variant<EmptyResponse, StringResponse, FileResponse>;


class FileHandler{
public:
    FileHandler(fs::path static_root) : static_root_(std::move(static_root)) {}
    FileRequestResult HandleFileRequest(const StringRequest& req);

private:
    fs::path static_root_;
    StringResponse MakeErrorResponse(http::status status, const StringRequest& request, const std::string& message);
    std::string_view file_type(const fs::path& path);
    bool isSubPath(fs::path path, fs::path base);
    std::string url_decode(std::string_view url);
};

}

