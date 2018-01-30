#include "path_finder.hpp"
#include "gtest/gtest.h"
#include "raf/game/constants.hpp"
#include "raf/game/entity.hpp"
#include "raf/math/math.hpp"

using raf::game::Entity;
using raf::game::Path;
using raf::game::Planet;
using raf::game::Ship;
using raf::math::Vec2d;
using raf::navigation::distance_to_point;
using raf::navigation::distance_to_radius;
using raf::navigation::edge_distance_to_edge;
using raf::navigation::edge_distance_to_dock;
using raf::navigation::weapon_distance_to_edge;
using raf::navigation::could_attack_next_turn;
using raf::navigation::will_attack_next_turn;

TEST(raf_path_finder, construction)
{
}

TEST(raf_path_finder, distance_to_point_vec2)
{
  const Vec2d a(0, 0);
  ASSERT_DOUBLE_EQ(distance_to_point(a, a), 0);

  const Vec2d b(1, 1);
  ASSERT_DOUBLE_EQ(distance_to_point(a, b), std::sqrt(2));
}

TEST(raf_path_finder, distance_to_point_entity)
{
  const Entity a{ 0, 0, {0, 0}, 0, 255 };
  ASSERT_DOUBLE_EQ(distance_to_point(a, a), 0);

  const Entity b{ 1, 0,{ 1, 1 }, 0, 255 };
  ASSERT_DOUBLE_EQ(distance_to_point(a, b), std::sqrt(2));
}

TEST(raf_path_finder, distance_to_point_radius_vec2)
{
  const Vec2d a(0, 0);
  ASSERT_DOUBLE_EQ(distance_to_radius(a, a, 0), 0);

  const Vec2d b(1, 1);
  ASSERT_DOUBLE_EQ(distance_to_radius(a, b, 0), std::sqrt(2));
  ASSERT_DOUBLE_EQ(distance_to_radius(a, b, 1), std::sqrt(2) - 1);

  const Vec2d c(1, 0);
  ASSERT_DOUBLE_EQ(distance_to_radius(a, c, 0), 1);
  ASSERT_DOUBLE_EQ(distance_to_radius(a, c, 1), 0);

}

TEST(raf_path_finder, distance_to_point_radius_entity)
{
  const Entity a{ 0, 0,{ 0, 0 }, 0, 255 };
  ASSERT_DOUBLE_EQ(distance_to_radius(a, a, 0), 0);

  const Entity b{ 1, 0,{ 1, 1 }, 0, 255 };
  ASSERT_DOUBLE_EQ(distance_to_radius(a, b, 0), std::sqrt(2));
  ASSERT_DOUBLE_EQ(distance_to_radius(a, b, 1), std::sqrt(2) - 1);

  const Entity c{ 0, 0,{ 1, 0 }, 0, 255 };
  ASSERT_DOUBLE_EQ(distance_to_radius(a, c, 0), 1);
  ASSERT_DOUBLE_EQ(distance_to_radius(a, c, 1), 0);
}

TEST(raf_path_finder, edge_distance_to_edge)
{
  const Entity a{ 0, 0,{ 0, 0 }, 0.5, 255 };
  ASSERT_DOUBLE_EQ(edge_distance_to_edge(a, a), -1);

  const Entity b{ 1, 0,{ 1, 1 }, 0.5, 255 };
  ASSERT_DOUBLE_EQ(edge_distance_to_edge(a, b), std::sqrt(2) - 1);

  const Entity c{ 0, 0,{ 1, 0 }, 0.5, 255 };
  ASSERT_DOUBLE_EQ(edge_distance_to_edge(a, c), 0);
}

TEST(raf_path_finder, weapon_distance_to_edge)
{
  const Ship a{ 0, 0,{ 0, 0 }, 0.5, 255 };
  // 2 * radius + WEAPON_RADIUS
  ASSERT_DOUBLE_EQ(weapon_distance_to_edge(a, a), -(raf::constants::WEAPON_RADIUS + 0.5 + 0.5));

  const Ship b{ 1, 0,{ 1, 1 }, 0.5, 255 };
  ASSERT_DOUBLE_EQ(weapon_distance_to_edge(a, b), -(raf::constants::WEAPON_RADIUS + 0.5 + 0.5) + std::sqrt(2) );

  const Ship c{ 0, 0,{ 1, 0 }, 0.5, 255 };
  ASSERT_DOUBLE_EQ(weapon_distance_to_edge(a, c), -(raf::constants::WEAPON_RADIUS));
}

