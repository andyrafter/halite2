#include "planet.hpp"
#include "ship.hpp"
#include "constants.hpp"
#include "raf/math/math.hpp"
#include "gtest/gtest.h"

#include <algorithm>

using raf::game::INVALID_ENTITIY_ID;
using raf::game::Planet;
using raf::game::Ship;
using raf::math::Vec2d;

static int planet_health_from_radius(double radius) {
  return radius * 255;
}

// Note: Planets of size 6 have only 2 docking spots
// https://halite.io/play/?game_id=3559068&replay_class=0&replay_name=replay-20171128-143620%2B0000--4040000344-312-208-1511879772
static int planet_docking_spots_from_radius(double radius) {
  return static_cast<int>(std::max(1.0, std::ceil(radius / 3.0)));
}

TEST(raf_planet, basic_invariants)
{
  Vec2d location{ 3, 3 };
  double radius = 3.0;
  const auto total_docking_spots = planet_docking_spots_from_radius(radius);
  auto planet = Planet(0, INVALID_ENTITIY_ID, location, radius, planet_health_from_radius(radius), total_docking_spots);

  ASSERT_TRUE(planet.is_empty());
  ASSERT_TRUE(planet.is_alive());
  ASSERT_FALSE(planet.is_owned());
  ASSERT_FALSE(planet.is_full());

  ASSERT_EQ(total_docking_spots, planet.total_docking_spots());
  ASSERT_EQ(total_docking_spots, planet.free_docking_spots());
  ASSERT_EQ(0, planet.used_docking_spots());

  ASSERT_FALSE(planet.is_docked(0));

  ASSERT_TRUE(is_dockable(planet, 0));
  ASSERT_FALSE(is_owned_by_opponent(planet, 0));
}

TEST(raf_planet, docking)
{
  const Vec2d planet_location{ 3, 3 };
  const Vec2d ship_location{ 0, 3 };
  double radius = 3.0;
  const auto total_docking_spots = planet_docking_spots_from_radius(radius);
  auto planet = Planet(0, INVALID_ENTITIY_ID, planet_location, radius, planet_health_from_radius(radius), total_docking_spots);
  auto ship = Ship(0, 0, ship_location, raf::constants::SHIP_RADIUS, raf::constants::BASE_SHIP_HEALTH);

  ASSERT_TRUE(can_dock(planet, ship));

  ship.update_location({ -1, 3 });
  ASSERT_TRUE(can_dock(planet, ship));

  ship.update_location({ -2, 3 });
  ASSERT_TRUE(can_dock(planet, ship));

  ship.update_location({ -3, 3 });
  ASSERT_TRUE(can_dock(planet, ship));

  ship.update_location({ -4, 3 });
  ASSERT_TRUE(can_dock(planet, ship));

  ship.update_location({ -4.5, 3 });
  ASSERT_TRUE(can_dock(planet, ship));

  // > 4.5 docking should fail.
  ship.update_location({ -4.501, 3 });
  ASSERT_FALSE(can_dock(planet, ship));

  ship.update_location({ -4.5001, 3 });
  ASSERT_FALSE(can_dock(planet, ship));

  ship.update_location({ -4.500001, 3 });
  ASSERT_FALSE(can_dock(planet, ship));
}

TEST(raf_planet, attacks_to_destroy)
{
  Vec2d location{ 3, 3 };

  for (double radius = 3.0; radius < 12.0; radius += 0.1) {
    auto planet = Planet(0, 0, location, radius, planet_health_from_radius(radius), planet_docking_spots_from_radius(radius));
    //ASSERT_EQ(static_cast<int>(std::ceil(radius)), attacks_to_destroy(planet, 255));
    ASSERT_EQ(planet.health(), planet_health_from_radius(radius));
  }
}



