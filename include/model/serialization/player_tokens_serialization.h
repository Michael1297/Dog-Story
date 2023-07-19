#pragma once
#include "model/player_tokens.h"
#include "player_serialization.h"
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>

namespace serialization {

// PlayerTokensRepr (PlayerTokensRepresentation) - сериализованное представление класса PlayerTokens
class PlayerTokensRepr{
public:
    PlayerTokensRepr() = default;

    explicit PlayerTokensRepr(const model::PlayerTokens& player_tokens){
        SerializePlayers(player_tokens.GetPlayers());
        SerializeTokens(player_tokens.GetTokens());
    }

    [[nodiscard]] const auto& GetPlayersSerialize() const{ return players_; }
    [[nodiscard]] const auto& GetTokens() const{ return tokens_; }

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version){
        ar& players_;
        ar& tokens_;
    }

private:
    void SerializePlayers(const std::map<size_t, model::PlayerPtr>& players){
        for(auto& [id, player] : players) {
            players_.emplace(id, *player);
        }
    }

    void SerializeTokens(const std::map<size_t, model::Token>& tokens){
        for(auto& [id, token] : tokens) {
            tokens_.emplace(id, *token);
        }
    }

    std::map<size_t, PlayerRepr> players_;
    std::map<size_t, std::string> tokens_;
};

}