#include "hlt/hlt.hpp"
#include "hlt/navigation.hpp"
#include "raf/raf.hpp"
#include <map>
#include <set>

static const char *BOT_VERSION = "RafBot32";
// TODO
//
// Fix ship crash issue
//
// Tune loop to ensure ships go to nearest planet rather than assign in indexed order.
// https://halite.io/play/?game_id=2207205&replay_class=0&replay_name=replay-20171103-224350%2B0000--3593863604-264-176-1509749022

std::vector<hlt::Ship> find_enemy_ships(
  const hlt::Map& map,
  hlt::PlayerId local_player_id)
{
  std::vector<hlt::Ship> enemy_ships;
  for (const auto &e : map.ships) {
    if (e.first == local_player_id) {
      continue;
    }

    const std::vector<hlt::Ship>& all_ships = e.second;
    // Copy all ships that do not belong to local_player_id
    std::copy_if(
      all_ships.begin(),
      all_ships.end(),
      std::back_inserter(enemy_ships),
      [local_player_id](const hlt::Ship &a) {
      return (a.owner_id != local_player_id);
    });
  }
  return enemy_ships;
}

void sort_planets(
  const std::vector<hlt::Planet>& all_planets,
  hlt::PlayerId local_player_id,
  std::vector<hlt::Planet>& owned_planets,
  std::vector<hlt::Planet>& enemy_planets,
  std::vector<hlt::Planet>& free_planets)
{
  for (const auto &planet : all_planets) {
    if (!planet.owned) {
      free_planets.push_back(planet);
    } else if (planet.owner_id == local_player_id) {
      owned_planets.push_back(planet);
    } else {
      enemy_planets.push_back(planet);
    }
  }
}

std::vector<hlt::Ship> player_docked_ships(
  const hlt::Map& map,
  hlt::PlayerId player_id) {
  const auto &all_ships = map.ships.at(player_id);
  std::vector<hlt::Ship> docked_ships;
  std::copy_if(
    std::begin(all_ships),
    std::end(all_ships),
    std::back_inserter(docked_ships),
    [player_id](const hlt::Ship &a) {
    return (a.owner_id == player_id) && (a.docking_status != hlt::ShipDockingStatus::Undocked);
  });

  return all_ships;
}

std::vector<hlt::Ship> find_threats_to_docked_ships(hlt::Map& map, hlt::PlayerId local_player_id) {
  auto client_docked_ships = player_docked_ships(map, local_player_id);
  auto enemy_ships = find_enemy_ships(map, local_player_id);
  constexpr auto num_turns_to_reach = 3;
  // for each of clients docked ships
  //   find all enemy ships within 3 turns of docked ship
  // return all those ships
  std::vector<hlt::Ship> threat_ships;
  for (const auto& cs : client_docked_ships) {
    for (const auto& es : enemy_ships) {
      if (cs.location.get_distance_to(es.location) <= num_turns_to_reach * hlt::constants::MAX_SPEED) {
        threat_ships.push_back(es);
      }
    }
  }
  return threat_ships;
}

void run_game(hlt::Map& map, hlt::PlayerId local_player_id)
{
  std::vector<hlt::Planet> owned_planets;
  std::vector<hlt::Planet> enemy_planets;
  std::vector<hlt::Planet> free_planets;

  sort_planets(map.planets, local_player_id, owned_planets, enemy_planets, free_planets);

  auto enemy_ships = find_enemy_ships(map, local_player_id);
}

double distance_to(const hlt::Ship& a, const hlt::Ship& b) {
  return a.location.get_distance_to(b.location);
}

bool can_attack(const hlt::Ship& a, const hlt::Ship& b) {
  return a.location.get_distance_to(b.location) < (hlt::constants::WEAPON_RADIUS);
}

int distance_in_turns(const hlt::Entity& a, const hlt::Entity& b) {
  return (a.location.get_distance_to(b.location) / hlt::constants::MAX_SPEED) + 1;
}

int turns_till_attackable(const hlt::Entity& a, const hlt::Entity& b) {
  auto radius_distance = a.location.get_distance_to(b.location) - (hlt::constants::WEAPON_RADIUS);

  if (radius_distance < 0.0) {
    return 0;
  }
  return (radius_distance / hlt::constants::MAX_SPEED) + 1;
}


