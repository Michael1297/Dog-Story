#pragma once
#include "model/game.h"
#include "game_session_serialization.h"
#include "player_tokens_serialization.h"
#include <boost/serialization/vector.hpp>

namespace serialization {

// GameRepr (GameRepresentation) - сериализованное представление класса Game
class GameRepr{
public:
    GameRepr() = default;

    explicit GameRepr(const model::Game& game)
            : player_tokens_(game.GetPlayerTokens()) {
        SerializeSessions(game.GetMapIdToSession());
    }

    [[nodiscard]] const auto& GetPlayerTokensSerialize() const { return player_tokens_; }
    [[nodiscard]] const auto& GetGameSessionsSerialize() const { return game_sessions_; }

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version){
        ar& player_tokens_;
        ar& game_sessions_;
    }

private:
    using MapIdHasher = util::TaggedHasher<model::Map::Id>;
    using MapIdToSession = std::unordered_map<model::Map::Id, model::SessionPtr, MapIdHasher>;

    void SerializeSessions(const MapIdToSession& map_id_to_session){
        game_sessions_.reserve(map_id_to_session.size());
        for(auto& [map_id, session] : map_id_to_session) {
            game_sessions_.emplace_back(*session);
        }
    }

    PlayerTokensRepr player_tokens_;
    std::vector<GameSessionRepr> game_sessions_;
};

}

