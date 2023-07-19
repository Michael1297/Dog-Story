#pragma once

#include <memory>
#include "model/map.h"
#include "model/game.h"
#include "model/application.h"
#include "command_parse.h"
#include "ticker.h"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

namespace http_handler{

namespace beast = boost::beast;
namespace http = beast::http;
using StringRequest = http::request<http::string_body>;     // Запрос, тело которого представлено в виде строки
using StringResponse = http::response<http::string_body>;   // Ответ, тело которого представлено в виде строки
using JsonResponse = http::response<http::string_body>;
using AppPtr = std::shared_ptr<model::Application>;

class ApiHandler {
    static constexpr std::string_view API_REQUEST          = "/api/";
    static constexpr std::string_view MAPS_REQUEST         = "/api/v1/maps";
    static constexpr std::string_view MAP_REQUEST          = "/api/v1/maps/";
    static constexpr std::string_view GAME_JOIN_REQUEST    = "/api/v1/game/join";
    static constexpr std::string_view GAME_PLAYERS_REQUEST = "/api/v1/game/players";
    static constexpr std::string_view GAME_STATE_REQUEST   = "/api/v1/game/state";
    static constexpr std::string_view GAME_RECORDS_REQUEST = "/api/v1/game/records";
    static constexpr std::string_view GAME_ACTION_REQUEST  = "/api/v1/game/player/action";
    static constexpr std::string_view GAME_TICK_REQUEST    = "/api/v1/game/tick";

public:
    ApiHandler(const AppPtr& app, const command_line::Args& args)
            : game_(app->GetGame())
            , app_(app)
            , args_(args) {
    }
    StringResponse HandleApiRequest(const StringRequest& req);

    static bool IsApiRequest(const StringRequest& req);

private:
    model::Game& game_;
    const AppPtr& app_;
    const command_line::Args& args_;
    //получить список карт
    StringResponse GetMapList(const StringRequest& req);
    //получить карту
    StringResponse GetMap(const StringRequest& req);
    // зайти в игру
    StringResponse JoinPlayer(const StringRequest& req);
    // получить список игроков, находящихся в одной игровой сессии с игроком
    StringResponse GetPlayersOnMap(const StringRequest& req);
    // получить информацию о состоянии игры
    StringResponse GetGameState(const StringRequest& req);
    //получить список рекордсменов
    StringResponse GetGameRecords(const StringRequest& req);
    //  управлять действиями персонажа
    StringResponse MovePlayer(const StringRequest& req);
    //запрос для управления игровым временем
    StringResponse UpdateTime(const StringRequest& req);

    static std::string GetTokenFromRequest(const StringRequest& req);
    JsonResponse MakeJsonResponse(unsigned http_version, std::string_view response);
    JsonResponse MakeErrorResponse(http::status status, unsigned http_version, std::string_view code, std::string_view message);
    JsonResponse MakeErrorMethodResponse(unsigned http_version, std::string_view message, std::string_view allowed_method);
};

}

