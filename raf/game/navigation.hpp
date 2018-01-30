#ifndef RAF_GAME_NAVIGATION_H_
#define RAF_GAME_NAVIGATION_H_

#include "collision.hpp"
#include "path.hpp"
#include "entity.hpp"
#include "planet.hpp"
#include "ship.hpp"
#include "../log.hpp"
#include "../types.hpp"
#include "../math/math.hpp"

#include "../stdlib_util.h"

#include <vector>

namespace raf {
  namespace navigation {
    using game::Entity;
    using math::Vec2d;

    static double distance(const Vec2d &a, const Vec2d &b) {
      return (a - b).length();
    }

    static void check_and_add_entity_between(
      std::vector<const Entity *>& entities_found,
      const Vec2d& start,
      const Vec2d& target,
      const Entity& entity_to_check)
    {
      const auto& location = entity_to_check.current_location();
      if (location == start || location == target) {
        return;
      }
      if (collision::segment_circle_intersect(
        start,
        target,
        entity_to_check.current_location(),
        constants::FORECAST_FUDGE_FACTOR,
        entity_to_check.radius())) {
        entities_found.push_back(&entity_to_check);
      }
    }

    static std::vector<const Entity *> objects_between(
      const std::vector<game::Planet> &planets,
      const std::vector<game::Ship> &ships,
      const Vec2d& start,
      const Vec2d& target) {
      std::vector<const Entity *> entities_found;

      for (const Entity& planet : planets) {
        check_and_add_entity_between(entities_found, start, target, planet);
      }

      for (const auto& ship : ships) {
        check_and_add_entity_between(entities_found, start, target, ship);
      }

      return entities_found;
    }

    static bool would_collide_in_transit(
      const std::vector<game::Planet> &planets,
      const std::vector<game::Ship> &ships,
      const Vec2d& start,
      const Vec2d& target,
      const std::vector<game::Path>& pending_moves) {

      for (const auto& move : pending_moves) {
        auto our_vel = target - start;
        auto dist = sqrt(min_dist_squared(
          move.start_pos,
          move.end_pos - move.start_pos,
          start,
          our_vel));
        if (dist < move.radius * 2.1)
        {
          raf::Log("distance : ", dist);
          return true;
        }
      }
      return false;
    }

