#ifndef RAF_UTIL_H_
#define RAF_UTIL_H_

#include "../hlt/navigation.hpp"

#include "math/math.hpp"

namespace raf {

namespace {

static hlt::possibly<hlt::Move> navigate_ship_to_location(
  const hlt::Map& map,
  const hlt::Ship& ship,
  const hlt::Location& target,
  const int max_thrust,
  const std::vector<hlt::Move>& pending_moves)
{
  const int max_corrections = hlt::constants::MAX_NAVIGATION_CORRECTIONS;
  const bool avoid_obstacles = true;
  const double angular_step_rad = M_PI / 180.0;

  return hlt::navigation::navigate_ship_towards_target(
    map, ship, target, max_thrust, avoid_obstacles, max_corrections, angular_step_rad, pending_moves);
}

}
}

#endif // !RAF_UTIL_H_
