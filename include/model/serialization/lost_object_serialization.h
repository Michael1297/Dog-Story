#pragma once
#include "model/loot.h"
#include "model_serialization.h"
#include <boost/serialization/vector.hpp>

namespace serialization{

// LootObjectRepr (LootObjectRepresentation) - сериализованное представление класса LootObject
class LootObjectRepr{
public:
    LootObjectRepr() = default;
    LootObjectRepr(const model::LootObject& lost_object)
            : id_(lost_object.GetId())
            , position_(lost_object.GetPosition())
            , loot_type_(lost_object.GetLootType()){
    }

    [[nodiscard]] model::LootObject Restore() const {
        model::LootObject lost_object(id_, position_, loot_type_);
        return lost_object;
    }

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version){
        ar& id_;
        ar& loot_type_;
        ar& position_;
    }

private:
    size_t id_;
    model::Position position_;
    size_t loot_type_;
};

}  // namespace serialization