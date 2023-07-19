#pragma once
#include "model/player.h"
#include "dog_serialization.h"
#include <boost/serialization/vector.hpp>

// Для восстановления игрока нужно сохранить ник, токен, ид карты, данные собаки

namespace serialization {

// PlayerRepr (PlayerRepresentation) - сериализованное представление класса Player
class PlayerRepr{
public:
    PlayerRepr() = default;

    explicit PlayerRepr(const model::Player& player)
            : id_(player.GetId())
            , name_(player.GetName())
            , map_id_(*player.GetMapId())
            , dog_(*player.GetDog()){
    }

    [[nodiscard]] size_t GetId() const { return id_; }
    [[nodiscard]] const std::string& GetName() const { return name_; }
    [[nodiscard]] auto GetMapId() const { return model::Map::Id(map_id_); }
    [[nodiscard]] const auto& GetDogSerialize() const { return dog_; }

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version){
        ar& id_;
        ar& name_;
        ar& map_id_;
        ar& dog_;
    }

private:
    size_t id_ = 0;
    std::string name_;
    std::string map_id_;
    DogRepr dog_;
};

}