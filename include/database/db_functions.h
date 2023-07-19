#pragma once
#include <string>
#include "player_record.h"

namespace db {

struct DbConnectionSettings {
    std::string db_url;
};

DbConnectionSettings GetConfigFromEnv();

void SaveRetiredPlayer(const std::string& player_name, size_t score, size_t play_time);

std::vector<PlayerRecord> GetRetiredPlayers(int start, int max_items);

}