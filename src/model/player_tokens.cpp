#include "model/player_tokens.h"
#include <sstream>
#include "request_exception.h"

namespace model{

Token PlayerTokens::AddPlayer(const PlayerPtr& player) {
    std::stringstream buf;

    auto first_part = generator1_();
    auto second_part = generator2_();
    buf << std::hex << first_part << second_part;

    std::string token_string = buf.str();

    if(token_string.length() < TOKEN_LENGTH) {
        token_string.insert(0, std::string(TOKEN_LENGTH - token_string.length(), '0'));
    }

    Token token = Token(std::move(token_string));

    tokenToPlayer_.emplace(token, player->GetId());
    tokens_.emplace(player->GetId(), token);
    players_[player->GetId()] = player;
    return token;
}

void PlayerTokens::AddPlayer(const PlayerPtr& player, const std::string& token) {
    tokenToPlayer_.emplace(token, player->GetId());
    tokens_.emplace(player->GetId(), token);
    players_[player->GetId()] = player;
}

const PlayerPtr& PlayerTokens::FindPlayerByToken(const Token& token) const {
    if ((*token).length() != TOKEN_LENGTH) {
        throw RequestException ("invalidToken", "Authorization header is missing");
    }

    if (tokenToPlayer_.contains(token)) {
        auto id = tokenToPlayer_.at(token);
        return players_.at(id);
    } else {
        throw RequestException("unknownToken", "Player token has not been found");
    }
}

const PlayerPtr& PlayerTokens::FindPlayerById(size_t id){
    if (!tokens_.contains(id)) {
        throw std::invalid_argument("Invalid player id!");
    }
    auto token = tokens_.at(id);
    return FindPlayerByToken(token);
}

const Token& PlayerTokens::FindTokenByPlayerId(size_t id){
    if (!tokens_.contains(id)) {
        throw std::invalid_argument("Invalid player id!");
    }
    return tokens_.at(id);
}

const std::map<size_t, Token>& PlayerTokens::GetTokens() const {
    return tokens_;
}

void PlayerTokens::RemovePlayer(size_t player_id) {
    auto player = players_[player_id];
    player->GetSession()->RemoveDog(player->GetDog()->GetId());
    tokenToPlayer_.erase(tokens_.at(player_id));
    tokens_.erase(player->GetId());
    players_.erase(player->GetId());
}

}