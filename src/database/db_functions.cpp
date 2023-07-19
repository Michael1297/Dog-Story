#include "database/db_functions.h"
#include "database/connection_pool.h"
#include "database/postgres.h"
#include "database/tagged_uuid.h"

using namespace std::literals;

namespace db {

DbConnectionSettings GetConfigFromEnv() {
    return {ConnectionPoolSingleton::GetConfigFromEnv()};
}

void SaveRetiredPlayer(const std::string& player_name, size_t score, size_t play_time){
    struct PlayerTag {};
    using PlayerId = util::TaggedUUID<PlayerTag>;

    PlayerRecord record{PlayerId::New().ToString(), player_name, score, play_time};

    auto inst = ConnectionPoolSingleton::GetInstance();
    auto conn_pool = inst->GetPool();
    auto conn = conn_pool->GetConnection();
    postgres::PlayerRecordRepositoryImpl rep{*conn};
    rep.SaveRetired(record);
}

std::vector<PlayerRecord> GetRetiredPlayers(int start, int max_items){
    auto inst = ConnectionPoolSingleton::GetInstance();
    auto conn_pool = inst->GetPool();
    auto conn = conn_pool->GetConnection();
    postgres::PlayerRecordRepositoryImpl rep{*conn};
    return rep.GetRecordsTable(start, max_items);
}

}