    // Refactor this
    // Change planets to be a set of planet id's to test against
    // Change ships to be a set of ship id's to test against.
    // Leave pending moves as is
    static possibly<math::Velocity> navigate_ship_towards_target(
      const std::vector<game::Planet> &planets,
      const std::vector<game::Ship> &ships,
      const Vec2d& ship,
      const Vec2d& target,
      const int max_thrust,
      const bool avoid_obstacles,
      const int max_corrections,
      const double angular_step_rad,
      const std::vector<game::Path>& pending_moves)
    {
      if (max_corrections <= 0) {
        return { math::Velocity(0, 0), false };
      }

      raf::Log("navigate: thrust=", max_thrust, ", angular_step_rad=", angular_step_rad);
      raf::Log("ship=" , ship, "target=", target);
      //raf::Log("Planets: ");
      //raf::Log(planets);
      //raf::Log("Ships:");
      //raf::Log(ships);
      //raf::Log("Pending moves:");
      //raf::Log(pending_moves);

      Vec2d adjusted_target = target;
      const double distance = navigation::distance(ship, target);
      double angle_rad = ship.orient_towards_in_rad(target);

      std::vector<game::Ship> stripped_ships;
      for (const auto &e : ships) {
        bool moves_this_frame = false;
        for (const auto &mover : pending_moves) {
          if (mover.ship_id == e.id()) {
            moves_this_frame = true;
          }
        }

        if (!moves_this_frame) {
          stripped_ships.push_back(e);
        }
      }

      if (avoid_obstacles && !objects_between(planets, stripped_ships, ship, target).empty()) {
        bool found = false;
        for (int i = 0; i < max_corrections; i++) {
          // If i is even go clockwise, else go anti clockwise
          // Make sure all intervals are performed both anti and counter clockwise
          double adjustment_angle = (i % 2 == 0) ? angular_step_rad * (i + 2)/2 : -angular_step_rad * (i + 2) / 2;
          const double new_target_dx = cos(angle_rad + adjustment_angle) * distance;
          const double new_target_dy = sin(angle_rad + adjustment_angle) * distance;
          adjusted_target = { ship.x() + new_target_dx, ship.y() + new_target_dy };

          if (objects_between(planets, stripped_ships, ship, adjusted_target).empty()) {
            angle_rad = angle_rad + adjustment_angle;
            found = true;
            break;
          }
        }
        if (!found) {
          return { math::Velocity(0, 0), false };
        }
      }

      //if (avoid_obstacles && !objects_between(planets, ships, ship, target).empty()) {
      //  const double new_target_dx = cos(angle_rad + angular_step_rad) * distance;
      //  const double new_target_dy = sin(angle_rad + angular_step_rad) * distance;
      //  const Vec2d new_target = { ship.x() + new_target_dx, ship.y() + new_target_dy };
      //
      //  return navigate_ship_towards_target(
      //    planets, ships, ship, new_target, max_thrust, true, (max_corrections - 1), angular_step_rad, pending_moves);
      //}

      int thrust;
      if (distance < max_thrust) {
        // Do not round up, since overshooting might cause collision.
        thrust = (int)distance;
      }
      else {
        thrust = max_thrust;
      }

      const int angle_deg = math::angle_rad_to_deg_clipped(angle_rad);

      {
        const double angle_rad = ship.orient_towards_in_rad(adjusted_target);
        const double new_target_dx = cos(angle_rad) * thrust;
        const double new_target_dy = sin(angle_rad) * thrust;
        Vec2d new_target = { ship.x() + new_target_dx, ship.y() + new_target_dy };
        while (would_collide_in_transit(planets, ships, ship, new_target, pending_moves) && thrust > 0) {
          // If a collision would happen, reduce thrust by 1.
          thrust -= 1;
          const double new_target_dx = cos(angle_rad) * thrust;
          const double new_target_dy = sin(angle_rad) * thrust;
          new_target = { ship.x() + new_target_dx, ship.y() + new_target_dy };
        }
      }

      raf::Log("result vector=", math::Velocity(thrust, angle_deg).to_vec());
      return { math::Velocity(thrust, angle_deg), true };
    }

    static possibly<math::Velocity> navigate_ship_to_dock(
      const std::vector<game::Planet> &planets,
      const std::vector<game::Ship> &ships,
      const Entity& ship,
      const Entity& dock_target,
      const int max_thrust,
      const std::vector<game::Path>& pending_moves)
    {
      const int max_corrections = constants::MAX_NAVIGATION_CORRECTIONS;
      const bool avoid_obstacles = true;
      const double angular_step_rad = 2 * M_PI / 180.0;
      const Vec2d& target = math::get_closest_point(dock_target.current_location(), ship.current_location(), dock_target.radius());


      raf::Log("target x:", target.x(), " y:", target.y());
      return navigate_ship_towards_target(
        planets, ships, ship.current_location(), target, max_thrust, avoid_obstacles, max_corrections, angular_step_rad, pending_moves);
    }

    static possibly<math::Velocity> navigate_ship_to_attack(
      const std::vector<game::Planet> &planets,
      const std::vector<game::Ship> &ships,
      const Entity& ship,
      const Entity& target,
      const int max_thrust,
      const std::vector<game::Path>& pending_moves)
    {
      const int max_corrections = constants::MAX_NAVIGATION_CORRECTIONS;
      const bool avoid_obstacles = true;
      const double angular_step_rad = 2 * M_PI / 180.0;
      const Vec2d& target_loc = math::get_closest_point(target.current_location(), ship.current_location(), constants::WEAPON_RADIUS-1.0);

      return navigate_ship_towards_target(planets, ships, ship.current_location(), target_loc, max_thrust, avoid_obstacles, max_corrections, angular_step_rad, pending_moves);
    }

  }
}

#endif // RAF_GAME_COLLISION_H_
