#pragma once
#include <string>
#include <utility>
#include <vector>

class PlayerRecord {
   public:
    PlayerRecord(std::string uuid, std::string name, size_t score, size_t play_time)
        : uuid_(std::move(uuid))
        , name_(std::move(name))
        , score_(score)
        , play_time_(play_time){
    }

    const std::string& GetUUID() const noexcept {
        return uuid_;
    }

    const std::string& GetName() const noexcept {
        return name_;
    }

    size_t GetScore() const noexcept {
        return score_;
    }

    size_t GetPlayTime() const noexcept {
        return play_time_;
    }

   private:
    std::string uuid_;
    std::string name_;
    size_t score_ = 0;
    size_t play_time_ = 0;
};

class PlayerRecordRepository {
   public:
    virtual void SaveRetired(const PlayerRecord& retired) = 0;
    virtual std::vector<PlayerRecord> GetRecordsTable(size_t offset, size_t limit) = 0;

   protected:
    ~PlayerRecordRepository() = default;
};