#include "ticker.h"

namespace util{

Ticker::Ticker(Strand& strand, std::chrono::milliseconds period, Handler handler)
        : strand_(strand)
        , timer_(strand)
        , period_(period)
        , handler_(std::move(handler)) {
}

void Ticker::Start() {
    last_tick_ = std::chrono::steady_clock::now();
    /* Выполнить SchedulTick внутри strand_ */
    return net::dispatch(strand_, [self = this->shared_from_this()] {
         assert(self->strand_.running_in_this_thread());
        self->ScheduleTick();
    });
}

void Ticker::ScheduleTick() {
    /* выполнить OnTick через промежуток времени period_ */
    timer_.expires_from_now(period_);
    timer_.async_wait(net::bind_executor(strand_, [self = shared_from_this()](sys::error_code ec) {
    self->OnTick(ec);
    }));
}

void Ticker::OnTick(sys::error_code ec) {
    if(ec) {
        return;
    }
    auto current_tick = std::chrono::steady_clock::now();
    handler_(std::chrono::duration_cast<std::chrono::milliseconds>(current_tick - last_tick_));
    last_tick_ = current_tick;
    ScheduleTick();
}

}