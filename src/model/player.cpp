#include "model/player.h"

namespace model{

Player::Player(size_t id, std::string name, SessionPtr session, DogPtr dog)
        : id_(id)
        , name_(std::move(name))
        , session_(std::move(session))
        , dog_(std::move(dog)){
    if(next_id <= id){
        next_id = id + 1;
    }
}

Player::Player(std::string name, SessionPtr session, DogPtr dog)
        : id_(next_id++)
        , name_(std::move(name))
        , session_(std::move(session))
        , dog_(std::move(dog)){
}

void Player::setDirection(const std::string& direction){
    dog_->SetDirection(direction, session_->GetMap().GetDogSpeed());
}

} //namespace model