TEST(raf_path_finder, could_attack_next_turn)
{
  const Ship a{ 0, 0,{ 0, 0 }, 0.5, 255 };
  ASSERT_TRUE(could_attack_next_turn(a, a));

  const Ship b{ 1, 0,{ 1, 1 }, 0.5, 255 };
  ASSERT_TRUE(could_attack_next_turn(a, b));

  const Ship c{ 0, 0,{ 1, 0 }, 0.5, 255 };
  ASSERT_TRUE(could_attack_next_turn(a, c));

  const Ship d{ 0, 0,{ 20, 0 }, 0.5, 255 };
  ASSERT_TRUE(could_attack_next_turn(a, d));

  const Ship e{ 0, 0,{ 21, 0 }, 0.5, 255 };
  ASSERT_FALSE(could_attack_next_turn(a, e));

  const Ship f{ 0, 0,{ 20.000001, 0 }, 0.5, 255 };
  ASSERT_FALSE(could_attack_next_turn(a, f));
}

TEST(raf_path_finder, will_attack_next_turn)
{
  const Ship a{ 0, 0,{ 0, 0 }, 0.5, 255 };
  ASSERT_TRUE(will_attack_next_turn(a, a));

  const Ship b{ 1, 0,{ 1, 1 }, 0.5, 255 };
  ASSERT_TRUE(will_attack_next_turn(a, b));

  const Ship c{ 0, 0,{ 1, 0 }, 0.5, 255 };
  ASSERT_TRUE(will_attack_next_turn(a, c));

  const Ship d{ 0, 0,{ 5, 0 }, 0.5, 255 };
  ASSERT_TRUE(will_attack_next_turn(a, d));

  const Ship e{ 0, 0,{ 6, 0 }, 0.5, 255 };
  ASSERT_FALSE(will_attack_next_turn(a, e));

  const Ship f{ 0, 0,{ 5.999999, 0 }, 0.5, 255 };
  ASSERT_TRUE(will_attack_next_turn(a, f));
}

TEST(raf_path_finder, edge_distance_to_dock)
{
  // Helper lambda
  const auto make_ship_at_location = [](Vec2d location) {
    return Ship{ 0, 0, location, 0.5, 255 };
  };

  const Planet planet{ 0, 0,{ 0, 0 }, 10, 255, 0 };
  const Ship a = make_ship_at_location({ 0, 0 });
  ASSERT_DOUBLE_EQ(edge_distance_to_dock(a, planet), 0 - (raf::constants::DOCK_RADIUS + 0.5 + 10));

  const Ship b = make_ship_at_location({ 10, 10 });
  ASSERT_DOUBLE_EQ(edge_distance_to_dock(b, planet), std::sqrt(200) - (raf::constants::DOCK_RADIUS + 0.5 + 10));

  const Ship c = make_ship_at_location({ 10, 0 });
  ASSERT_DOUBLE_EQ(edge_distance_to_dock(c, planet), 10 - (raf::constants::DOCK_RADIUS + 0.5 + 10));

  const Ship d = make_ship_at_location({ 15, 0 });
  ASSERT_DOUBLE_EQ(edge_distance_to_dock(d, planet), 0.5);
}

TEST(raf_path_finder, path_finder_find)
{
  // Helper lambda
  const auto make_ship_at_location = [](Vec2d location) {
    return Ship{ 0, 0, location, 0.5, 255 };
  };

  const auto make_planet_at_location = [](Vec2d location) {
    return Planet{ 0, 0, location, 5, 255, 0 };
  };

  const std::vector<Planet> planets{
    make_planet_at_location({ 20, 0 }),
  };

  const std::vector<Ship> ships{
    make_ship_at_location({ 0, 10 }),
    make_ship_at_location({ 0, -10 }),
    make_ship_at_location({ 10, 0 }),
    make_ship_at_location({ -10, 0 }),
  };

  const std::vector<Path> paths;

  const raf::navigation::PathFinder pf{ planets, ships, paths };
  const auto my_ship = make_ship_at_location({ 0, 0 });

  raf::navigation::PathFinderParams params;
  auto result = pf.find(params, my_ship, { 0, 0 });
  ASSERT_TRUE(result.second);
  ASSERT_EQ(result.first.ship_id, my_ship.id());
  ASSERT_DOUBLE_EQ(result.first.start_pos.x(), 0);
  ASSERT_DOUBLE_EQ(result.first.start_pos.y(), 0);
  ASSERT_DOUBLE_EQ(result.first.end_pos.x(), 0);
  ASSERT_DOUBLE_EQ(result.first.end_pos.y(), 0);

  result = pf.find(params, my_ship, { 7, 0 });
  ASSERT_TRUE(result.second);
  ASSERT_EQ(result.first.ship_id, my_ship.id());
  ASSERT_DOUBLE_EQ(result.first.start_pos.x(), 0);
  ASSERT_DOUBLE_EQ(result.first.start_pos.y(), 0);
  ASSERT_DOUBLE_EQ(result.first.end_pos.x(), 7);
  ASSERT_DOUBLE_EQ(result.first.end_pos.y(), 0);

}
