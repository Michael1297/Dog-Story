#pragma once

#include <cstdint>
#include <chrono>
#include <utility>
#include <map>
#include <vector>
#include "loot_generator.h"
#include "item_dog_provider.h"
#include "loot.h"
#include "map.h"
#include "dog.h"

namespace model{

using DogPtr = std::shared_ptr<Dog>;
using LootPtr = std::shared_ptr<LootObject>;

class GameSession {
public:

    GameSession(const GameSession& other) = default;
    GameSession(Map& map, loot_gen::LootGenerator gen);
    const auto& GetMapId() const { return map_.GetId(); }
    const auto& GetMap() const { return map_; }
    const auto& GetLostObjects() const { return lost_objects_; }

    DogPtr AddDog(const std::string& name, bool random_spawn);
    void AddDog(const DogPtr& dog);
    void AddLoot(const LootPtr& loot);
    void UpdateTime(std::chrono::milliseconds time_delta);
    void GenerateLoot(std::chrono::milliseconds time_delta);
    void RemoveDog(size_t dog_id);

private:
    void UpdateDogPosition(const DogPtr& dog, std::chrono::milliseconds time_delta);
    void HandleLoot();
    void CollectLoot(const ItemDogProvider &provider, size_t item_id, size_t gatherer_id);
    void DropLoot(const ItemDogProvider &provider, size_t item_id, size_t gatherer_id);

    std::map<size_t, DogPtr> dogs_;
    std::map<size_t ,LootPtr> lost_objects_;
    Map& map_;
    loot_gen::LootGenerator loot_generator_;
};

}