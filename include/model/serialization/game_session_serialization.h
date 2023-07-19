#pragma once
#include "model/game_session.h"
#include "dog_serialization.h"
#include "lost_object_serialization.h"
#include <boost/serialization/vector.hpp>

namespace serialization {

// GameSessionRepr (GameSessionRepresentation) - сериализованное представление класса GameSession
class GameSessionRepr{
    using LostObjects = std::map<size_t,model::LootPtr>;

public:
    GameSessionRepr() = default;

    explicit GameSessionRepr(const model::GameSession& game_session)
            : map_id_(*game_session.GetMapId()){
        SerializeLostObjects(game_session.GetLostObjects());
    }

    [[nodiscard]] model::Map::Id RestoreMapId() const{
        return model::Map::Id(map_id_);
    }

    [[nodiscard]] const auto& GetLostObjectsSerialize() const { return lost_objects_; }

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar& map_id_;
        ar& lost_objects_;
    }

private:
    void SerializeLostObjects(const LostObjects& lost_objects){
        lost_objects_.reserve(lost_objects.size());
        for(auto& [id, object] : lost_objects) {
            lost_objects_.emplace_back(*object);
        }
    }

    std::string map_id_;
    std::vector<LootObjectRepr> lost_objects_;
};

}   //namespace serialization