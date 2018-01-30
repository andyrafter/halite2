#ifndef RAF_GAME_PATH_FINDER_H_
#define RAF_GAME_PATH_FINDER_H_

#include "collision.hpp"
#include "constants.hpp"
#include "path.hpp"
#include "entity.hpp"
#include "navigation.hpp"
#include "planet.hpp"
#include "ship.hpp"

#include "../stdlib_util.h"
#include <vector>

namespace raf {
namespace navigation {

// need functions that determine different types of distances
// distance_to_point, which calculates an object at origin to a point (which is origin with radius 0)
//    useful for moving an origin to another origin
// edge_distance_to_edge - calculate the minimum distances between edges based on orgin + object radius
//    useful for calculating whether edges will touch

double distance_to_point(const math::Vec2d& a, const math::Vec2d& b) {
  const auto delta = a - b;
  return delta.length();
}

double distance_to_point(const game::Entity& a, const game::Entity& b) {
  return distance_to_point(a.current_location(), b.current_location());
}

// returns negative if within radius
double distance_to_radius(const math::Vec2d& a, const math::Vec2d& b, double radius) {
  return distance_to_point(a, b) - radius;
}

double distance_to_radius(const game::Entity& a, const game::Entity& b, double radius) {
  return distance_to_radius(a.current_location(), b.current_location(), radius);
}

// Calculate the distance to the edge of another entity
// If this is negative then the objects have collided.
double edge_distance_to_edge(const game::Entity& a, const game::Entity& b) {
  //const auto point_distance = distance_to_point(a, b);
  //return point_distance - a.radius() - b.radius();
  const auto entity_radius = a.radius() + b.radius();
  return distance_to_radius(a, b, entity_radius);
}
// alias to
const auto& collision_distance = edge_distance_to_edge;

// Calculate the distance until two ships are in attack range
// If negative they are already in attack range
double weapon_distance_to_edge(const game::Ship& a, const game::Ship& b) {
  //const auto point_distance = distance_to_point(a, b);
  //const auto weapon_radius = constants::WEAPON_RADIUS + a.radius() + b.radius();
  //return point_distance - weapon_radius;
  const auto weapon_radius = constants::WEAPON_RADIUS + a.radius() + b.radius();
  return distance_to_radius(a, b, weapon_radius);
}

bool could_attack_next_turn(const game::Ship& a, const game::Ship& b) {
  // If two ships travel towards each other they will reduce distance by 2 * MAX_SPEED
  // Effectively this filters all ships that are within distance of
  //    2 * MAX_SPEED + a.radius() + b.radius() + WEAPON_RADIUS
  // which equates to
  //    14 + 0.5 + 0.5 + 5 = 20
  return weapon_distance_to_edge(a, b) <= constants::MAX_SPEED * 2.0;
}

// Damage is dealt at start of turn if within radius
// Determines whether two ships are within the radius required for damage to be applied.
bool will_attack_next_turn(const game::Ship& a, const game::Ship& b) {
  // Check the halite engine code to see what their exact calculation is.
  // Some of them don't seem unclear such as WEAPON radius being ship radius +
  // weapon radius rather than just radius from origin.
  return weapon_distance_to_edge(a, b) < 0;
}

double edge_distance_to_dock(const game::Ship& ship, const game::Planet& planet) {
  const auto dock_radius = constants::DOCK_RADIUS + ship.radius() + planet.radius();
  return distance_to_radius(ship, planet, dock_radius);
}


bool is_within_distance_of(const game::Entity& a, const game::Entity& b, double distance);
bool is_within_distance_of(const math::Vec2d& a, const math::Vec2d& b, double distance);
bool is_within_distance_of(const game::Entity& a, const math::Vec2d& b, double distance);


// Default to 10 turns of radius
const double DEFAULT_PLANET_LOOKAHEAD_RADIUS = constants::MAX_SPEED * 10.0;
// Default to the maximum two ships could move towards each other + their radii + some fudge
const double DEFAULT_SHIP_COLLISION_LOOKAHEAD_RADIUS = (constants::MAX_SPEED * 2.0) + (2 * constants::SHIP_RADIUS) + 1.0;
const double DEFAULT_SHIP_WEAPON_LOOKAHEAD_RADIUS = (constants::MAX_SPEED * 2.0) + (2 * constants::SHIP_RADIUS) + constants::WEAPON_RADIUS + 1.0;

struct PathFinderParams {
  // Lookahead radius for planets
  // Planets beyond this radius will be discarded for purposes of path
  // adjustment and collision testing
  double planet_lookahead_radius = DEFAULT_PLANET_LOOKAHEAD_RADIUS;