hlt::Ship nearest_ship(const hlt::Map& map, const hlt::Ship& a, hlt::PlayerId local_player) {
  auto nearest_distance = std::numeric_limits<double>::max();
  hlt::Ship nearest_target;
  for (const auto& e : map.ships) {
    if (e.first == local_player) {
      continue;
    }

    for (const auto& target : e.second) {
      auto distance = a.location.get_distance_to(target.location);
      if (distance < nearest_distance) {
        nearest_distance = distance;
        nearest_target = target;
      }
    }
  }

  return nearest_target;

}

std::vector<hlt::Location> generate_planet_dock_points(const hlt::Planet& planet) {
  const double angular_step_rad = M_PI / 36.0;
  const hlt::Location& location = planet.location;
  const double radius = planet.radius;

  std::vector<hlt::Location> targets;
  for (double angle_radians = 0.0; angle_radians < 2 * M_PI; angle_radians += angular_step_rad) {
    const double new_target_dx = cos(angle_radians + angular_step_rad) * radius;
    const double new_target_dy = sin(angle_radians + angular_step_rad) * radius;
    const hlt::Location new_target = { location.pos_x + new_target_dx, location.pos_y + new_target_dy };
    targets.push_back(new_target);
  }

  return targets;
}

// Spawn point is defined as the nearest part the the center of the map.
hlt::Location planet_spawn_point(const hlt::Map& map, const hlt::Planet& planet) {
  hlt::Location center{ map.map_width / 2, map.map_height / 2 };
  return planet.location.get_closest_point(center, 0.5);
}

// Planet dock points map
// Divides a planet up and generates docking points in order to spread ships out
// Also a cheap hack solution to start of game ship collisions
using PlanetDockMap = std::map<hlt::EntityId, std::vector<hlt::Location>>;
PlanetDockMap planet_dock_points;

std::map<hlt::EntityId, std::set<int> > in_use_points;
hlt::Location get_next_dock_point(const hlt::Map& map, const hlt::Planet& planet) {
  int start_slot = 0;

  // Find first empty slot.
  for (;;) {
    if (!in_use_points.count(start_slot))
      break;
    start_slot++;
  }
  in_use_points[planet.entity_id].insert(start_slot);
  return planet_dock_points[planet.entity_id][start_slot];
}

std::map<hlt::EntityId, std::set<int> > point_in_use;

namespace hlt {
namespace constants {
const int MAX_MAP_WIDTH = 384;
const int MAX_MAP_HEIGHT = 256;
}
}

static int grid_loc(const hlt::Location& location) {
  // Return a comparable unique coordinate.
  return location.pos_x * hlt::constants::MAX_MAP_WIDTH + hlt::constants::MAX_MAP_HEIGHT;
}

hlt::Location get_nearest_spawn_point(const hlt::Entity& target, const hlt::Planet& planet) {
  auto points = planet_dock_points[planet.entity_id];

  std::sort(
    std::begin(points),
    std::end(points),
    [&planet](const hlt::Location& a, const hlt::Location& b) {
    return planet.location.get_distance_to(a) < planet.location.get_distance_to(b);
  });

  for (auto &e : points) {
    auto unique_grid_location = grid_loc(e);
    if (point_in_use[planet.entity_id].find(unique_grid_location) == point_in_use[planet.entity_id].cend())
      continue;

    point_in_use[planet.entity_id].insert(unique_grid_location);
    return e;
  }

  // If we can't find one, return planet location.
  // Should rarely happen...
  return planet.location;
}

void initialise_planet_dock_points(const hlt::Map& map)
{
  for (const auto& p : map.planets) {
    planet_dock_points[p.entity_id] = generate_planet_dock_points(p);
  }
}

void accelerate_by(hlt::Location& loc, int thrust, int angle_in_degrees)
{
  auto angle_rads = angle_in_degrees * M_PI / 180.0;

  loc.pos_x += thrust * std::cos(angle_rads);
  loc.pos_y += thrust * std::sin(angle_rads);
}

static raf::math::Vec2d to_vec2(const hlt::Location& loc) {
  return raf::math::Vec2d(loc.pos_x, loc.pos_y);
}

void update_raf_map_state(raf::game::MapState &map_state, const hlt::Map& map) {
#if 0
  // HLT types directly convert via constructors for now.
  // This dependency will be reduced and this code will be enabled in the future.
  for (const auto &e : map.planets) {
    raf::game::Planet planet(e.entity_id, e.owner_id, to_vec2(e.location), e.radius, e.health, e.docking_spots);
    e.current_production;
    e.docked_ships;
    // e.docking_spots;
    // e.entity_id;
    // e.health;
    // e.location;
    e.owned;
    e.owner_id;
    // e.radius;
    e.remaining_production;
  }
#endif

  for (const auto &e : map.planets) {
    map_state.update(e);
  }

  for (const auto &e : map.ships) {
    for (const auto &f : e.second) {
      map_state.update(f);
    }
  }
}

