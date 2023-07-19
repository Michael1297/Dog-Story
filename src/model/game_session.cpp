#include "model/game_session.h"
#include <cstdlib>
#include <ctime>

namespace model{

GameSession::GameSession(Map& map, loot_gen::LootGenerator gen)
        : map_(map)
        , loot_generator_(std::move(gen)){
    std::srand((unsigned)std::time(nullptr));
}

DogPtr GameSession::AddDog(const std::string& name, bool random_spawn) {
    auto position = random_spawn ? map_.GetRandomPosition() : Position();
    auto dog = std::make_shared<Dog>(name, position, map_.GetBagCapacity());
    dogs_[dog->GetId()] = dog;
    GenerateLoot(loot_generator_.getPeriod());
    return dog;
}

void GameSession::AddDog(const DogPtr& dog) {
    dogs_[dog->GetId()] = dog;
}

void GameSession::AddLoot(const LootPtr& loot) {
    lost_objects_[loot->GetId()] = loot;
}

void GameSession::UpdateTime(std::chrono::milliseconds time_delta) {
    for (auto& [id, dog] : dogs_) {
        UpdateDogPosition(dog, time_delta);
    }
    HandleLoot();
}

void GameSession::GenerateLoot(std::chrono::milliseconds time_delta) {
    const size_t max_loots = map_.GetLootTypes().size();
    if(max_loots == 0 || lost_objects_.size() >= dogs_.size()) {
        return;
    }

    const auto count = loot_generator_.Generate(time_delta, lost_objects_.size(), dogs_.size());

    for(size_t i = 0; i < count; ++i) {
        const auto loot_type =  std::rand() % max_loots;
        auto loot = std::make_shared<LootObject>(map_.GetRandomPosition(), loot_type);
        lost_objects_[loot->GetId()] = loot;
    }
}

void GameSession::RemoveDog(size_t dog_id) {
    dogs_.erase(dog_id);
}

void GameSession::UpdateDogPosition(const DogPtr& dog, std::chrono::milliseconds time_delta) {
    Speed speed_on_current_road, speed_on_other_road;

    Position pos_on_current_road = dog->GetPosition();
    Position pos_on_other_road = dog->GetPosition();

    Position result_pos = dog->CalculateNewPosition(time_delta);

    auto current_road = map_.FindRoadByPosition(dog->GetPosition());
    if (current_road.has_value()) {
        auto result_on_current_road = map_.CalculatePositionAndSpeedOnRoad(current_road.value(), result_pos,
                                                                           dog->GetSpeed());
        pos_on_current_road = result_on_current_road.first;
        speed_on_current_road = result_on_current_road.second;

        auto other_road = map_.FindRoadByPosition(result_pos, current_road.value().GetId());
        if (other_road.has_value()) {
            auto result_on_other_road = map_.CalculatePositionAndSpeedOnRoad(other_road.value(), result_pos,
                                                                             dog->GetSpeed());
            pos_on_other_road = result_on_other_road.first;
            speed_on_other_road = result_on_other_road.second;
        }
    }

    auto distance_on_current_road = Dog::GetDistance(pos_on_current_road, dog->GetPosition());
    auto distance_on_other_road = Dog::GetDistance(pos_on_other_road, dog->GetPosition());

    if (distance_on_current_road > distance_on_other_road || Dog::GetAbsSpeed(speed_on_current_road) > Dog::GetAbsSpeed(speed_on_other_road)) {
        dog->SetPosition(pos_on_current_road);
        dog->SetSpeed(speed_on_current_road, time_delta);
    } else {
        dog->SetPosition(pos_on_other_road);
        dog->SetSpeed(speed_on_other_road, time_delta);
    }
}

void GameSession::HandleLoot() {
    std::vector<DogPtr> dogs;
    dogs.reserve(dogs_.size());

    for(auto& [id, dog] : dogs_){
        dogs.push_back(dog);
    }

    std::vector<std::shared_ptr<collision_detector::Item>> items;
    items.reserve(lost_objects_.size());

    for(auto& [id, loot] : lost_objects_){
        items.push_back(loot);
    }

    for(auto& office : map_.GetOffices()){
        items.push_back(std::make_shared<model::Office>(office));
    }

    ItemDogProvider provider(std::move(items), std::move(dogs));
    auto collected_loot = collision_detector::FindGatherEvents(provider);
    if(collected_loot.empty()) {
        return;
    }

    for(auto& loot : collected_loot){
        CollectLoot(provider, loot.item_id, loot.gatherer_id);
        DropLoot(provider, loot.item_id, loot.gatherer_id);
    }
}

void GameSession::CollectLoot(const ItemDogProvider &provider, size_t item_id, size_t gatherer_id) {
    auto dog = dogs_[provider.GetDogId(gatherer_id)];
    const auto* const casted_lost_obj = provider.TryCastItemTo<LootObject>(item_id);
    if(casted_lost_obj == nullptr || dog->IsFullBag()) {
        return;
    }

    dog->CollectLostObject(lost_objects_[casted_lost_obj->GetId()]);
    lost_objects_.erase(casted_lost_obj->GetId());
}

void GameSession::DropLoot(const ItemDogProvider &provider, size_t item_id, size_t gatherer_id) {
    auto dog = dogs_[provider.GetDogId(gatherer_id)];
    const auto* const casted_office = provider.TryCastItemTo<Office>(item_id);
    if(casted_office == nullptr || dog->IsEmptyBag()) {
        return;
    }

    dog->DropLostObjects(map_.GetLootTypes());
}

}