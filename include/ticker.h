#pragma once

#include <chrono>
#include <utility>
#include <memory>
#include <boost/asio.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>

namespace util{

namespace net = boost::asio;
namespace sys = boost::system;

class Ticker : public std::enable_shared_from_this<Ticker> {
public:
    using Strand = net::strand<net::io_context::executor_type>;
    using Handler = std::function<void(std::chrono::milliseconds delta)>;

    Ticker(Strand& strand, std::chrono::milliseconds period, Handler handler);
    void Start();

private:
    void ScheduleTick();
    void OnTick(sys::error_code ec);

    Strand& strand_;
    net::steady_timer timer_;
    std::chrono::milliseconds period_;
    Handler handler_;
    std::chrono::steady_clock::time_point last_tick_;
};

}   //namespace util