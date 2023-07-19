#pragma once

#include "physics/geom.h"

namespace model{

using Dimension = int;
using Coord = Dimension;
using Position = geom::Point2D;
using Speed = geom::Vec2D;

const double BORDER_WIDTH = 0.4;
const double DOG_WIDTH = 0.3;
const double LOOT_WIDTH = 0.0;
const double BASE_WIDTH = 0.5;
const double MILLISECONDS_IN_SECOND = 1000.0;
const size_t ONE_MINUTE_IN_SECONDS = 60;
const int TOKEN_LENGTH = 32;

struct Point {
    Coord x, y;
};

struct Size {
    Dimension width, height;
};

struct Rectangle {
    Point position;
    Size size;
};

struct Offset {
    Dimension dx, dy;
};

}