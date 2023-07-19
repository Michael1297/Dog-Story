#include "model/json_loader.h"

#include <fstream>
#include "model/json_key.h"
#include "model/json_model_converter.h"

namespace json = boost::json;

namespace json_loader {

static std::string LoadFile(const std::filesystem::path& filePath){
    std::ifstream file(filePath, std::ios::binary | std::ios::ate); //открыть файл с конца
    if(!file.is_open()) {
        throw std::runtime_error("Couldn't open the configuration file");
    }
    std::string result(file.tellg(), ' ');
    file.seekg(0);
    file.read((char*)result.c_str(), result.length());
    file.close();
    return result;
}

model::Game LoadGame(const command_line::Args& args) {
    // Загрузить содержимое файла args.config_file, например, в виде строки
    // Распарсить строку как JSON, используя boost::json::parse
    // Загрузить модель игры из файла
    model::Game game(args);

    boost::system::error_code ec;
    auto json_obj = json::parse(LoadFile(args.config_file), ec);

    if (ec) {
        throw std::runtime_error("Couldn't parse json file!");
    }

    auto json_maps = json_obj.at(Key::MAPS).as_array();

    for (auto& json_map : json_maps) {
        auto id = model::Map::Id(json::value_to<std::string>(json_map.at(Key::ID)));
        auto name = json::value_to<std::string>(json_map.at(Key::NAME));
        model::Map map{id, name};

        model::SetRoads(map, json_map.at(Key::ROADS).as_array());
        model::SetBuildings(map, json_map.at(Key::BUILDINGS).as_array());
        model::SetOffices(map, json_map.at(Key::OFFICES).as_array());
        model::SetLootTypes(map, json_map.at(Key::LOOT_TYPES).as_array());
        model::SetDog(map, json_obj, json_map.as_object());
        model::SetBagCapacity(map, json_obj, json_map.as_object());

        game.AddMap(map);
    }

    model::SetLootGenerator(game, json_obj.at(Key::LOOT_GENERATOR_CONFIG));
    model::SetMaxInactiveTime(json_obj);

    return game;
}

}  // namespace json_loader