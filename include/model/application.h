#pragma once
#include <memory>
#include "model/game.h"
#include "ticker.h"
#include <boost/beast/core.hpp>
#include <boost/asio/strand.hpp>

namespace model{

namespace net = boost::asio;
using Strand = net::strand<net::io_context::executor_type>;

class Application : public std::enable_shared_from_this<Application>{
public:
    Application(Game& game, Strand& api_strand, const command_line::Args& args)
            : game_(game)
            , api_strand_(api_strand)
            , args_(args){
    }

    Application(const Application& other) = delete;
    Application(Application&& other) = delete;
    Application& operator = (const Application& other) = delete;
    Application& operator = (Application&& other) = delete;

    Game& GetGame();
    void StartTicker();
    void ForceSave();
    void SaveGameState(const std::chrono::milliseconds& delta_time);
    void RestoreGame();

private:
    void SaveGame();

    Game& game_;
    Strand& api_strand_;
    const command_line::Args& args_;
    std::shared_ptr<util::Ticker> update_game_state_ticker_;
    std::shared_ptr<util::Ticker> generate_loot_ticker_;
};

}

