#pragma once
#include <unordered_map>
#include <memory>
#include <optional>
#include "physics/collision_detector.h"
#include "loot.h"
#include "dog.h"

namespace model {

class ItemDogProvider: public collision_detector::ItemGathererProvider {
public:
    using Item = std::shared_ptr<collision_detector::Item>;
    using Items = std::vector<Item>;
    using Dogs = std::vector< std::shared_ptr<Dog> >;

    ItemDogProvider(Items&& items, Dogs&& dogs)
        : items_(std::move(items))
        , dogs_(std::move(dogs)) {};
    virtual ~ItemDogProvider() = default;

    size_t ItemsCount() const override{
        return items_.size();
    }
    collision_detector::Item GetItem(size_t idx) const override{
        return *items_[idx];
    }
    size_t GatherersCount() const override{
        return dogs_.size();
    }
    collision_detector::Gatherer GetGatherer(size_t idx) const override{
        return dogs_[idx]->GetGatherer();
    }

    const auto& GetDogId(size_t idx) const{
        return dogs_[idx]->GetId();
    }

    template<typename T>
    const T* const TryCastItemTo(size_t idx) const {
        return dynamic_cast<T*>(items_[idx].get());
    };

private:
    Items items_;
    Dogs dogs_;
};

}