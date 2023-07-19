#pragma once
#include <memory>
#include "model/dog.h"
#include "lost_object_serialization.h"
#include <boost/serialization/vector.hpp>

namespace serialization {

// DogRepr (DogRepresentation) - сериализованное представление класса Dog
class DogRepr {
public:
    DogRepr() = default;

    explicit DogRepr(const model::Dog& dog)
        : id_(dog.GetId())
        , name_(dog.GetName())
        , pos_(dog.GetPosition())
        , bag_capacity_(dog.GetBagCapacity())
        , speed_(dog.GetSpeed())
        , direction_(dog.GetDirection())
        , score_(dog.GetScore()){
        SerializeLoot(dog.GetBag());
    }

    [[nodiscard]] std::shared_ptr<model::Dog> Restore() const {
        auto dog = std::make_shared<model::Dog>(id_, name_, pos_, bag_capacity_);
        dog->SetSpeed(speed_);
        dog->SetDirection(direction_);
        dog->AddScore(score_);
        for (const auto& item : bag_) {
            dog->CollectLostObject(std::make_shared<model::LootObject>(item.Restore()));
        }
        return dog;
    }

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar& id_;
        ar& name_;
        ar& pos_;
        ar& bag_capacity_;
        ar& speed_;
        ar& direction_;
        ar& score_;
        ar& bag_;
    }

private:
    void SerializeLoot(const model::Dog::BagContent& bag){
        bag_.reserve(bag.size());
        for(auto& lost_object : bag) {
            bag_.emplace_back(*lost_object);
        }
    }

    size_t id_ = 0;
    std::string name_;
    geom::Point2D pos_;
    size_t bag_capacity_ = 0;
    geom::Vec2D speed_;
    std::string direction_ = "U";
    size_t score_ = 0;
    std::vector<LootObjectRepr> bag_;
};

}  // namespace serialization