#include "model/dog.h"
#include <stdexcept>
#include <cmath>
#include <utility>
#include "request_exception.h"

using namespace std::chrono_literals;

namespace model{

Dog::Dog(size_t id, std::string name, const Position& position, size_t bag_capacity)
        : id_(id)
        , name_(std::move(name))
        , position_(position)
        , bag_capacity_(bag_capacity){
    if(next_id <= id){
        next_id = id + 1;
    }
}

Dog::Dog(std::string name, const Position& position, size_t bag_capacity)
        : id_(next_id++)
        , name_(std::move(name))
        , position_(position)
        , bag_capacity_(bag_capacity){
}

const size_t& Dog::GetId() const {
    return id_;
}

const std::string& Dog::GetName() const {
    return name_;
}

const Position& Dog::GetPosition() const {
    return position_;
}

const Speed& Dog::GetSpeed() const {
    return speed_;
}

const std::string& Dog::GetDirection() const {
    return direction_;
}

const Dog::BagContent& Dog::GetBag() const {
    return bag_;
}

const size_t& Dog::GetBagCapacity() const {
    return bag_capacity_;
}

const size_t& Dog::GetScore() const {
    return score_;
}

const std::chrono::milliseconds& Dog::GetLiveTime() {
    return live_time_;
}

const collision_detector::Gatherer& Dog::GetGatherer() {
    return gatherer_;
}

double Dog::GetAbsSpeed(const Speed &speed) {
    return std::sqrt(std::pow(speed.x, 2) + std::pow(speed.y, 2));
}

double Dog::GetDistance(const Position& pos1, const Position& pos2) {
    return std::sqrt(std::pow(pos2.x - pos1.x, 2) + std::pow(pos2.y - pos1.y, 2));
}

void Dog::SetMaxInactiveTime(size_t max_inactive_time_in_seconds) {
    max_inactive_time_ = std::chrono::seconds(max_inactive_time_in_seconds);
}

void Dog::SetPosition(const Position& position) {
    position_ = position;

    gatherer_.start_pos = gatherer_.end_pos;
    gatherer_.end_pos = position_;
}

void Dog::SetSpeed(const Speed& speed) {
    speed_ = speed;
}

void Dog::SetSpeed(const Speed& speed, const std::chrono::milliseconds& time_delta) {
    speed_ = speed;
    live_time_ += time_delta;

    if(speed == Speed{0, 0}){
        afk_time_ += time_delta;
    } else{
        afk_time_ = 0ms;
    }
}

void Dog::SetDirection(const std::string& direction) {
    if(direction.size() > 1 || direction.find_first_not_of("LRUD") != std::string::npos){
        throw std::runtime_error("Invalid dog direction");
    }
    direction_ = direction;
}

void Dog::SetDirection(const std::string& direction, double dog_speed) {
    if(direction.size() > 1 || direction.find_first_not_of("LRUD") != std::string::npos){
        throw RequestException ("invalidArgument", "Failed to parse action");
    }
    direction_ = direction;
    afk_time_ = 0ms;

    if(direction.empty()){
        speed_ = {0, 0};
        return;
    }

    switch (direction.front()) {
        case 'L':
            speed_ = {-dog_speed, 0};
            return;
        case 'R':
            speed_ = {dog_speed, 0};
            return;
        case 'U':
            speed_ = {0, -dog_speed};
            return;
        case 'D':
            speed_ = {0, dog_speed};
            return;
    }
}

Position Dog::CalculateNewPosition(const std::chrono::milliseconds& delta_time) const {
    auto position = GetPosition();
    const auto& speed = GetSpeed();
    position.x += speed.x * delta_time.count()  / MILLISECONDS_IN_SECOND;
    position.y += speed.y * delta_time.count()  / MILLISECONDS_IN_SECOND;
    return position;
}

void Dog::CollectLostObject(const std::shared_ptr<LootObject>& lost_obj) {
    if(!IsFullBag()){
        bag_.push_back(lost_obj);
    }
}

void Dog::DropLostObjects(const LootTypes& loot_types) {
    for(auto& loot : bag_){
        AddScore(loot_types[loot->GetLootType()].value);
    }

    bag_.clear();
}

void Dog::AddScore(size_t value) {
    score_ += value;
}

bool Dog::IsFullBag() {
    return bag_.size() >= bag_capacity_;
}

bool Dog::IsEmptyBag() {
    return bag_.empty();
}

bool Dog::IsAfk() {
    return afk_time_ >= max_inactive_time_;
}

}