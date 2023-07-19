#include "model/game.h"
#include <stdexcept>
#include "request_exception.h"
#include "database/db_functions.h"

namespace model{

using namespace std::literals;

void Game::AddMap(Map map) {
    const size_t index = maps_.size();
    if (auto [it, inserted] = map_id_to_index_.emplace(map.GetId(), index); !inserted) {
        throw std::invalid_argument("Map with id "s + *map.GetId() + " already exists"s);
    } else {
        try {
            maps_.emplace_back(std::move(map));
        } catch (...) {
            map_id_to_index_.erase(it);
            throw;
        }
    }
}

const Game::Maps& Game::GetMaps() const noexcept {
    return maps_;
}

const Map* Game::FindMap(const Map::Id& id) const noexcept {
    if (auto it = map_id_to_index_.find(id); it != map_id_to_index_.end()) {
        return &maps_.at(it->second);
    }
    return nullptr;
}

Token Game::JoinGame(const std::string& name, const Map::Id& map_id, bool random_spawn){
    auto map = FindMap(map_id);
    if (!map) {
        throw RequestException("invalidArgumentMap", "Map not find");
    }

    if (!name_to_id_.contains(name)) {
        auto session = FindSession(map_id);
        auto dog = session->AddDog(name, random_spawn);
        auto player = std::make_shared<Player>(name, session, dog);
        name_to_id_[name] = player->GetId();
        map_id_to_player_id_[map_id].insert(player->GetId());
        return player_tokens_.AddPlayer(player);
    } else {
        return player_tokens_.FindTokenByPlayerId(name_to_id_.at(name));
    }
}

void Game::LootGeneratorConfig(const double& period, const double& probability) {
    period_loot_gen_ = period;
    probability_loot_gen_ = probability;
}

const PlayerPtr& Game::FindPlayerByToken(const Token& token){
    return player_tokens_.FindPlayerByToken(token);
}

const PlayerPtr& Game::FindPlayerById(size_t id){
    return player_tokens_.FindPlayerById(id);
}

Game::SessionPtr Game::FindSession(const Map::Id& id){
    if (!map_id_to_session_.contains(id)) {
        if (auto map = FindMap(id)) {
            std::chrono::milliseconds ms(static_cast<uint64_t>(period_loot_gen_ * MILLISECONDS_IN_SECOND));
            map_id_to_session_[id] = std::make_shared<GameSession>(const_cast<Map&>(*map), loot_gen::LootGenerator(ms , probability_loot_gen_));
        } else {
            throw RequestException("invalidArgumentMap", "Map not find");
        }
    }
    return map_id_to_session_.at(id);
}

const std::unordered_set<size_t>& Game::GetPlayersOnMap(const Map::Id& id){
    if (!map_id_to_player_id_.contains(id)) {
        throw RequestException("invalidArgumentMap", "Map not find");
    }
    return map_id_to_player_id_.at(id);
}

double Game::GetLootGenPeriod() const {
    return period_loot_gen_;
}

void Game::UpdateTimeInSessions(std::chrono::milliseconds period) {
    assert(args_.period);

    for (auto& [map, session] : map_id_to_session_) {
        session->UpdateTime(period);
    }
}

void Game::GenerateLootInSessions(std::chrono::milliseconds period) {
    for (auto& [map, session] : map_id_to_session_) {
        session->GenerateLoot(period);
    }
}

void Game::RestoreGame(const SessiosRepr& sessions_serialization, const TokensRepr& player_tokens_serialization) {
    //восстановить лут в сессии
    for(auto& repr_session : sessions_serialization){
        auto session = FindSession(repr_session.RestoreMapId());
        for(const auto& loot : repr_session.GetLostObjectsSerialize()){
            auto loot_ptr = std::make_shared<LootObject>(loot.Restore());
            session->AddLoot(loot_ptr);
        }
    }

    const auto& repr_players = player_tokens_serialization.GetPlayersSerialize();
    const auto& repr_tokens = player_tokens_serialization.GetTokens();

    //восстановить игроков
    for(const auto& [id, repr_player] : repr_players){
        const auto& repr_token = repr_tokens.at(id);        //получить восстановленный токен игрока
        auto session = FindSession(repr_player.GetMapId());
        auto dog = repr_player.GetDogSerialize().Restore(); //восстановить собаку
        session->AddDog(dog);                               //добавить собаку на карту

        // восстановить игрока
        auto player = std::make_shared<Player>(repr_player.GetId(), repr_player.GetName(), session, dog);
        name_to_id_[player->GetName()] = player->GetId();
        map_id_to_player_id_[repr_player.GetMapId()].insert(player->GetId());
        player_tokens_.AddPlayer(player, repr_token);
    }
}

void Game::RemoveAfkPlayers() {
    std::vector<PlayerPtr> afk_players;
    afk_players.reserve(GetPlayers().size());

    for(auto& [id, player] : GetPlayers()){
        if(player->GetDog()->IsAfk()) afk_players.push_back(player);
    }

    for(auto& afk_player : afk_players){
        auto afk_dog = afk_player->GetDog();
        db::SaveRetiredPlayer(afk_dog->GetName(), afk_dog->GetScore(), afk_dog->GetLiveTime().count());
        player_tokens_.RemovePlayer(afk_player->GetId());
        map_id_to_player_id_[afk_player->GetMapId()].erase(afk_player->GetId());
        name_to_id_.erase(afk_player->GetName());
    }
}

}