#pragma once

#include <random>
#include <unordered_map>
#include <functional>
#include <memory>
#include <map>
#include "tagged.h"
#include "player.h"

namespace model{

namespace detail {
struct TokenTag {};
}  // namespace detail

using Token = util::Tagged<std::string, detail::TokenTag>;
using PlayerPtr = std::shared_ptr<Player>;
static std::random_device random_device;

class PlayerTokens {
public:

    Token AddPlayer(const PlayerPtr& player);
    void AddPlayer(const PlayerPtr& player, const std::string& token);
    const PlayerPtr& FindPlayerByToken(const Token& token) const;
    const PlayerPtr& FindPlayerById(size_t id);
    const Token& FindTokenByPlayerId(size_t id);
    const auto& GetPlayers() const { return players_; }
    const std::map<size_t, Token>& GetTokens() const;
    void RemovePlayer(size_t player_id);

private:
    std::unordered_map<Token, size_t, util::TaggedHasher<Token>> tokenToPlayer_;
    std::map<size_t, PlayerPtr> players_;
    std::map<size_t, Token> tokens_;

    std::mt19937_64 generator1_{[this] {
        std::uniform_int_distribution<std::mt19937_64::result_type> dist;
        return dist(random_device);
    }()};

    std::mt19937_64 generator2_{[this] {
        std::uniform_int_distribution<std::mt19937_64::result_type> dist;
        return dist(random_device);
    }()};
};

}