  // Lookahead radius for ships
  // Ships beyond this radius will be discarded for purposes of path
  // adjustment and collision testing
  double ship_collision_lookahead_radius = DEFAULT_SHIP_COLLISION_LOOKAHEAD_RADIUS;
  double ship_weapon_lookahead_radius = DEFAULT_SHIP_WEAPON_LOOKAHEAD_RADIUS;

  // Maximum speed to move the entity
  int max_move_speed = constants::MAX_SPEED;

  // Avoid locations that would lead to being attacked
  bool avoid_attack = false;

  // Use predicted entity location for enemy ships when avoiding attack and collision
  bool use_prediction = false;
};

// Path finder todo
// Strip planets not in the direction of travel
// - Could get buggy if doing a 180
// - Potentially reduce lookahead for things behind to 2 * MAX_SPEED instead.
class PathFinder {
public:
  // Create a path finder context
  // planets are all planets to test against
  // ships is all ships to test against
  // @param planets
  // @param ships static location of ships at start of frame
  // @param pending_paths any pending paths the finder should take into account.
  //        if a ship as a path pending, then its static location will not be
  //        tested for a collision, and the in transit collision algorithm will
  //        be used instead.
  using planet_container = std::vector<game::Planet>;
  using ship_container = std::vector<game::Ship>;
  using pending_path_container = std::vector<game::Path>;

  // EUGH, might ditch this holding a reference idea.
  /// Pointers would potentially solve it however break constness
  PathFinder(const planet_container& planets, const ship_container& ships, const pending_path_container&& pending_paths) = delete;
  PathFinder(const planet_container& planets, const ship_container&& ships, const pending_path_container&& pending_paths) = delete;
  PathFinder(const planet_container&& planets, const ship_container&& ships, const pending_path_container&& pending_paths) = delete;

  // how do you stop binding of temporaries?
  // Seems the best way is require explicit passing to find
  // Even a params struct has the same issue?
  // Maybe pointers are better as you can't take the address of a temporary
  PathFinder(
    const planet_container& planets,
    const ship_container& ships,
    const pending_path_container& pending_paths)
    : planets_(planets),
    ships_(ships),
    pending_paths_(pending_paths)
  {
  }

  // Entity based path finding.
  possibly<game::Path> find(const PathFinderParams& params, const game::Ship& ship, const game::Entity& target);

  possibly<game::Path> find(const PathFinderParams& params, const game::Ship& ship, const math::Vec2d& target) const {
    const auto has_pending_path = pending_paths_ids();
    auto in_range_planets = planets_in_range(ship, params.planet_lookahead_radius);

    auto max_range = std::max(params.ship_collision_lookahead_radius, params.ship_weapon_lookahead_radius);
    auto in_range_ships = ships_in_range(ship, max_range);
    auto ship_origin = ship.current_location();

    // Filter out ships with paths pending.
    // These will go through a seperate algorithm for determing if there is a
    // collision during movement
    raf::filter(in_range_ships, [&has_pending_path](const game::Ship& ship) {
      return has_pending_path.count(ship.id());
    });

    auto move_result = navigation::navigate_ship_towards_target(
      in_range_planets,
      in_range_ships,
      ship_origin,
      target,
      params.max_move_speed,
      true,
      constants::MAX_NAVIGATION_CORRECTIONS,
      math::degrees_to_rads(2),
      pending_paths_);

    return { game::Path(ship, move_result.first), move_result.second };
  }

private:

  // Update map of ships with pending paths.
  std::map<game::EntityId, bool> pending_paths_ids() const {
    std::map<game::EntityId, bool> has_pending_path;
    for (const auto& e : pending_paths_) {
      has_pending_path[e.ship_id] = true;
    }
    return has_pending_path;
  }

  std::vector<game::Planet> planets_in_range(const game::Ship& ship, double range) const {
    std::vector<game::Planet> in_range_planets;
    std::copy_if(
      std::begin(planets_),
      std::end(planets_),
      std::back_inserter(in_range_planets),
      [range, &ship](const game::Planet& p)
    {
      return ship.distance_to_edge(p) < range;
    });

    return in_range_planets;
  }

  std::vector<game::Ship> ships_in_range(const game::Ship& ship, double range) const {
    std::vector<game::Ship> in_range_ships;
    std::copy_if(
      std::begin(ships_),
      std::end(ships_),
      std::back_inserter(in_range_ships),
      [range, &ship](const game::Ship& s)
    {
      // Filter self
      return ship.id() != s.id() && ship.distance_to_edge(s) < range;
    });

    return in_range_ships;
  }

  const planet_container& planets_;
  const ship_container& ships_;
  const pending_path_container& pending_paths_;

  std::map<game::EntityId, bool> has_pending_path_;
};

// As above but returns stats on what is in an area.
class AreaSearch {
};

}
}

#endif // RAF_GAME_PATH_FINDER_H_
