#include "file_handler.h"
#include <unordered_map>
#include <utility>
#include <regex>
#include <boost/url.hpp>

namespace http_handler{

namespace url = boost::urls;

StringResponse FileHandler::MakeErrorResponse(http::status status, const StringRequest& request, const std::string& message) {
    StringResponse response {status, request.version()};
    response.set(http::field::content_type, "text/plain");
    response.keep_alive(request.keep_alive());
    response.body() = message;
    response.prepare_payload();
    return response;
}

FileRequestResult FileHandler::HandleFileRequest(const StringRequest &req) {
    if( req.method() != http::verb::get && req.method() != http::verb::head){
        return MakeErrorResponse(http::status::bad_request, req, "Method not allowed");
    }

    if(req.target().empty() || req.target()[0] != '/'){
        return MakeErrorResponse(http::status::bad_request, req, "Bad request");
    }

    std::string decoded_path = url_decode(req.target());

    if (decoded_path.back() == '/') {
        decoded_path = decoded_path.append("index.html");
    }

    auto file_path = fs::weakly_canonical(static_root_ / decoded_path.substr(1));

    if (file_path.generic_string().back() == '/') {
        file_path = file_path / "index.html";
    }

    if (!isSubPath(file_path, static_root_)){
        return MakeErrorResponse(http::status::bad_request, req, "Incorrent path");
    }

    http::file_body::value_type file_body;
    sys::error_code ec;
    file_body.open(file_path.string().c_str(), beast::file_mode::scan, ec);

    if (ec == sys::errc::no_such_file_or_directory){
        return MakeErrorResponse(http::status::not_found, req, "File not found");
    }

    if (ec) {
        return MakeErrorResponse(http::status::internal_server_error, req, ec.message());
    }

    const auto size = file_body.size();

    if (req.method() == http::verb::head){
        EmptyResponse response {http::status::ok, req.version()};
        response.set(http::field::content_type, file_type(file_path));
        response.content_length(size);
        response.keep_alive(req.keep_alive());
        return (response);
    }

    FileResponse response {
            std::piecewise_construct,
            std::make_tuple(std::move(file_body)),
            std::make_tuple(http::status::ok, req.version())
    };

    response.set(http::field::content_type, file_type(file_path));
    response.content_length(size);
    response.keep_alive(req.keep_alive());
    return response;
}

std::string_view FileHandler::file_type(const fs::path& path){
    if (!path.has_extension()){
        return "application/octet-stream";
    }
    auto ext = path.extension().string();
    std::for_each(ext.begin(), ext.end(), [](char& symbol){
        symbol = tolower(symbol);
    });

    static const std::unordered_map<std::string, std::string_view> extension {
            {".htm",    "text/html"},
            {".html",   "text/html"},
            {".php",    "text/html"},
            {".css",    "text/css"},
            {".txt",    "text/plain"},
            {".js",     "application/javascript"},
            {".json",   "application/json"},
            {".xml",    "application/xml"},
            {".swf",    "application/x-shockwave-flash"},
            {".flv",    "video/x-flv"},
            {".png",    "image/png"},
            {".jpe",    "image/jpeg"},
            {".jpeg",   "image/jpeg"},
            {".jpg",    "image/jpeg"},
            {".gif",    "image/gif"},
            {".bmp",    "image/bmp"},
            {".ico",    "image/vnd.microsoft.icon"},
            {".tiff",   "image/tiff"},
            {".tif",    "image/tiff"},
            {".svg",    "image/svg+xml"},
            {".svgz",   "image/svg+xml"}
    };

    try{
        return extension.at(ext);
    }
    catch (...) {
        return "application/octet-stream";
    }
}


    // Возвращает true, если каталог path содержится внутри base_path.
bool FileHandler::isSubPath(fs::path path, fs::path base) {
    // Приводим оба пути к каноничному виду (без . и ..)
    path = fs::weakly_canonical(path);
    base = fs::weakly_canonical(base);

    // Проверяем, что все компоненты base содержатся внутри path
    for (auto b = base.begin(), p = path.begin(); b != base.end(); ++b, ++p) {
        if (p == path.end() || *p != *b) {
            return false;
        }
    }
    return true;
}

std::string FileHandler::url_decode(const std::string_view url) {
    url::decode_view decoded_target(url);
    std::string decoded_path (decoded_target.begin(), decoded_target.end());
    decoded_path = std::regex_replace(decoded_path, std::regex("\\+"), " ");
    return decoded_path;
}

}