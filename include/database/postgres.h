#pragma once
#include "player_record.h"
#include <pqxx/connection>
#include <pqxx/transaction>

namespace postgres {

const size_t DEFAULT_OFFSET = 0;
const size_t DEFAULT_LIMIT  = 100;

class PlayerRecordRepositoryImpl : public PlayerRecordRepository {
public:
    explicit PlayerRecordRepositoryImpl(pqxx::connection& connection)
        : connection_(connection){};

    void SaveRetired(const PlayerRecord& retired) override;
    std::vector<PlayerRecord> GetRecordsTable(size_t offset = DEFAULT_OFFSET, size_t limit = DEFAULT_LIMIT) override;

private:
    pqxx::connection& connection_;
};

class Database {
public:
    explicit Database(pqxx::connection connection);
    void CreateTable();

private:
    pqxx::connection connection_;
};

}  // namespace postgres