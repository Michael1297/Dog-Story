#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <optional>
#include "model_base.h"
#include "tagged.h"
#include "loot.h"
#include "physics/collision_detector.h"

namespace model {

class Road {
    inline static size_t next_id = 0;

    struct HorizontalTag {
        HorizontalTag() = default;
    };

    struct VerticalTag {
        VerticalTag() = default;
    };

public:
    constexpr static HorizontalTag HORIZONTAL{};
    constexpr static VerticalTag VERTICAL{};

    Road(HorizontalTag, Point start, Coord end_x) noexcept
        : start_{start}
        , end_{end_x, start.y}
        , id_(next_id++){
        CalculateBoarders();
    }

    Road(VerticalTag, Point start, Coord end_y) noexcept
        : start_{start}
        , end_{start.x, end_y}
        , id_(next_id++){
        CalculateBoarders();
    }

    bool IsHorizontal() const noexcept {
        return start_.y == end_.y;
    }

    bool IsVertical() const noexcept {
        return start_.x == end_.x;
    }

    Point GetStart() const noexcept {
        return start_;
    }

    Point GetEnd() const noexcept {
        return end_;
    }

    const Position& GetStartBorder() const {
        return border_begin_;
    }

    const Position& GetEndBorder() const {
        return border_end_;
    }

    size_t GetId() const noexcept {
        return id_;
    }

    bool IsCoordinatesOnRoad(const Position& coord) const;

private:
    void CalculateBoarders();
    Point start_;
    Point end_;
    Position border_begin_;
    Position border_end_;
    size_t id_ = 0;
};

class Building {
public:
    explicit Building(Rectangle bounds) noexcept
        : bounds_{bounds} {
    }

    const Rectangle& getBounds() const noexcept {
        return bounds_;
    }

private:
    Rectangle bounds_;
};

class Office : public collision_detector::Item {
public:
    using Id = util::Tagged<std::string, Office>;

    Office(Id id, Position position, Offset offset, double width = BASE_WIDTH) noexcept
        : id_{std::move(id)}
        , Item(position, width)
        , offset_{offset} {
    }

    const Id& GetId() const noexcept {
        return id_;
    }

    Offset GetOffset() const noexcept {
        return offset_;
    }

private:
    Id id_;
    Offset offset_;
};

class Map {
public:
    using Id = util::Tagged<std::string, Map>;
    using Roads = std::vector<Road>;
    using Buildings = std::vector<Building>;
    using Offices = std::vector<Office>;
    using LootTypes = std::vector<LootType>;

    Map(Id id, std::string name) noexcept;

    const Id& GetId() const noexcept {
        return id_;
    }

    const std::string& GetName() const noexcept {
        return name_;
    }

    const Buildings& GetBuildings() const noexcept {
        return buildings_;
    }

    const Roads& GetRoads() const noexcept {
        return roads_;
    }

    const Offices& GetOffices() const noexcept {
        return offices_;
    }

    double GetDogSpeed() const {
        return dog_speed_;
    }

    size_t GetBagCapacity() const{
        return bag_capacity_;
    }

    const LootTypes& GetLootTypes() const noexcept{
        return loot_types_;
    }

    void AddRoad(const Road& road) {
        roads_.emplace_back(road);
    }

    void AddBuilding(const Building& building) {
        buildings_.emplace_back(building);
    }

    void AddOffice(Office office);

    void SetDogSpeed(double dog_speed) {
        dog_speed_ = dog_speed;
    }

    void SetBagCapacity(size_t bag_capacity){
        bag_capacity_ = bag_capacity;
    }

    void AddLootTypes(const LootType& loot_type){
        loot_types_.emplace_back(loot_type);
    }

    Position GetRandomPosition();
    std::optional<Road> FindRoadByPosition(const Position& position);
    std::optional<Road> FindRoadByPosition(const Position& position, size_t excepted_id);
    std::pair<Position, Speed> CalculatePositionAndSpeedOnRoad(const Road& road, const Position& pos, const Speed& speed);

private:
    using OfficeIdToIndex = std::unordered_map<Office::Id, size_t, util::TaggedHasher<Office::Id>>;

    Id id_;
    std::string name_;
    Roads roads_;
    Buildings buildings_;
    double dog_speed_ = 1;
    size_t bag_capacity_ = 3;

    OfficeIdToIndex warehouse_id_to_index_;
    Offices offices_;
    LootTypes loot_types_;
};

}  // namespace model
