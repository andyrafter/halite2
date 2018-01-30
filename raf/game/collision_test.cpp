#include "raf/math/math.hpp"
#include "collision.hpp"
#include "navigation.hpp"
#include "planet.hpp"
#include "ship.hpp"
#include "gtest/gtest.h"

#include <vector>

using raf::game::Planet;
using raf::game::Ship;
using raf::game::INVALID_ENTITIY_ID;
using raf::game::EntityId;

// https://halite.io/play/?game_id=3639882&replay_class=0&replay_name=replay-20171129-173105%2B0000--2144081290-288-192-1511976660
TEST(raf_collision, failed_in_deployment)
{
  const EntityId djgandy = 0;
  Planet planet(11, djgandy, { 117.21, 24.2308 }, 11.7339, 2992, 4);

  Ship docked_a(djgandy, 19, { 104.7854, 28.4567 }, raf::constants::SHIP_RADIUS, 255);

  Ship docked_b(djgandy, 28, { 104.1000, 24.1757 }, raf::constants::SHIP_RADIUS, 255);
  Ship collider(djgandy, 32, { 100.8589, 27.6934 }, raf::constants::SHIP_RADIUS, 255);

  // This was the dodgy vector being returned!
  raf::math::Velocity collider_vel(2.72799, -2.92541);

  auto final_dest = collider.current_location() + collider_vel;
  auto intersect = raf::collision::segment_circle_intersect(
    collider.current_location(),
    final_dest,
    docked_b.current_location(), 0.6, 0.5);

  auto distance = (final_dest - docked_b.current_location()).length();

  EXPECT_EQ(true, intersect);

  auto result = raf::navigation::navigate_ship_to_dock(
  { planet },
  { docked_a, docked_b, collider },
    collider,
    planet,
    raf::constants::MAX_SPEED,
    std::vector<raf::game::Path>()
  );

  if (result.second) {
    auto vector = raf::math::Vec2d(result.first.to_vec());
    std::cout << vector;
    auto intersect = raf::collision::segment_circle_intersect(
      collider.current_location(),
      collider.current_location() + vector,
      docked_b.current_location(),
      raf::constants::FORECAST_FUDGE_FACTOR,
      docked_b.radius());
    EXPECT_EQ(false, intersect);
  }
}
