#pragma once

#include "collision.hpp"
#include "log.hpp"
#include "map.hpp"
#include "move.hpp"
#include "util.hpp"

// FIXME remove need for relative includes
// Waiting on halite.io to fix submission process.
#include "../raf/math/math.hpp"

#include <sstream>

namespace hlt {
    namespace navigation {
        static raf::math::Vec2d to_vec2(const hlt::Location& loc) {
          return raf::math::Vec2d(loc.pos_x, loc.pos_y);
        }

        static void check_and_add_entity_between(
                std::vector<const Entity *>& entities_found,
                const Location& start,
                const Location& target,
                const Entity& entity_to_check)
        {
            const Location &location = entity_to_check.location;
            if (location == start || location == target) {
                return;
            }
            if (collision::segment_circle_intersect(start, target, entity_to_check, constants::FORECAST_FUDGE_FACTOR)) {
                entities_found.push_back(&entity_to_check);
            }
        }

        static std::vector<const Entity *> objects_between(const Map& map, const Location& start, const Location& target) {
            std::vector<const Entity *> entities_found;

            for (const Planet& planet : map.planets) {
                check_and_add_entity_between(entities_found, start, target, planet);
            }

            for (const auto& player_ship : map.ships) {
              for (const Ship& ship : player_ship.second) {
                check_and_add_entity_between(entities_found, start, target, ship);
              }
            }

            return entities_found;
        }

        static void accelerate_by(hlt::Location& loc, int thrust, int angle_in_degrees)
        {
          auto angle_rads = angle_in_degrees * M_PI / 180.0;

          loc.pos_x += thrust * std::cos(angle_rads);
          loc.pos_y += thrust * std::sin(angle_rads);
        }

        static bool would_collide_in_transit(
          const hlt::Map& map,
          const Location& start,
          const Location& target,
          const std::vector<Move>& pending_moves) {
          for (const auto& move : pending_moves) {
            if (move.type != MoveType::Thrust) {
              continue;
            }

            for (const auto& player_ship : map.ships) {
              auto ship = std::find_if(player_ship.second.begin(), player_ship.second.end(),
                [&move](const hlt::Ship& ship) -> bool {
                if (move.ship_id == ship.entity_id)
                  return true;
                return false;
              });
              if (ship == player_ship.second.end()) {
                continue;
              }
              // This is a horrible reverse lookup :<
              // We found it

              auto start_pos = ship->location;
              auto velocity = hlt::Location{ 0, 0 };
              // Convert to component vector
              accelerate_by(velocity, move.move_thrust, move.move_angle_deg);

              auto our_vel = to_vec2(target) - to_vec2(start);
              auto dist = sqrt(min_dist_squared(
                to_vec2(start_pos),
                to_vec2(velocity),
                to_vec2(start),
                our_vel));
              if (dist < ship->radius * 2.1)
              {
                std::ostringstream error;
                error << "dist = " << dist;
                hlt::Log::log(error.str());
                return true;
              }
            }
          }
          return false;
        }


        static possibly<Move> navigate_ship_towards_target(
                const Map& map,
                const Ship& ship,
                const Location& target,
                const int max_thrust,
                const bool avoid_obstacles,
                const int max_corrections,
                const double angular_step_rad,
                const std::vector<Move>& pending_moves)
        {
            if (max_corrections <= 0) {
                return { Move::noop(), false };
            }

            const double distance = ship.location.get_distance_to(target);
            const double angle_rad = ship.location.orient_towards_in_rad(target);

            if (avoid_obstacles && !objects_between(map, ship.location, target).empty()) {
                const double new_target_dx = cos(angle_rad + angular_step_rad) * distance;
                const double new_target_dy = sin(angle_rad + angular_step_rad) * distance;
                const Location new_target = { ship.location.pos_x + new_target_dx, ship.location.pos_y + new_target_dy };

                return navigate_ship_towards_target(
                        map, ship, new_target, max_thrust, true, (max_corrections - 1), angular_step_rad, pending_moves);
            }

            int thrust;
            if (distance < max_thrust) {
                // Do not round up, since overshooting might cause collision.
                thrust = (int) distance;
            } else {
                thrust = max_thrust;
            }

            const int angle_deg = util::angle_rad_to_deg_clipped(angle_rad);

            {
              const double angle_rad = ship.location.orient_towards_in_rad(target);
              const double new_target_dx = cos(angle_rad) * thrust;
              const double new_target_dy = sin(angle_rad) * thrust;
              Location new_target = { ship.location.pos_x + new_target_dx, ship.location.pos_y + new_target_dy };
              while (would_collide_in_transit(map, ship.location, new_target, pending_moves) && thrust > 0) {
                // If a collision would happen, reduce thrust by 1.
                thrust -= 1;
                const double new_target_dx = cos(angle_rad) * thrust;
                const double new_target_dy = sin(angle_rad) * thrust;
                new_target = { ship.location.pos_x + new_target_dx, ship.location.pos_y + new_target_dy };
              }
            }

            return { Move::thrust(ship.entity_id, thrust, angle_deg), true };
        }

        static possibly<Move> navigate_ship_to_dock(
                const Map& map,
                const Ship& ship,
                const Entity& dock_target,
                const int max_thrust,
                const std::vector<Move>& pending_moves)
        {
            const int max_corrections = constants::MAX_NAVIGATION_CORRECTIONS;
            const bool avoid_obstacles = true;
            const double angular_step_rad = M_PI / 180.0;
            const Location& target = ship.location.get_closest_point(dock_target.location, dock_target.radius);

            return navigate_ship_towards_target(
                    map, ship, target, max_thrust, avoid_obstacles, max_corrections, angular_step_rad, pending_moves);
        }
    }
}
