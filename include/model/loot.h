#pragma once
#include <string>
#include <utility>
#include "model_base.h"
#include "physics/collision_detector.h"
#include <boost/optional.hpp>

namespace model{

struct LootType {
    std::string name;                    // example -> = "key";
    std::string file;                    // example -> = "assets/key.obj";
    std::string type;                    // example -> = "obj";
    boost::optional<int> rotation;       // example -> = 90;
    boost::optional<std::string> color;  // example -> = "#338844";
    double scale = 0.0;                  // example -> = 0.03;
    size_t value = 0;                    // example -> = 10;
};

class LootObject : public collision_detector::Item{
    inline static size_t next_id = 0;

public:
    LootObject(size_t id, const Position& pos, size_t loot_type_);
    LootObject(const Position& pos, size_t loot_type_);

    bool operator==(const LootObject& other) const = default;
    const size_t& GetId() const { return id_; }
    const size_t& GetLootType() const {return loot_type_; }

private:
    size_t id_ = 0;
    size_t loot_type_;
};

}//namespace model

