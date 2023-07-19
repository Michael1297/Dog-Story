#include "api_handler.h"
#include <chrono>
#include <optional>
#include <sstream>
#include "model/model_base.h"
#include "model/json_key.h"
#include "model/json_model_converter.h"
#include "database/db_functions.h"
#include "request_exception.h"
#include <boost/json.hpp>
#include <boost/url/parse.hpp>
#include <boost/url/params_view.hpp>

#define RECORD_LIMIT 100

template <typename T>
T GetValueFromUrlParameter(const boost::urls::params_view& params, const std::string& key) {
    std::stringstream ss;
    ss << (*params.find(key)).value;
    T res;
    ss >> res;
    return res;
};

namespace http_handler{

namespace sys = boost::system;
namespace json = boost::json;

StringResponse ApiHandler::GetMapList(const StringRequest& req){
    assert(req.target() == MAPS_REQUEST);
    auto version = req.version();
    if(req.method() != http::verb::get && req.method() != http::verb::head){
        return MakeErrorMethodResponse(version, "Only GET, HEAD methods are expected", "GET, HEAD");
    }

    auto maps = game_.GetMaps();
    json::array json_array;

    for(auto& map : maps){
        json::object json_map;
        json_map[Key::ID] = *map.GetId();
        json_map[Key::NAME] = map.GetName();
        json_array.emplace_back(json_map);
    }

    return MakeJsonResponse(version, json::serialize(json_array));
}

StringResponse ApiHandler::GetMap(const StringRequest& req) {
    assert(req.target().starts_with(MAP_REQUEST));
    auto version = req.version();
    if(req.method() != http::verb::get && req.method() != http::verb::head){
        return MakeErrorMethodResponse(version, "Only GET, HEAD methods are expected", "GET, HEAD");
    }

    auto map_id = req.target().substr(MAP_REQUEST.length());
    auto map = game_.FindMap(model::Map::Id{map_id});

    //карта не найдена
    if (map == nullptr){
        return MakeErrorResponse(http::status::not_found, version, "mapNotFound", "Map not found");
    }

    json::object obj;
    obj[Key::ID] = *map->GetId();
    obj[Key::NAME] = map->GetName();
    obj[Key::ROADS] = model::GetRoads(map);
    obj[Key::BUILDINGS] = model::GetBuildings(map);
    obj[Key::OFFICES] = model::GetOffices(map);
    obj[Key::LOOT_TYPES] = model::GetLootTypes(map);

    return MakeJsonResponse(version, json::serialize(obj));
}

StringResponse ApiHandler::JoinPlayer(const StringRequest& req) {
    assert(req.target() == GAME_JOIN_REQUEST);
    auto version = req.version();
    if (req.method() != http::verb::post){
        return MakeErrorMethodResponse(version, "Only POST method is expected", "POST");
    }

    json::object result;
    sys::error_code ec;
    json::value val = json::parse(req.body(), ec);

    if (ec || !val.as_object().contains(Key::USER_NAME) || !val.as_object().contains(Key::MAP_ID) ||  !val.at(Key::USER_NAME).if_string() || !val.at(Key::MAP_ID).if_string()) {
        return MakeErrorResponse(http::status::bad_request, version, "invalidArgument",  "Join game request parse error");
    }

    auto name = json::value_to<std::string>(val.at(Key::USER_NAME));

    if (name.empty()) {
        return MakeErrorResponse(http::status::bad_request, version, "invalidArgument",  "Invalid name");
    }

    auto mapId = json::value_to<std::string>(val.at(Key::MAP_ID));
    model::Map::Id map_id{mapId};

    if(game_.FindMap(map_id)){
        auto token = game_.JoinGame(name, map_id, args_.random_spawn);
        auto player = game_.FindPlayerByToken(token);

        result[Key::AUTH_TOKEN] = *token;
        result[Key::PLAYER_ID] = player->GetId();
    } else{
        return MakeErrorResponse(http::status::not_found, version, "mapNotFound",  "Map not found");
    }

    return MakeJsonResponse(version, json::serialize(result));
}

StringResponse ApiHandler::GetPlayersOnMap(const http_handler::StringRequest& req) {
    assert(req.target() == GAME_PLAYERS_REQUEST);
    auto version = req.version();
    if(req.method() != http::verb::get && req.method() != http::verb::head){
        return MakeErrorMethodResponse(version, "Only GET, HEAD methods are expected", "GET, HEAD");
    }

    json::object result;
    try{
        auto token = GetTokenFromRequest(req);
        auto player = game_.FindPlayerByToken(model::Token(token));
        const auto& map_id = player->GetMapId();
        const auto& players = game_.GetPlayersOnMap(map_id);
        for (const auto& player_id : players) {
            const auto& current_player = game_.FindPlayerById(player_id);
            json::object name_obj;
            name_obj[Key::NAME] = current_player->GetName();
            result[std::to_string(current_player->GetId())] = name_obj;
        }
    }
    catch (RequestException& e){
        return MakeErrorResponse(http::status::unauthorized, version, e.what(), e.message());
    }

    return MakeJsonResponse(version, json::serialize(result));
}

StringResponse ApiHandler::GetGameState(const http_handler::StringRequest& req) {
    assert(req.target() == GAME_STATE_REQUEST);
    auto version = req.version();
    if(req.method() != http::verb::get && req.method() != http::verb::head){
        return MakeErrorMethodResponse(version, "Only GET, HEAD methods are expected", "GET, HEAD");
    }

    json::object result;
    try{
        auto token = GetTokenFromRequest(req);
        game_.FindPlayerByToken(model::Token(token));   //проверить что игрок с этим токенов существует

        json::object players_obj;
        for (const auto& [id, player] : game_.GetPlayers()) {
            players_obj[std::to_string(player->GetId())] = model::GetDog(player->GetDog());
        }

        result[Key::PLAYERS] = players_obj;
        result[Key::LOST_OBJECTS] = model::GetLostObjects(game_);
    }
    catch (RequestException& e) {
        return MakeErrorResponse(http::status::unauthorized, version, e.what(), e.message());
    }
    return MakeJsonResponse(version, json::serialize(result));
}

StringResponse ApiHandler::GetGameRecords(const http_handler::StringRequest& req){
    assert(req.target().starts_with(GAME_RECORDS_REQUEST));
    auto version = req.version();
    if(req.method() != http::verb::get && req.method() != http::verb::head){
        return MakeErrorMethodResponse(version, "Only GET, HEAD methods are expected", "GET, HEAD");
    }

    std::optional<int> offset;
    std::optional<int> limit;
    auto params = boost::urls::url_view{req.target()}.params();

    if (params.contains("start")) {
        offset = GetValueFromUrlParameter<int>(params, "start");
    }

    if (params.contains("maxItems")) {
        limit = GetValueFromUrlParameter<int>(params, "maxItems");
    }

    if(limit.has_value() && limit.value() > RECORD_LIMIT){
        return MakeErrorResponse(http::status::bad_request, version, "badRequest", "Bad Request");
    }

    json::array result;
    for(auto& player : db::GetRetiredPlayers(offset.value_or(0), limit.value_or(RECORD_LIMIT))){
        json::object player_record ;
        player_record[Key::NAME] = player.GetName();
        player_record[Key::SCORE] = player.GetScore();
        player_record[Key::PLAY_TIME] = static_cast<double>(player.GetPlayTime()) / model::MILLISECONDS_IN_SECOND;
        result.push_back(player_record);
    }

    return MakeJsonResponse(version, json::serialize(result));
}

StringResponse ApiHandler::MovePlayer(const http_handler::StringRequest& req) {
    assert(req.target() == GAME_ACTION_REQUEST);
    auto version = req.version();
    if (req.method() != http::verb::post){
        return MakeErrorMethodResponse(version, "Only POST method is expected", "POST");
    }

    try{
        auto token = GetTokenFromRequest(req);
        auto player = game_.FindPlayerByToken(model::Token(token));

        sys::error_code ec;
        json::value val = json::parse(req.body(), ec);
        if(ec || !val.as_object().contains(Key::MOVE) || !val.at(Key::MOVE).is_string()){
            return MakeErrorResponse(http::status::bad_request, version, "invalidArgument",  "Failed to parse action");
        }

        auto direction = json::value_to<std::string>(val.at(Key::MOVE));
        player->setDirection(direction);
    }
    catch (RequestException& e) {
        return MakeErrorResponse(http::status::unauthorized, version, e.what(), e.message());
    }

    return MakeJsonResponse(version, "{}");
}

StringResponse ApiHandler::UpdateTime(const http_handler::StringRequest &req) {
    assert(req.target() == GAME_TICK_REQUEST);
    assert(!args_.period);
    auto version = req.version();
    if (req.method() != http::verb::post){
        return MakeErrorMethodResponse(version, "Only POST method is expected", "POST");
    }
    sys::error_code ec;
    json::value object = json::parse(req.body(), ec);

    if(ec || !object.as_object().contains(Key::TIME_DELTA) || !object.at(Key::TIME_DELTA).is_int64()){
        return MakeErrorResponse(http::status::bad_request, version, "invalidArgument",  "Failed to parse action");
    }

    std::chrono::milliseconds delta_time (object.at(Key::TIME_DELTA).as_int64());
    game_.UpdateTimeInSessions(delta_time);
    game_.RemoveAfkPlayers();
    app_->SaveGameState(delta_time);

    return MakeJsonResponse(version, "{}");
}

std::string ApiHandler::GetTokenFromRequest(const http_handler::StringRequest &req) {
    if (req.find("authorization") == req.end()) {
        throw RequestException ("invalidToken", "Authorization header is missing");
    }

    auto str = req.at("authorization");
    if (str.find("Bearer") == std::string::npos) {
        throw RequestException ("invalidToken", "Authorization header is missing");
    }

    auto pos = str.find_last_of(' ');
    std::string result = str.substr(pos + 1);

    if (result.length() != model::TOKEN_LENGTH) {
        throw RequestException ("invalidToken", "Authorization header is missing");
    }

    return result;
}

JsonResponse ApiHandler::MakeJsonResponse(unsigned int http_version, std::string_view response) {
    JsonResponse json_response(http::status::ok, http_version);
    json_response.body() = response;
    json_response.content_length(json_response.body().length());
    json_response.set(http::field::content_type, "application/json");
    json_response.set(http::field::cache_control, "no-cache");
    return json_response;
}

JsonResponse ApiHandler::MakeErrorResponse(http::status status, unsigned int http_version, std::string_view code, std::string_view message) {
    json::value jv({{"code", code}, {"message", message}});
    JsonResponse response(status, http_version);
    response.body() = json::serialize(jv);
    response.content_length(response.body().length());
    response.set(http::field::content_type, "application/json");
    response.set(http::field::cache_control, "no-cache");
    return response;
}

JsonResponse ApiHandler::MakeErrorMethodResponse(unsigned int http_version, std::string_view message, std::string_view allowed_method) {
    auto response = MakeErrorResponse(http::status::method_not_allowed, http_version, "invalidMethod", message);
    response.set(http::field::allow, allowed_method);
    return response;
}

bool ApiHandler::IsApiRequest(const http_handler::StringRequest& req) {
    return req.target().starts_with(API_REQUEST);
}

StringResponse ApiHandler::HandleApiRequest(const StringRequest& req) {
    if (req.target() == MAPS_REQUEST){    // отправить список карт
        return GetMapList(req);
    }

    if (req.target().starts_with(MAP_REQUEST)){     // отправить карту
        return GetMap(req);
    }

    if (req.target() == GAME_JOIN_REQUEST){  // зайти в игру
        return JoinPlayer(req);
    }

    if (req.target() == GAME_PLAYERS_REQUEST){    // получить список игроков, находящихся в одной игровой сессии с игроком
        return GetPlayersOnMap(req);
    }

    if(req.target() == GAME_STATE_REQUEST){   // получить информацию о состоянии игры
        return GetGameState(req);
    }

    if(req.target().starts_with(GAME_RECORDS_REQUEST)){   // получить список рекордсменов
        return GetGameRecords(req);
    }

    if(req.target() == GAME_ACTION_REQUEST){   // управлять действиями персонажа
        return MovePlayer(req);
    }

    if(req.target() == GAME_TICK_REQUEST && !args_.period){    //запрос для управления игровым временем
        return UpdateTime(req);
    }

    return MakeErrorResponse(http::status::bad_request, req.version(), "badRequest", "Bad request");
}

}

