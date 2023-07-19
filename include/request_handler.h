#pragma once
#include <filesystem>
#include <memory>
#include "command_parse.h"
#include "server/http_server.h"
#include "api_handler.h"
#include "file_handler.h"
#include "model/map.h"


namespace http_handler {

namespace net = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;
namespace fs = std::filesystem;
using Strand = net::strand<net::io_context::executor_type>;


class RequestHandler: public std::enable_shared_from_this<RequestHandler> {
public:
    explicit RequestHandler(const AppPtr& app, Strand& api_strand, const command_line::Args& args)
            : api_strand_(api_strand){
        api_handler = std::make_unique<ApiHandler>(app, args);
        file_handler = std::make_unique<FileHandler>(args.source_dir);
    }

    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        // Обработать запрос request и отправить ответ, используя send
        if(ApiHandler::IsApiRequest(req)){
            auto handle = [self = this->shared_from_this(), req = std::forward<decltype(req)>(req), send = std::forward<decltype(send)>(send)]{
                send(self->api_handler->HandleApiRequest(req));
            };
            return net::dispatch(api_strand_, handle);
        } else{
            auto response = file_handler->HandleFileRequest(req);
            std::visit([&send](auto& v){send(v);}, response);
        }
    }

private:
    std::unique_ptr<ApiHandler> api_handler;
    std::unique_ptr<FileHandler> file_handler;
    Strand& api_strand_;
};

}  // namespace http_handler
