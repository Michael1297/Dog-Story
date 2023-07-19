#pragma once

#include <vector>
#include <string>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include "map.h"
#include "game_session.h"
#include "player_tokens.h"
#include "command_parse.h"
#include "player.h"
#include "ticker.h"
#include "serialization/game_session_serialization.h"
#include "serialization/player_tokens_serialization.h"

namespace model{

class Game{
public:
    using Maps = std::vector<Map>;
    using SessionPtr = std::shared_ptr<GameSession>;
    using SessiosRepr = std::vector<serialization::GameSessionRepr>;
    using TokensRepr = serialization::PlayerTokensRepr;
    using MapIdHasher = util::TaggedHasher<Map::Id>;
    using MapIdToIndex = std::unordered_map<Map::Id, size_t, MapIdHasher>;
    using MapIdToPlayers = std::unordered_map<Map::Id, std::unordered_set<size_t>, MapIdHasher>;
    using MapIdToSession = std::unordered_map<Map::Id, SessionPtr, MapIdHasher>;

    Game(const command_line::Args& args) : args_(args) { }

    void AddMap(Map map);
    const Maps& GetMaps() const noexcept;
    const Map* FindMap(const Map::Id& id) const noexcept;
    Token JoinGame(const std::string& name, const Map::Id& map_id, bool random_spawn);
    void LootGeneratorConfig(const double& period, const double& probability);
    const PlayerPtr& FindPlayerByToken(const Token& token);
    const PlayerPtr& FindPlayerById(size_t id);
    SessionPtr FindSession(const Map::Id& id);
    const std::unordered_set<size_t>& GetPlayersOnMap(const Map::Id& id);
    const auto& GetPlayers() const { return player_tokens_.GetPlayers(); }
    const auto& GetPlayerTokens() const{ return player_tokens_; }
    const auto& GetMapIdToSession() const { return map_id_to_session_; }
    double GetLootGenPeriod() const;
    void UpdateTimeInSessions(std::chrono::milliseconds period);
    void GenerateLootInSessions(std::chrono::milliseconds period);
    void RestoreGame(const SessiosRepr& sessions_serialization, const TokensRepr& player_tokens_serialization);
    void RemoveAfkPlayers();

private:
    std::vector<Map> maps_;
    std::unordered_map<std::string, size_t> name_to_id_;

    MapIdToIndex map_id_to_index_;
    MapIdToPlayers map_id_to_player_id_;
    MapIdToSession map_id_to_session_;
    PlayerTokens player_tokens_;
    const command_line::Args& args_;
    double period_loot_gen_ = 0.0;
    double probability_loot_gen_ = 0.0;
};

}