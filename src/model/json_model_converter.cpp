#include "model/json_model_converter.h"
#include "model/json_key.h"
#include "model/loot.h"

namespace model {

json::array GetBuildings(const Map* map){
    json::array result;
    for(auto& building : map->GetBuildings()) {
        json::object building_json{
                {Key::X, building.getBounds().position.x},
                {Key::Y, building.getBounds().position.y},
                {Key::WIDTH, building.getBounds().size.width},
                {Key::HEIGHT, building.getBounds().size.height}
        };
        result.emplace_back(building_json);
    }
    return result;
}

json::array GetRoads(const Map* map){
    json::array result;
    for(auto& road : map->GetRoads()) {
        json::object road_json {
                {Key::X0, road.GetStart().x},
                {Key::Y0, road.GetStart().y},
                {road.IsHorizontal() ? Key::X1 : Key::Y1, road.IsHorizontal() ? road.GetEnd().x : road.GetEnd().y}
        };
        result.emplace_back(road_json);
    }
    return result;
}

json::array GetOffices(const Map* map){
    json::array result;
    for(auto& office : map->GetOffices()){
        json::object office_json {
                {Key::ID, *office.GetId()},
                {Key::X, office.GetPosition().x},
                {Key::Y, office.GetPosition().y},
                {Key::OFFSET_X, office.GetOffset().dx},
                {Key::OFFSET_Y, office.GetOffset().dy}
        };
        result.emplace_back(office_json);
    }
    return result;
}

json::array GetLootTypes(const Map* map){
    json::array result;
    for(auto& loot_type : map->GetLootTypes()){
        json::object loot_type_json;

        loot_type_json[Key::NAME] = loot_type.name;
        loot_type_json[Key::FILE] = loot_type.file;
        loot_type_json[Key::TYPE] = loot_type.type;
        if(loot_type.rotation.has_value())   loot_type_json[Key::ROTATION] = loot_type.rotation.value();
        if(loot_type.color.has_value())      loot_type_json[Key::COLOR] = loot_type.color.value();
        loot_type_json[Key::SCALE] = loot_type.scale;
        loot_type_json[Key::VALUE] = loot_type.value;

        result.emplace_back(loot_type_json);
    }
    return result;
}

json::object GetDog(const DogPtr& dog){
    json::object dog_json;
    dog_json[Key::POSITION] = json::array({dog->GetPosition().x, dog->GetPosition().y});
    dog_json[Key::SPEED] = json::array({dog->GetSpeed().x, dog->GetSpeed().y});
    dog_json[Key::DIRECTION] = dog->GetDirection();

    json::array bag_json;
    for(const auto& bag : dog->GetBag()){
        json::object loot;
        loot[Key::ID] = bag->GetId();
        loot[Key::TYPE] = bag->GetLootType();
        bag_json.push_back(loot);
    }

    dog_json[Key::BAG] = bag_json;
    dog_json[Key::SCORE] = dog->GetScore();

    return dog_json;
}

json::object GetLostObjects(const Game& game){
    json::object loot_obj;
    for (const auto& [map, session] : game.GetMapIdToSession()) {
        for(const auto& [id, loot] : session->GetLostObjects()){
            json::object temp;
            temp[Key::TYPE] = loot->GetLootType();
            temp[Key::POSITION] = json::array({loot->GetPosition().x, loot->GetPosition().y});
            loot_obj[std::to_string(loot->GetId())] = temp;
        }
    }
    return loot_obj;
}

void SetRoads(Map& map, const json::array& json_roads){
    for (auto& json_road : json_roads) {
        auto road_object = json_road.as_object();
        auto x0 = road_object[Key::X0].to_number<int>();
        auto y0 = road_object[Key::Y0].to_number<int>();

        if (road_object.if_contains(Key::X1)){
            auto x1 = road_object[Key::X1].to_number<int>();
            map.AddRoad({Road::HORIZONTAL, Point{x0, y0}, x1});
        } else if (road_object.if_contains(Key::Y1)) {
            auto y1 = road_object[Key::Y1].to_number<int>();
            map.AddRoad({Road::VERTICAL, Point{x0, y0}, y1});
        } else{
            throw std::runtime_error("Неправильный формат описания дороги");
        }
    }
}

void SetBuildings(Map& map, const json::array& json_buildings){
    for(auto& json_building : json_buildings) {
        auto pos = Point{json_building.at(Key::X).to_number<int>() ,json_building.at(Key::Y).to_number<int>()};
        auto size = Size{json_building.at(Key::WIDTH).to_number<int>(), json_building.at(Key::HEIGHT).to_number<int>()};
        auto building = Building{Rectangle{pos, size}};
        map.AddBuilding(building);
    }
}

void SetOffices(Map& map, const json::array& json_offices){
    for (auto& json_office : json_offices){
        auto office_id = Office::Id(json::value_to<std::string>(json_office.at(Key::ID)));
        auto position = Position{json_office.at(Key::X).to_number<double>(), json_office.at(Key::Y).to_number<double>()};
        auto offset = Offset{json_office.at(Key::OFFSET_X).to_number<int>(), json_office.at(Key::OFFSET_Y).to_number<int>()};
        map.AddOffice({office_id, position, offset});
    }
}

void SetDog(Map& map, const json::value& full_json, const json::object& map_obj){
    //скорость собаки задана для отдельной карты
    if(map_obj.contains(Key::DOG_SPEED)){
        auto speed = map_obj.at(Key::DOG_SPEED).as_double();
        map.SetDogSpeed(speed);
        return;
    }

    //использовать скорость, заданную для всех карт
    if(full_json.as_object().contains(Key::DEFAULT_DOG_SPEED)){
        auto speed = full_json.at(Key::DEFAULT_DOG_SPEED).as_double();
        map.SetDogSpeed(speed);
        return;
    }

    //скорость не задана
    map.SetDogSpeed(1);
}

void SetBagCapacity(Map& map, const json::value& full_json, const json::object& map_obj){
    //размер сумки задан для отдельной карты
    if(map_obj.contains(Key::BAG_CAPACITY)){
        auto capacity = map_obj.at(Key::BAG_CAPACITY).as_uint64();
        map.SetBagCapacity(capacity);
        return;
    }

    //использовать размер сумки, заданный для всех карт
    if(full_json.as_object().contains(Key::DEFAULT_BAG_CAPACITY)){
        auto capacity = full_json.at(Key::DEFAULT_BAG_CAPACITY).as_uint64();
        map.SetBagCapacity(capacity);
        return;
    }

    //размер сумки не задана
    map.SetBagCapacity(3);
}

void SetLootGenerator(Game& game, const json::value& lootGeneratorConfig){
    auto period = lootGeneratorConfig.at(Key::PERIOD).as_double();
    auto probability = lootGeneratorConfig.at(Key::PROBABILITY).as_double();

    game.LootGeneratorConfig(period, probability);
}

void SetLootTypes(Map& map, const json::array& json_loot_types){
    for(auto& json_loot_type : json_loot_types){
        auto& loot_type_obj = json_loot_type.as_object();
        LootType loot_type;

        loot_type.name = json::value_to<std::string>(json_loot_type.at(Key::NAME));
        loot_type.file = json::value_to<std::string>(json_loot_type.at(Key::FILE));
        loot_type.type = json::value_to<std::string>(json_loot_type.at(Key::TYPE));
        if(loot_type_obj.contains(Key::ROTATION)) loot_type.rotation = json_loot_type.at(Key::ROTATION).as_int64();
        if(loot_type_obj.contains(Key::COLOR))    loot_type.color = json::value_to<std::string>(json_loot_type.at(Key::COLOR));
        loot_type.scale = json_loot_type.at(Key::SCALE).as_double();
        loot_type.value = json_loot_type.at(Key::VALUE).as_int64();

        map.AddLootTypes(loot_type);
    }
}

void SetMaxInactiveTime(const json::value& full_json){
    if(full_json.as_object().contains(Key::DOG_RETIREMENT_TIME)){
        auto time = json::value_to<size_t>(full_json.at(Key::DOG_RETIREMENT_TIME));
        Dog::SetMaxInactiveTime(time);
    }
}

}   // namespace model