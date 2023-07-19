#pragma once
#include "game.h"
#include "map.h"
#include <boost/json.hpp>

namespace model {
namespace json = boost::json;

json::array GetBuildings(const Map* map);
json::array GetRoads(const Map* map);
json::array GetOffices(const Map* map);
json::array GetLootTypes(const Map* map);
json::object GetDog(const DogPtr& dog);
json::object GetLostObjects(const Game& game);
void SetRoads(Map & map, const json::array& json_roads);
void SetBuildings(Map & map, const json::array& json_buildings);
void SetOffices(Map & map, const json::array& json_offices);
void SetDog(Map & map, const json::value& full_json, const json::object& map_obj);
void SetBagCapacity(Map & map, const json::value& full_json, const json::object& map_obj);
void SetLootGenerator(Game & game, const json::value& lootGeneratorConfig);
void SetLootTypes(Map & map, const json::array& json_loot_types);
void SetMaxInactiveTime(const json::value& full_json);

}   //  namespace model