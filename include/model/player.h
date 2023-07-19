#pragma once

#include <string>
#include <cstdint>
#include <utility>
#include "dog.h"
#include "map.h"
#include "game_session.h"

namespace model{

using SessionPtr = std::shared_ptr<GameSession>;
using DogPtr = std::shared_ptr<Dog>;

class Player {
    inline static size_t next_id = 0;

public:
    Player(size_t id, std::string name, SessionPtr session, DogPtr dog);
    Player(std::string name, SessionPtr session, DogPtr dog);

    const size_t GetId() const { return id_; }
    const std::string& GetName() const { return name_; }
    const auto GetSession() const { return session_; }
    const auto& GetMapId() const { return session_->GetMapId(); }
    const auto& GetDog() const { return dog_; }
    void setDirection(const std::string& direction);

private:
    size_t id_ = 0;
    std::string name_;
    SessionPtr session_;
    DogPtr dog_;
};

} //namespace model