struct PlayerShipInfo {
  int docked_ships;
  int docking_ships;
  int undocking_ships;
  int undocked_ships;

  int in_docking_state() const {
    return docked_ships + docking_ships + undocking_ships;
  }

  int total() const {
    return docked_ships + docking_ships + undocking_ships + undocked_ships;
  }

  double dock_ratio() const {
    return static_cast<double>(in_docking_state()) / static_cast<double>(total());
  }
};

PlayerShipInfo player_ship_info(const hlt::Map& map, hlt::PlayerId player_id) {
  // Always zero initialise
  PlayerShipInfo info{};
  for (const hlt::Ship& ship : map.ships.at(player_id)) {

    switch (ship.docking_status) {
    case hlt::ShipDockingStatus::Undocked: info.undocked_ships++;
    case hlt::ShipDockingStatus::Docked: info.docked_ships++;
    case hlt::ShipDockingStatus::Docking: info.docking_ships++;
    case hlt::ShipDockingStatus::Undocking: info.undocking_ships++;
    }
  }

  return info;
}

bool should_attack(const hlt::Map& map, hlt::PlayerId player_id) {
  //auto num_ships = map.ships.at(player_id).size();

  return player_ship_info(map, player_id).dock_ratio() > 0.34;
}

// Perform update tasks before running current frame.
void calculated_ships_on_planet(hlt::entity_map<int>& ships_on_planet, const hlt::Map& map) {
  for (const auto& planet: map.planets) {
    ships_on_planet[planet.entity_id] = planet.docked_ships.size();
  }
}

struct DefenceMetrics {
  std::vector<hlt::EntityId> docked_ships;
  std::vector<hlt::EntityId> undocked_ships;
  std::map<hlt::EntityId, std::vector<hlt::EntityId>> protected_by;
  std::set<hlt::EntityId> defenders;
};

DefenceMetrics get_docked_defence_metrics(const hlt::Map& map, hlt::PlayerId player_id)
{
  DefenceMetrics metrics;
  for (const hlt::Ship& ship : map.ships.at(player_id)) {
    if (ship.docking_status == hlt::ShipDockingStatus::Undocked) {
      metrics.undocked_ships.push_back(ship.entity_id);
    } else {
      metrics.docked_ships.push_back(ship.entity_id);
    }
  }

  // For each docked ship, see if an undocked ship is protecting in.
  for (auto id : metrics.docked_ships) {
    const auto &a = map.get_ship(player_id, id);
    for (auto defence : metrics.undocked_ships) {
      const auto &b = map.get_ship(player_id, defence);

      auto distance = distance_to(a, b);
      if (distance < 4.0) {
        metrics.protected_by[id].push_back(defence);
        metrics.defenders.insert(defence);
      }
    }
  }

  return metrics;
}

bool ship_is_defending(const DefenceMetrics &metrics, const hlt::Ship& ship)
{
  if (metrics.defenders.count(ship.entity_id)) {
    return true;
  }
  return false;
}

int docked_defence_count(DefenceMetrics &metrics, const hlt::Ship& ship) {
  return metrics.protected_by[ship.entity_id].size();
}

bool ship_meets_minimum_defence_requirements(DefenceMetrics &metrics, const hlt::Ship& ship) {
  // This ship is defended by at least two ships.
  return docked_defence_count(metrics, ship) >= 2;
}

void new_main() {
  const hlt::Metadata metadata = hlt::initialize(BOT_VERSION);
  const hlt::PlayerId player_id = metadata.player_id;

  const hlt::Map& initial_map = metadata.initial_map;
  initialise_planet_dock_points(initial_map);

  // We now have 1 full minute to analyse the initial map.
  std::ostringstream initial_map_intelligence;
  initial_map_intelligence
    << "width: " << initial_map.map_width
    << "; height: " << initial_map.map_height
    << "; players: " << initial_map.ship_map.size()
    << "; my ships: " << initial_map.ship_map.at(player_id).size()
    << "; planets: " << initial_map.planets.size();
  hlt::Log::log(initial_map_intelligence.str());


  auto num_players = initial_map.ship_map.size();

  auto map_state = raf::game::MapState(
    raf::math::Vec2i(initial_map.map_width, initial_map.map_height),
    metadata.player_id,
    initial_map.ship_map.size()
  );

  update_raf_map_state(map_state, initial_map);

  for (int frame = 1;; frame++) {
    hlt::Map map = hlt::in::get_map();

    map_state.BeginRound(frame);
    update_raf_map_state(map_state, map);

    map_state.pre_frame();
    map_state.run_frame();
    auto moves = map_state.post_frame();

    if (!hlt::out::send_moves(moves)) {
      hlt::Log::log("send_moves failed; exiting");
      break;
    }
  }
}


