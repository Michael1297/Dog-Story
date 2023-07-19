#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <algorithm>
#include <sstream>

#include "physics/geom.h"
#include "model/serialization/dog_serialization.h"

using namespace model;
using namespace std::literals;
namespace {

using InputArchive = boost::archive::text_iarchive;
using OutputArchive = boost::archive::text_oarchive;

struct Fixture {
    std::stringstream strm;
    OutputArchive output_archive{strm};
};

}  // namespace

template<typename T>
bool Compare(const std::vector<std::shared_ptr<T>>& first, const std::vector<std::shared_ptr<T>>& second){
    return std::equal(first.begin(), first.end(), second.begin(), second.end(), [](auto& value1, auto& value2){
        return *value1 == *value2;
    });
}

SCENARIO_METHOD(Fixture, "Point serialization") {
    GIVEN("A point") {
        const geom::Point2D p{10, 20};
        WHEN("point is serialized") {
            output_archive << p;

            THEN("it is equal to point after serialization") {
                InputArchive input_archive{strm};
                geom::Point2D restored_point;
                input_archive >> restored_point;
                CHECK(p == restored_point);
            }
        }
    }
}

SCENARIO_METHOD(Fixture, "Dog Serialization") {
    GIVEN("a dog") {
        const auto dog = [] {
            Dog dog{42, "Pluto"s, {42.2, 12.5}, 3};
            dog.AddScore(42);
            dog.CollectLostObject(std::make_shared<LootObject>(10, Position{1, 5}, 2));
            dog.SetDirection("D");
            dog.SetSpeed({2.3, -1.2});
            return dog;
        }();

        WHEN("dog is serialized") {
            {
                serialization::DogRepr repr{dog};
                output_archive << repr;
            }

            THEN("it can be deserialized") {
                InputArchive input_archive{strm};
                serialization::DogRepr repr;
                input_archive >> repr;
                auto restored = repr.Restore();

                CHECK(dog.GetId() == restored->GetId());
                CHECK(dog.GetName() == restored->GetName());
                CHECK(dog.GetPosition() == restored->GetPosition());
                CHECK(dog.GetSpeed() == restored->GetSpeed());
                CHECK(dog.GetBagCapacity() == restored->GetBagCapacity());
                CHECK(Compare(dog.GetBag(), restored->GetBag()));
            }
        }
    }
}
