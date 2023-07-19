#include "model/application.h"
#include <fstream>
#include <filesystem>
#include "model/serialization/game_serialization.h"
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

using namespace std::literals;
namespace fs = std::filesystem;

namespace model{

Game& Application::GetGame() {
    return game_;
}

void Application::StartTicker() {
    //тикер для обновления времени
    if(args_.period) {
        std::chrono::milliseconds period_of_update_game_state_ (args_.period.value());

        update_game_state_ticker_  = std::make_shared<util::Ticker>(api_strand_, period_of_update_game_state_,
                                                                    [&] (std::chrono::milliseconds time) {
                                                                        game_.UpdateTimeInSessions(time);
                                                                        game_.RemoveAfkPlayers();
                                                                        SaveGameState(time);
                                                                    });
        update_game_state_ticker_->Start();
    }

    //тикер для генерации лута
    std::chrono::milliseconds period_of_loot_generation(uint64_t(game_.GetLootGenPeriod() * MILLISECONDS_IN_SECOND));

    generate_loot_ticker_ = std::make_shared<util::Ticker>(api_strand_, period_of_loot_generation,
                                                           [&] (std::chrono::milliseconds time) {
                                                               game_.GenerateLootInSessions(time);
                                                           });
    generate_loot_ticker_->Start();
}

void Application::ForceSave() {
    return net::dispatch(api_strand_, [self = this->shared_from_this()] {
        assert(self->api_strand_.running_in_this_thread());
        self->SaveGame();
    });
}

void Application::RestoreGame() {
    if(!args_.state_file.has_value()) {
        return;
    }

    std::ifstream save_file(args_.state_file.value());
    if(!save_file.is_open()) {
        return;
    }
    boost::archive::text_iarchive ia{save_file};
    serialization::GameRepr game_repr;
    ia >> game_repr;
    game_.RestoreGame(game_repr.GetGameSessionsSerialize(), game_repr.GetPlayerTokensSerialize());
    save_file.close();
}

void Application::SaveGame() {
    if(!args_.state_file.has_value()) {
        return;
    }
    fs::path temp_file = args_.state_file.value().string() + ".temp"s;

    std::ofstream save_file(temp_file);
    boost::archive::text_oarchive oa{save_file};
    serialization::GameRepr game_repr(game_);
    oa << game_repr;
    save_file.close();

    fs::rename(temp_file, args_.state_file.value());
}

void Application::SaveGameState(const std::chrono::milliseconds& delta_time) {
    static int64_t period = args_.save_state_period.has_value() ? args_.save_state_period.value() : 0;
    if(!args_.save_state_period.has_value()) {
        return;
    }
    period -= delta_time.count();
    if(period <= 0) {
        SaveGame();
        period = args_.save_state_period.value();
    }
}

}

