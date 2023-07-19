#include "model/loot.h"

namespace model{

LootObject::LootObject(size_t id, const Position& pos, size_t loot_type_)
        : id_(id)
        , Item(pos, LOOT_WIDTH)
        , loot_type_(loot_type_){
    if(next_id <= id){
        next_id = id + 1;
    }
}

LootObject::LootObject(const Position& pos, size_t loot_type_)
        : id_(next_id++)
        , Item(pos, LOOT_WIDTH)
        , loot_type_(loot_type_){
}

} //namespace model