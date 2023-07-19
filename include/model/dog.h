#pragma once

#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include "loot.h"
#include "model_base.h"
#include "physics/collision_detector.h"

namespace model{

class Dog {
    inline static size_t next_id = 0;
    inline static std::chrono::seconds max_inactive_time_{ONE_MINUTE_IN_SECONDS};

public:
    using BagContent = std::vector<std::shared_ptr<LootObject>>;
    using LootTypes = std::vector<LootType>;

    Dog(size_t id, std::string name, const Position& position, size_t bag_capacity);
    Dog(std::string name, const Position& position, size_t bag_capacity);

    const size_t& GetId() const;
    const std::string& GetName() const;
    const Position& GetPosition() const;
    const Speed& GetSpeed() const;
    const std::string& GetDirection() const;
    const BagContent& GetBag() const;
    const size_t& GetBagCapacity() const;
    const size_t& GetScore() const;
    const std::chrono::milliseconds& GetLiveTime();
    const collision_detector::Gatherer& GetGatherer();
    static double GetAbsSpeed(const Speed& speed);
    static double GetDistance(const Position& pos1, const Position& pos2);
    static void SetMaxInactiveTime(size_t max_inactive_time_in_seconds);
    void SetPosition(const Position& position);
    void SetSpeed(const Speed& speed);
    void SetSpeed(const Speed& speed, const std::chrono::milliseconds& time_delta);
    void SetDirection(const std::string& direction);
    void SetDirection(const std::string& direction, double map_speed);
    Position CalculateNewPosition(const std::chrono::milliseconds& delta_time) const;
    void CollectLostObject(const std::shared_ptr<LootObject>& lost_obj);
    void DropLostObjects(const LootTypes& loot_types);
    void AddScore(size_t value);
    bool IsFullBag();
    bool IsEmptyBag();
    bool IsAfk();

private:
    size_t id_;
    std::string name_;
    Position position_;
    Speed speed_;
    std::string direction_ = "U";
    BagContent bag_;
    size_t score_ = 0;
    size_t bag_capacity_ = 0;
    std::chrono::milliseconds afk_time_{0};
    std::chrono::milliseconds live_time_{0};
    collision_detector::Gatherer gatherer_{{0.0, 0.0}, {0.0, 0.0}, DOG_WIDTH};
};

}