int main() {
  new_main();
  return 0;

  const hlt::Metadata metadata = hlt::initialize("RafBot11");
  const hlt::PlayerId player_id = metadata.player_id;

  const hlt::Map& initial_map = metadata.initial_map;
  initialise_planet_dock_points(initial_map);

  // We now have 1 full minute to analyse the initial map.
  std::ostringstream initial_map_intelligence;
  initial_map_intelligence
    << "width: " << initial_map.map_width
    << "; height: " << initial_map.map_height
    << "; players: " << initial_map.ship_map.size()
    << "; my ships: " << initial_map.ship_map.at(player_id).size()
    << "; planets: " << initial_map.planets.size();
  hlt::Log::log(initial_map_intelligence.str());

  auto num_players = initial_map.ship_map.size();

  std::vector<hlt::Move> moves;
  for (;;) {
    moves.clear();
    const hlt::Map map = hlt::in::get_map();

    decltype(map.planets) nearest_planets;
    decltype(map.planets) enemy_planets;

    // Filter full planets
    std::copy_if(
      map.planets.begin(),
      map.planets.end(),
      std::back_inserter(nearest_planets),
      [player_id](const hlt::Planet &a) {
      return !(
        // If it's full we aren't interested
        a.is_full()
        // If its owned by another player
        || (a.owned && a.owner_id != player_id)
        // If we own it but there are less than 2 slots free don't bother...
        //|| (a.owned &&
        //  //a.docking_spots - a.docked_ships.size() < 2
        //  static_cast<float>(a.docking_spots) / static_cast<float>(a.docked_ships.size()) > 0.70f
        //  )
        );
    });

    // Find enemy planets
    std::copy_if(
      map.planets.begin(),
      map.planets.end(),
      std::back_inserter(enemy_planets),
      [player_id](const hlt::Planet &a) {
      return (a.owned && a.owner_id != player_id);
    });

    hlt::entity_map<int> ships_on_planet;
    calculated_ships_on_planet(ships_on_planet, map);

    // Iterate through all local player ships that are not docked.
    // and decide which action to perform with them.
    //
    // Docked ships are never undocked once they have been docked.
    auto local_player_ship_info = player_ship_info(map, player_id);
    for (const hlt::Ship& ship : map.ships.at(player_id)) {
      if (ship.docking_status != hlt::ShipDockingStatus::Undocked) {
        continue;
      }

      auto potential_planets = nearest_planets;

      potential_planets.erase(
        std::remove_if(
          std::begin(potential_planets),
          std::end(potential_planets),
          [&ship, &ships_on_planet](const hlt::Planet& a) {
        // If it would take more than N goes to get there don't bother.
        // Remove planets where they will be full from ships already moving there or docked there
        return distance_in_turns(ship, a) > 8 || ships_on_planet[a.entity_id] >= a.docking_spots;// -a.docked_ships.size();
      }),
        std::end(potential_planets)
        );

      std::sort(
        std::begin(potential_planets),
        std::end(potential_planets),
        [&ship](const hlt::Planet& a, const hlt::Planet& b) {
        return ship.location.get_distance_to(a.location) < ship.location.get_distance_to(b.location);
      });

      bool always_attack = num_players == 2;// && map.ships.at(player_id).size() <= 5;
      // Once the docked ratio goes above 33% start attacking.
      always_attack = (local_player_ship_info.dock_ratio() > 0.65);

      if (num_players == 2) {
        always_attack = (local_player_ship_info.dock_ratio() > 0.60);
      } else {
        always_attack = (local_player_ship_info.dock_ratio() > 0.90);
      }

      std::vector<hlt::Ship> enemy_ships;
      // Find enemy ships
      for (const auto& e : map.ships) {
        if (e.first == player_id) {
          continue;
        }


        // Copy all ships that are docked but not ours.
        std::copy_if(
          e.second.begin(),
          e.second.end(),
          std::back_inserter(enemy_ships),
          [player_id](const hlt::Ship &a) {
          return (a.owner_id != player_id) && a.docking_status != hlt::ShipDockingStatus::Undocked;
        });

        // If we're using our 2 player attach strategy, and there are no docked ships
        // target any ships.
        if (always_attack && enemy_ships.empty()) {
          std::copy_if(
            e.second.begin(),
            e.second.end(),
            std::back_inserter(enemy_ships),
            [player_id](const hlt::Ship &a) {
            return (a.owner_id != player_id);
          });
        }
      }

      std::sort(
        std::begin(enemy_ships),
        std::end(enemy_ships),
        [&ship](const hlt::Ship& a, const hlt::Ship& b) {
        return ship.location.get_distance_to(a.location) < ship.location.get_distance_to(b.location);
      });

      if (always_attack) {
        auto distance = std::min_element(
          enemy_ships.cbegin(),
          enemy_ships.cend(),
          [&ship](const hlt::Ship& a, const hlt::Ship& b) {
          return ship.location.get_distance_to(a.location) < ship.location.get_distance_to(b.location);
        });

        // 5 Turns for enemy to dock + 72 / (6 *3) = 4 turns to make a ship + 5 fudge factor = 13
        if (distance_in_turns(ship, *distance) > 13) {
          always_attack = false;
        }
      }



      if (potential_planets.empty() || always_attack) {
        std::sort(
          std::begin(enemy_planets),
          std::end(enemy_planets),
          [&ship](const hlt::Planet& a, const hlt::Planet& b) {
          return ship.location.get_distance_to(a.location) < ship.location.get_distance_to(b.location);
        });

        std::sort(
          std::begin(enemy_ships),
          std::end(enemy_ships),
          [&ship](const hlt::Ship& a, const hlt::Ship& b) {
          return ship.location.get_distance_to(a.location) < ship.location.get_distance_to(b.location);
        });

        // No planets left
        hlt::Log::log("All planets are taken!");

        auto metrics = get_docked_defence_metrics(map, player_id);

        if (!ship_meets_minimum_defence_requirements(metrics, ship)) {
        }


        // Just suicide all remaining ships into enemy planets.
        for (const auto& target : enemy_ships) {

          if (can_attack(ship, target)) {
            // Already attacking, stay still.
            break;
          }

          const hlt::possibly<hlt::Move> move =
            hlt::navigation::navigate_ship_to_dock(
              map,
              ship,
              target,
              hlt::constants::MAX_SPEED,
              moves);

          if (move.second) {
            moves.push_back(move.first);
            break;
          } else {
            hlt::Log::log("No attack move for Ship!");
          }
        }

      } else {
        // Logic to move to nearest available planet with docking...
        for (const hlt::Planet& planet : potential_planets) {
          if (ship.can_dock(planet)) {
            moves.push_back(hlt::Move::dock(ship.entity_id, planet.entity_id));
            break;
          }

          if (ships_on_planet[planet.entity_id] >= planet.docking_spots) {
            hlt::Log::log("Planet full, skipping!");
            continue;
          }

          auto planet_radius_adjusted = planet;
          // Uncomment to make ships dock nearer edge of planet.
          // TODO: Figure out why this works
          // Initial idea was to add DOCK_RADIUS to planet radius to generate nearest dock point.
          // This doesn't work as expected and instead causes ships not to reach planet even if adding 0.1.
          // The default implementation actually only docks right on the DOCK_RADIUS boundary which is odd as this
          // is not taken into account at all in the path finding....
          // planet_radius_adjusted.radius -= 3.0;
          //
          // UPDATE: This is because of the MIN_DISTANCE_FOR_CLOSEST_POINT constant!

          // position to dock
          const bool dock_near_spawn_point = false;
          const hlt::possibly<hlt::Move> move = dock_near_spawn_point ?
            raf::navigate_ship_to_location(map, ship, get_nearest_spawn_point(ship, planet), hlt::constants::MAX_SPEED, moves) :
            hlt::navigation::navigate_ship_to_dock(map, ship, planet, hlt::constants::MAX_SPEED, moves);
          if (move.second) {
            moves.push_back(move.first);
            ships_on_planet[planet.entity_id]++;
            break;
          } else {
            hlt::Log::log("No dock move for Ship!");
          }
        }
      }
    }

    if (!hlt::out::send_moves(moves)) {
      hlt::Log::log("send_moves failed; exiting");
      break;
    }
  }
}
