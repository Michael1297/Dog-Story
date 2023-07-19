#include "database/postgres.h"
#include <boost/format.hpp>
#include <pqxx/pqxx>
#include <pqxx/zview.hxx>

namespace postgres {

using namespace std::literals;
using pqxx::operator"" _zv;

void PlayerRecordRepositoryImpl::SaveRetired(const PlayerRecord& retired) {
    pqxx::work work_{connection_};
    work_.exec_params(R"(
            INSERT INTO retired_players (id, name, score, play_time_ms) VALUES ($1, $2, $3, $4);
            )"_zv,
                      retired.GetUUID(),
                      retired.GetName(),
                      retired.GetScore(),
                      retired.GetPlayTime());
    work_.commit();
}

std::vector<PlayerRecord> PlayerRecordRepositoryImpl::GetRecordsTable(size_t offset, size_t limit) {
    std::vector<PlayerRecord> records_table;
    pqxx::read_transaction read_transaction_{connection_};
    auto query_text = boost::format("SELECT id, name, score, play_time_ms FROM retired_players ORDER BY score DESC, play_time_ms LIMIT %1% OFFSET %2%;") % limit % offset;
    for (auto [uuid, name, score, play_time] : read_transaction_.query<std::string, std::string, size_t, size_t>(query_text.str())) {
        records_table.emplace_back(uuid, name, score, play_time);
    }
    return records_table;
}

Database::Database(pqxx::connection connection)
        : connection_{std::move(connection)}{
}

void Database::CreateTable(){
    pqxx::work work_{connection_};
    work_.exec(R"(
CREATE TABLE IF NOT EXISTS retired_players (
    id UUID PRIMARY KEY,
    name varchar(100) NOT NULL,
    score integer NOT NULL,
    play_time_ms integer NOT NULL
);
CREATE INDEX IF NOT EXISTS score_time_name_idx ON retired_players (score DESC, play_time_ms, name);
)"_zv);
    // коммитим изменения
    work_.commit();
}

}  // namespace postgres