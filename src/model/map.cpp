#include "model/map.h"
#include <stdexcept>
#include <cstdlib>
#include <ctime>

namespace model {
using namespace std::literals;

void Map::AddOffice(Office office) {
    if (warehouse_id_to_index_.contains(office.GetId())) {
        throw std::invalid_argument("Duplicate warehouse");
    }

    const size_t index = offices_.size();
    Office& o = offices_.emplace_back(std::move(office));
    try {
        warehouse_id_to_index_.emplace(o.GetId(), index);
    } catch (...) {
        // Удаляем офис из вектора, если не удалось вставить в unordered_map
        offices_.pop_back();
        throw;
    }
}

bool Road::IsCoordinatesOnRoad(const Position &coord) const {
    return (coord.x >= border_begin_.x && coord.x <= border_end_.x)
           && (coord.y >= border_begin_.y && coord.y <= border_end_.y);
}

void Road::CalculateBoarders() {
    border_begin_.x = std::min(start_.x, end_.x) - BORDER_WIDTH;
    border_begin_.y = std::min(start_.y, end_.y) - BORDER_WIDTH;
    border_end_.x = std::max(start_.x, end_.x) + BORDER_WIDTH;
    border_end_.y = std::max(start_.y, end_.y) + BORDER_WIDTH;
}

Map::Map(model::Map::Id id, std::string name) noexcept
        : id_(std::move(id))
        , name_(std::move(name)){
    std::srand((unsigned)std::time(nullptr));
}

Position Map::GetRandomPosition() {
    auto road = std::rand() % roads_.size();
    const auto& start_border = roads_[road].GetStartBorder();
    const auto& end_border = roads_[road].GetEndBorder();
    double pos_x =  start_border.x + (double)std::rand() / RAND_MAX  * (end_border.x - start_border.x);
    double pos_y =  start_border.y + (double)std::rand() / RAND_MAX  * (end_border.y - start_border.y);
    return {pos_x, pos_y};
}

std::optional<Road> Map::FindRoadByPosition(const Position& position) {
    for(auto& road : roads_){
        if(road.IsCoordinatesOnRoad(position)) return road;
    }
    return {};
}

std::optional<Road> Map::FindRoadByPosition(const Position& position, size_t excepted_id){
    for(auto& road : roads_){
        if(road.GetId() != excepted_id && road.IsCoordinatesOnRoad(position)) return road;
    }
    return {};
}

std::pair<Position, Speed> Map::CalculatePositionAndSpeedOnRoad(const Road& road, const Position& pos, const Speed& speed) {
    Position result_pos;
    Speed result_speed = speed;

    const double left_boarder = road.GetStartBorder().x;
    const double right_boarder = road.GetEndBorder().x;
    const double down_boarder = road.GetStartBorder().y;
    const double up_boarder = road.GetEndBorder().y;

    if (pos.x <= left_boarder) {
        result_pos.x = left_boarder;
        result_speed.x = 0;
    } else if (pos.x >= right_boarder) {
        result_pos.x = right_boarder;
        result_speed.x = 0;
    } else {
        result_pos.x = pos.x;
    }

    if (pos.y >= up_boarder) {
        result_pos.y = up_boarder;
        result_speed.y = 0;
    } else if (pos.y <= down_boarder) {
        result_pos.y = down_boarder;
        result_speed.y = 0;
    } else {
        result_pos.y = pos.y;
    }

    return {result_pos, result_speed};
}

}  // namespace model