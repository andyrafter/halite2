#include "map_state.hpp"

#include <algorithm>
#include <map>
#include <set>
#include <utility>
#include <vector>

#include "../stdlib_util.h"
// Production mechanics
//
// 72 units to produce a ship
// 1 ship has 6 production
// = 12 turns per ship
//
// 2 ships have 12 production
// = 6 turns per ship
//
// 3 ships have 18 production
// = 4 turns per ship
//
// 4 ships have 24 production
// = 3 turns per ship
//
// 5 turns to dock
//

// Ship attack mechanics
// Total dealt / damage received per ship is the amount of turns a group could
// survive
//
// 1 vs 1
// 64 damage dealt per ship | 64 damage received per ship
// 64 total dealt           | 64 total received
// Turns to kill 4
// Will die after 1st encounter
//
// 2 vs 1
// 64 damage dealt per ship | 32 damage received per ship
// 128 total dealt          | 64 total received
// Turns to kill enemy 2
// Will die after 4th encounter
//
// 3 vs 1
// 64 damage dealt per ship | 21 damage received per ship
// 192 total dealt          | 64 total received
// Turns to kill 2
// Will die after 9th encounter?
// How this works depends on rounding and the game mechanics...
// As the 1 is alive for round two, does it do its 64 damage first to all
// within radius or die in some sequence where the first ship kills it?
//
// 4 vs 1
// 64 damage dealt per ship | 16 damage received per ship
// 256 total dealt          | 64 total received
// Will die after 16th encounter
//
// 8 vs 1
// 64 damage dealt per ship | 8 damage received per ship
// 256 total dealt          | 64 total received
// Will die after 32nd encounter


// Buggy games
//
// Ships don't dock?!
// https://halite.io/play/?game_id=3042881&replay_class=0&replay_name=replay-20171121-095013%2B0000--556159876-288-192-1511257813
//
// Ships don't do anything if too far away.
// https://halite.io/play/?game_id=3042915&replay_class=0&replay_name=replay-20171121-095104%2B0000--605580875-336-224-1511257862
// https://halite.io/play/?game_id=3047953&replay_class=0&replay_name=replay-20171121-112843%2B0000--2171685277-384-256-1511263723
//
// 4 Player bot is too aggressive
// https://halite.io/play/?game_id=3042924&replay_class=0&replay_name=replay-20171121-095117%2B0000--576225040-312-208-1511257833
//
// Chasing ship of player that has no chance of winning!
// https://halite.io/play/?game_id=3045539&replay_class=0&replay_name=replay-20171121-103939%2B0000--3518605138-384-256-1511260775
// Useful feature... Send ship to nearest corner of map from starting location
// or average location of all owned ships / planets...
// This guy does exactly this - ajhofmann
// https://halite.io/play/?game_id=3056203&replay_class=0&replay_name=replay-20171121-141659%2B0000--3667243161-360-240-1511273809

// Crashing into own planets?
// https://halite.io/play/?game_id=3621514&replay_class=0&replay_name=replay-20171129-112318%2B0000--1555087285-312-208-1511954596
//
// V23...
// https://halite.io/play/?game_id=3639882&replay_class=0&replay_name=replay-20171129-173105%2B0000--2144081290-288-192-1511976660
// Seems to be crashes caused by docking. Implement seperated dock points to prevent this.
//
// Why do they dock?
// https://halite.io/play/?game_id=3639909&replay_class=0&replay_name=replay-20171129-173223%2B0000--2226701944-288-192-1511976743


// Do a 4 player strategy like this
// https://halite.io/play/?game_id=3648070&replay_class=1&replay_name=replay-20171129-203439%2B0000--121279244-312-208-1511987522
//

// Bugs
// Ships just stop?
// replay-20171201-145912-0000--2625979251-360-240-1017826

// Ideas
//
// Attack target algorithm
// ----------------------------------------------------------------------------
// For each ship
//  Find a target and vote on it
// Move ships towards target based on distance to target
// Ensure that ships that ahead slow down to allow those further back in the vote
// a chance to catch up and maximise attack group size
// Don't maximise beyond 4 ships!
//
//
// Attack group algorithm
// ----------------------------------------------------------------------------
// Requires adding on planet spawn ID to ship info.
//
// Ensure no planets are left to dock on, ie. don't start attacking too early!
//  Find a ship to attack
//  Wait at current planet until a new ship spawns nearby to attack with
//   EXTENSION: OR? Calculate next planet to spawn in direction of enemy target and move towards that?
//  When ship spawns vote on attack will be two
// Move towards target
//
//
// Post processing
// ----------------------------------------------------------------------------
// Run a pass across all ships with the moves list
// Adjust moves with similar targets/directions in order to coordinate better
//


namespace raf {
namespace game {

hlt::Move from_velocity(const game::Entity& entity, const math::Velocity& vel) {
  return hlt::Move::thrust(entity.id(), vel.thrust(), vel.angle_in_degrees());
}


#if 0
  void MapState::UpdateEntity();
  void MapState::UpdateEntity(const PlanetSnapshot& planet_snapshot) {
    auto planet = planets_.find(planet_snapshot.entity_id);
    if (planet == std::end(planets_)) {
      game::Planet new_planet{
        planet_snapshot.entity_id,
        planet_snapshot.owner_id,
        planet_snapshot.position,
        planet_snapshot.radius,
        planet_snapshot.health,
        planet_snapshot.docking_spots };
      planet_snapshot.update_planet(new_planet);
      planets_.emplace(planet_snapshot.entity_id, new_planet);
    } else {
      planet_snapshot.update_planet(planet->second);
    }
  }

  void MapState::UpdateEntity(const ShipSnapshot& ship_snapshot) {
    auto ship = ships_.find(ship_snapshot.entity_id);
    if (ship == std::end(ships_)) {
      game::Ship new_ship{
        ship_snapshot.entity_id,
        ship_snapshot.owner_id,
        ship_snapshot.position,
        ship_snapshot.radius,
        ship_snapshot.health };
      ship_snapshot.update_ship(new_ship);
      ships_.emplace(ship_snapshot.entity_id, new_ship);
    } else {
      ship_snapshot.update_ship(ship->second);
    }
  }
#endif

#if 0
  game::Planet nearest_planet_to(const game::Entity& entity) {
    return players_.at(0).nearest_planet_to(entity);
  }
#endif

// Update
// Take a new snapshot of data and apply it to existing persistant data
// If the new entity does not exist then create it, else update it.
void MapState::update(const hlt::Planet& planet) {
  const auto& p = planets_.insert({ planet.entity_id, planet });
  if (!p.second) {
    p.first->second.update(planet);
  }

  mark_valid(p.first->second);
}

void MapState::update(const hlt::Ship& ship) {
  const auto &s = (ship.owner_id == local_player_id_) ?
    player_ships_.insert({ ship.entity_id, ship }) :
    enemy_ships_.insert({ ship.entity_id, ship });

  if (!s.second) {
    s.first->second.update(ship);
  }

  mark_valid(s.first->second);
}

void MapState::mark_valid(const Planet& planet) {
  valid_planets_.insert(planet.id());
}

void MapState::mark_valid(const Ship& ship) {
  valid_ships_.insert(ship.id());
  valid_players_.insert(ship.owner());
}

bool MapState::is_valid(const Planet& planet) const {
  return valid_planets_.find(planet.id()) != std::end(valid_planets_);
}

bool MapState::is_valid(const Ship& ship) const {
  return valid_ships_.find(ship.id()) != std::end(valid_ships_);
}

bool MapState::can_dock_more(game::EntityId planet_id) const {
  if (heading_to_planet_.find(planet_id) == std::end(heading_to_planet_)) {
    return !planets_.at(planet_id).is_full();
  }
  return heading_to_planet_.at(planet_id).size() < planets_.at(planet_id).free_docking_spots();
}

void MapState::prune_dead_entities() {
  map_erase_if(player_ships_, [this](const Ship& ship) { return !is_valid(ship); });
  map_erase_if(enemy_ships_, [this](const Ship& ship) { return !is_valid(ship); });
  map_erase_if(planets_, [this](const Planet& planet) { return !is_valid(planet); });
}

//
// Move to algos.h
//
// TODO: Add nearby planet info
// +2 score for a free planet within 28 radius of spawn point
// +1 score for a free planet within 56 radius of spawn point

class PlanetPerimeterInfo {
public:
  PlanetPerimeterInfo(
    double attack_radius,
    double dock_radius,
    PlayerId player_id,
    const Planet& planet,
    const std::vector<Planet>& planets,
    const std::vector<Ship>& ships,
    int map_width,
    int map_height) :
    attack_radius_(attack_radius),
    dock_radius_(dock_radius),
    planet_id_(planet.id()),
    player_id_(player_id)
  {
    const auto spawn = spawn_point(planet, map_width, map_height);
    docking_score = planet.total_docking_spots();
    const auto center_point = math::Vec2d(map_width / 2, map_height / 2);
    distance_to_center_ = ((planet.current_location() - center_point).length() / center_point.length()) * 5.0f;
    distance_to_center_ *= distance_to_center_;
    for (const auto& target : planets) {
      const auto distance = target.distance_to(spawn);

      if (distance <= 28.0f) {
        const auto num_spots = target.total_docking_spots();
        const auto score_factor = (!target.is_owned() || target.owner() == player_id_) ? num_spots : -2;
        docking_score += score_factor;
      } else if (distance <= 70.0f) {
        const auto score_factor = (!target.is_owned() || target.owner() == player_id_) ? 1 : -1;
        docking_score += score_factor;
      }
    }

    raf::Log("planet_id_=", planet_id_, ", docking_score=", docking_score, ", distance_to_center=", distance_to_center_);

    for (const auto& target : ships) {
      const auto distance = planet.distance_to(target);
      if (distance <= attack_radius) {
        if (target.owner() == player_id_) {
          friendly_in_attack_radius.push_back(target.id());
        } else {
          enemy_in_attack_radius.push_back(target.id());
        }
      }

      if (distance <= dock_radius) {
        if (target.owner() == player_id_) {
          friendly_in_dock_radius.push_back(target.id());
        } else {
          enemy_in_dock_radius.push_back(target.id());
        }
      }
    }
  }

  double attack_ratio() const {
    const auto friendly = friendly_in_attack_radius.size();
    const auto enemy = enemy_in_attack_radius.size();
    auto sum = friendly + enemy;
    return sum > 0 ? friendly / sum : 0.0;
  }

  double dock_ratio() const {
    const auto friendly = friendly_in_dock_radius.size();
    const auto enemy = enemy_in_dock_radius.size();
    auto sum = friendly + enemy;
    // If there is nothing in the dock ratio return 1.0, i.e safe.
    return sum > 0 ? friendly / sum : 1.0;
  }

  int score_based_on_distance_in_turns(int turns) const {
    return turns - docking_score;
  }

  int score_based_on_distance_to_center() const {
    return static_cast<int>(distance_to_center_);
  }

//private:
  int docking_score;
  const double attack_radius_;
  const double dock_radius_;
  const EntityId planet_id_;
  const PlayerId player_id_;
  double distance_to_center_;
  std::vector<EntityId> enemy_in_attack_radius;
  std::vector<EntityId> friendly_in_attack_radius;
  std::vector<EntityId> enemy_in_dock_radius;
  std::vector<EntityId> friendly_in_dock_radius;
};

std::vector<PlanetPerimeterInfo> get_planet_info(
  const std::vector<game::Planet>& planets,
  const std::vector<game::Ship>& ships,
  game::PlayerId player_id,
  double threat_radius,
  double dock_radius,
  int map_width,
  int map_height) {
  std::vector<PlanetPerimeterInfo> planet_info;
  for (const auto& planet : planets) {
    PlanetPerimeterInfo info(threat_radius, dock_radius, player_id, planet, planets, ships, map_width, map_height);
    planet_info.emplace_back(info);
  }

  return planet_info;
}


std::vector<game::Planet> dockable_planets(
  const std::map<game::EntityId, game::Planet>& planets,
  game::PlayerId player_id) {
  std::vector<game::Planet> dockable;

  raf::transform_if(
    planets.cbegin(),
    planets.cend(),
    std::back_inserter(dockable),
    [player_id](const game::Planet& planet) {
    return game::is_dockable(planet, player_id);
  });

  return dockable;
}

std::vector<game::Planet> opponent_planets(
  const std::map<game::EntityId, game::Planet>& planets,
  game::PlayerId player_id) {
  std::vector<game::Planet> opponent_owned;

  raf::transform_if(
    planets.cbegin(),
    planets.cend(),
    std::back_inserter(opponent_owned),
    [player_id](const game::Planet& planet) {
    return game::is_owned_by_opponent(planet, player_id);
  });

  return opponent_owned;
}

std::vector<Ship> find_movable_ships(
  const std::map<EntityId, Ship>& ships,
  hlt::PlayerId local_player_id)
{
  std::vector<Ship> movable;
  for (const auto &e : ships) {
    const auto &ship = e.second;
    // If it isn't ours, we can't move it
    if (ship.owner() != local_player_id) {
      continue;
    }

    // If it isn't undocked we cant't move it
    if (!ship.is_undocked()) {
      continue;
    }

    if (!ship.is_alive()) {
      continue;
    }
    movable.push_back(ship);
  }
  return movable;
}

std::vector<Ship> find_enemy_ships(
  const std::map<EntityId, Ship>& ships,
  hlt::PlayerId local_player_id)
{
  std::vector<Ship> enemy_ships;
  for (const auto &e : ships) {
    const auto &ship = e.second;
    if (ship.owner() == local_player_id) {
      continue;
    }

    if (!ship.is_alive()) {
      continue;
    }
    enemy_ships.push_back(ship);
  }
  return enemy_ships;
}

std::vector<Ship> find_threats_to_ship(
  const Ship& target,
  const std::map<EntityId, Ship>& ships,
  hlt::PlayerId local_player_id,
  int max_distance_in_turns)
{
  std::vector<Ship> threats;
  for (const auto &e : ships) {
    const auto &ship = e.second;
    if (ship.owner() == local_player_id) {
      continue;
    }

    if (!ship.is_alive()) {
      continue;
    }

    // Don't treat ships that can't attack as threats.
    if (!ship.is_undocked()) {
      continue;
    }

    if (ship.distance_min_turns(target) > max_distance_in_turns) {
      continue;
    }
    threats.push_back(ship);
  }
  return threats;
}


std::vector<Ship> ships_within_range(
  const Ship& target,
  const std::map<EntityId, Ship>& ships,
  hlt::PlayerId player_id,
  int max_distance_in_turns)
{
  std::vector<Ship> nearby;
  for (const auto &e : ships) {
    const auto &ship = e.second;
    if (ship.owner() == player_id) {
      continue;
    }

    if (!ship.is_alive()) {
      continue;
    }

    if (ship.distance_min_turns(target) > max_distance_in_turns) {
      continue;
    }
    nearby.push_back(ship);
  }
  return nearby;
}

void MapState::pre_frame() {
  queued_moves_.clear();
  pending_paths_.clear();
  prune_dead_entities();
  heading_to_planet_.clear();
  heading_to_attack_.clear();
  moved_ships_.clear();
  // Update the heading lists
  // Erase any ships that have now docked
  // Erase any ships that died
  for (const auto& p : heading_to_planet_) {
    const auto& planet = planets_.at(p.first);
    for (auto s = std::begin(p.second); s != std::end(p.second); ) {
      const auto& ship = player_ships_.at(*s);

      if (!ship.is_undocked()) {
        // Ship is no longer marked as undocked
        // Remove from heading to planet list
        s = heading_to_planet_.at(p.first).erase(s);


        if (planet.is_docked(ship.id())) {
          // Ship transitioned to docked
        } else if (!ship.is_alive()) {
          // Ship transitioned to death
        } else {
          // assert(false);
        }

      } else {
        s++;
      }
    }
  }
}

struct MapResourceInfo {
  int total_docking_spots;
  int total_planets;
  std::map<game::PlayerId, int> docked_ships;
  std::map<game::PlayerId, int> owned_planets;
  std::set<game::PlayerId> players;
  int num_players_with_planets() const { return players.size(); }

  double player_planet_ratio(game::PlayerId a, game::PlayerId b) const {
    if (owned_planets.at(b) == 0) {
      return owned_planets.at(a);
    }
    return static_cast<double>(owned_planets.at(a)) /
      static_cast<double>(owned_planets.at(b));
  }

  double total_planet_ratio(game::PlayerId a) const {
    return static_cast<double>(owned_planets.at(a)) /
      static_cast<double>(total_planets);
  }

  double player_docked_ratio(game::PlayerId a, game::PlayerId b) const {
    if (docked_ships.at(b) == 0) {
      return docked_ships.at(a);
    }
    return static_cast<double>(docked_ships.at(a)) /
      static_cast<double>(docked_ships.at(b));
  }

  double total_docked_ratio(game::PlayerId a) const {
    return static_cast<double>(docked_ships.at(a)) /
      static_cast<double>(total_docking_spots);
  }

  int player_total_docked_ships(game::PlayerId a) {
    return docked_ships[a];
  }

};

static MapResourceInfo map_resource_info(
  const std::map<game::EntityId, game::Planet>& planets) {
  MapResourceInfo info{};

  for (const auto& pair : planets) {
    const auto &planet = pair.second;

    if (planet.is_owned()) {
      info.owned_planets[planet.owner()]++;
      info.docked_ships[planet.owner()] += planet.used_docking_spots();
      info.players.insert(planet.owner());
    }

    info.total_docking_spots += planet.total_docking_spots();
    info.total_planets++;
  }

  return info;
}

struct PlayerShipInfo {
  int docked_ships = 0;
  int docking_ships = 0;
  int undocking_ships = 0;
  int undocked_ships = 0;
  math::Vec2d average_docked_location;
  math::Vec2d average_undocked_location;
  math::Vec2d average_location;

  int undocked() const {
    return undocked_ships;
  }

  int in_docking_state() const {
    return docked_ships + docking_ships + undocking_ships;
  }

  int total_ships() const {
    return docked_ships + docking_ships + undocking_ships + undocked_ships;
  }

  double dock_ratio() const {
    return static_cast<double>(in_docking_state()) / static_cast<double>(total_ships());
  }
};

static PlayerShipInfo player_ship_info(
  const std::map<game::EntityId, game::Ship>& ships,
  game::PlayerId player_id) {
  // Always zero initialise
  PlayerShipInfo info{};
  for (const auto& pair : ships) {
    const auto& ship = pair.second;

    if (ship.owner() != player_id) {
      continue;
    }

    if (ship.is_docked()) {
      info.docked_ships++;
      info.average_docked_location += ship.current_location();
    } else if (ship.is_undocked()) {
      info.undocked_ships++;
      info.average_undocked_location += ship.current_location();
    } else if (ship.is_undocking()) {
      info.undocking_ships++;
      info.average_docked_location += ship.current_location();
    } else if (ship.is_docking()) {
      info.docking_ships++;
      info.average_docked_location += ship.current_location();
    }

    info.average_location += ship.current_location();
  }

  info.average_docked_location /= info.in_docking_state();
  info.average_undocked_location /= info.undocked();
  info.average_location /= info.total_ships();

  return info;
}

PlayerId nearest_threat_to(const std::map<PlayerId, PlayerShipInfo>& player_info, PlayerId player_id) {
  auto player_avg_loc = player_info.at(player_id).average_location;
  double best_distance = 99999999;
  PlayerId nearest = INVALID_ENTITIY_ID;
  for (const auto &e : player_info) {
    if (e.first == player_id) {
      continue;
    }

    const auto diff = player_avg_loc - e.second.average_location;
    if (diff.length_squared() < best_distance) {
      best_distance = diff.length_squared();
      nearest = e.first;
    }
  }
  return nearest;
}


std::vector<EntityId> nearest_safe_planets() {
  return std::vector<EntityId>();
}

void MapState::run_frame() {
  raf::Log("New Frame: ", current_round_);
  current_round_++;

  raf::Log("Num planets: ", planets_.size());
  raf::Log("Num player ships: ", player_ships_.size());
  raf::Log("Num enemy ships: ", enemy_ships_.size());
  raf::Log("Valid ships : ");
  //for (const auto &id : valid_ships_) {
  //  raf::Log(" ", id);
  //}
  //
  //raf::Log("Player ships : ");
  //for (const auto &ship : player_ships_) {
  //  raf::Log(" ", ship.first, "=", ship.second.id());
  //}
  //
  //raf::Log("Enemy ships  : ");
  //for (const auto &ship : enemy_ships_) {
  //  raf::Log(" ", ship.first, "=", ship.second.id());
  //}

  raf::Log("Processing headed queue.");

  //for (const auto& p : heading_to_planet_) {
  //  const auto& planet = planets_.at(p.first);
  //  for (const auto & s : p.second) {
  //    // Continue heading
  //    const auto& ship = player_ships_.at(s);
  //    if (can_dock(planet, ship)) {
  //      // Add command to dock.
  //      auto move = hlt::Move::dock(ship.id(), planet.id());
  //      queued_moves_.push_back(move);
  //      raf::Log("Ship ", ship.id(), " docking to planet ", planet.id());
  //    }
  //  }
  //}

  // Change this to just pass ID's around and dependents can use an API to query data.
  std::vector<Ship> all_ships = get_all_ships();
  const auto planets_vector = get_all_planets();

  auto map_info = map_resource_info(planets_);
  //auto player_info = player_ship_info(player_ships_, local_player_id_);

  std::map<PlayerId, PlayerShipInfo> player_info;

  player_info[local_player_id_] = player_ship_info(player_ships_, local_player_id_);

  for (auto e : valid_players_) {
    if (e == local_player_id_) {
      continue;
    }
    player_info[e] = player_ship_info(enemy_ships_, e);
  }

  auto nearest_attacking_opponent = nearest_threat_to(player_info, local_player_id_);
  //if (nearest_attacking_opponent == INVALID_ENTITIY_ID) {
  //  raf::Log("Inconsistent game state: nearest_attacking_opponent == INVALID_ENTITIY_ID");
  //  return;
  //}

  // For each planet opponent owns
  //  Compute distance to average location
  //  Rank in order of weakness from weakest to strongest
  // Move to weakest first.

  //auto dockable = dockable_planets(planets_, local_player_id_);
  auto dockable = planets_vector;
  for (auto &e : player_ships_) {
    const auto& ship = e.second;
    auto potential_planets = dockable;
    raf::Log("ship id ", ship.id());
    if (!ship.is_alive()) {
      raf::Log("Is dead", ship.id());
      continue;
    }

    if (!ship.is_undocked()) {
      // Check for enemy threats.
      auto threats = find_threats_to_ship(ship, enemy_ships_, local_player_id_, 4);

      std::sort(
        std::begin(threats),
        std::end(threats),
        [&ship](const Ship& a, const Ship& b) {
        return ship.distance_to(a) < ship.distance_to(b);
      });

      for (const auto& threat : threats) {
        // Calculate the time taken for the threat to reach our docked ship
        auto threat_distance = threat.distance_min_turns(ship);
        auto movable = find_movable_ships(player_ships_, local_player_id_);
        // Filter anything that has already been moved.
        filter(movable, [this](const Ship&a) { return has_already_moved(a); });
        // Filter anything that is too far away.
        filter(movable, [&ship](const Ship&a) { return ship.distance_min_turns(a) > 4; });

        // If there are not many ships to use as defence, defend the mid point.
        // Once the ship mass grows attack instead.
        bool defend_mid_point = movable.size() <= 3;
        for (const auto& defender : movable) {
          possibly<math::Velocity> velocity = { { 0,0 }, false };
          if (defend_mid_point) {
            auto mid_point = (ship.current_location() + threat.current_location()) / 2;
            // standard move
            velocity =
              navigation::navigate_ship_towards_target(
                planets_vector,
                all_ships,
                defender.current_location(),
                mid_point,
                constants::MAX_SPEED,
                true,
                constants::MAX_NAVIGATION_CORRECTIONS,
                math::degrees_to_rads(2),
                pending_paths_);
          } else {
            // standard move
            velocity =
              navigation::navigate_ship_to_attack(
                planets_vector,
                all_ships,
                defender,
                threat,
                constants::MAX_SPEED,
                pending_paths_);
          }

          if (velocity.second) {
            move(defender, velocity.first);
            defend(defender, ship, threat);
            raf::Log("DEFEND: Moving ship ", defender.id(), " towards target", threat.id());
            // This currently only moves one ship to defend each docked ship..
            // Potentially want to move all ships if possible...
          } else {
            raf::Log("DEFEND: no move for Ship!");
          }
        }

      }
      continue;
    }

    // Ship has already moved
    if (has_already_moved(ship)) {
      continue;
    }

    //filter(potential_planets, [&ship, this](const game::Planet& a) {
    //  // If it would take more than N goes to get there don't bother.
    //  return ship.distance_to_edge_min_turns(a) > 10 || !can_dock_more(a.id());
    //});

    filter(potential_planets, [&ship, this](const game::Planet& a) {
      // if it is owned by us, then use our tracking function
      if (a.is_owned() && a.owner() == local_player_id_) {
        return !can_dock_more(a.id());
      }
      return false;
    });

    auto planet_info = get_planet_info(potential_planets, all_ships, local_player_id_, 25.0, 49.0, dimensions_.x(), dimensions_.y());

    const auto current_alive_players = num_players();
    std::sort(
      std::begin(potential_planets),
      std::end(potential_planets),
      [&ship, planet_info, current_alive_players](const game::Planet& a, const game::Planet& b) {
      // If A is less than B pick A
      auto a_info = std::find_if(std::begin(planet_info), std::end(planet_info), [a](const PlanetPerimeterInfo& x) {
        return a.id() == x.planet_id_;
      });;

      auto b_info = std::find_if(std::begin(planet_info), std::end(planet_info), [b](const PlanetPerimeterInfo& x) {
        return b.id() == x.planet_id_;
      });;

      
      auto a_score = a_info->score_based_on_distance_in_turns(a.distance_min_turns(ship));
      auto b_score = b_info->score_based_on_distance_in_turns(b.distance_min_turns(ship));

      if (current_alive_players == 4) {
        // Further away planets will have higher scores
        a_score -= a_info->score_based_on_distance_to_center();
        b_score -= b_info->score_based_on_distance_to_center();
      } else if (current_alive_players == 3) {
      } else if (current_alive_players == 2) {
        //// More central planets will have lower scores
        //a_score += a_info->score_based_on_distance_to_center();
        //b_score += b_info->score_based_on_distance_to_center();
      }

      if (a_score < b_score) {
        return true;
      }
      return false;

      auto a_factor = ship.distance_to_edge_min_turns(a) + (2 * b.total_docking_spots()) - 1;
      auto b_factor = ship.distance_to_edge_min_turns(b) + (2 * a.total_docking_spots()) - 1;
      if (a_factor < b_factor) {
        return true;
      } else if (a_factor > b_factor) {
        // A is further, pick B
        return false;
      } else {
        // Favour the ship with the largest radius.
        return a.radius() > b.radius();
      }
    });

    enum State {
      Attempt_dock,
      Attack_docked_enemy,
    };

    State ship_state = Attempt_dock;
    bool action_performed = false;

    //while (!action_performed) {
    //}

    if (!potential_planets.empty()) {
      ship_state = Attempt_dock;
    }

    //if (map_info.docked_ships[local_player_id_]) {}

    // Start considering attacking when we have more than 5 ships.
    bool consider_attack = false;

    if (num_players() == 2 ) {
      //if (current_round_ <= 13) {
      if (1) {
        // Just go gung ho in the first few rounds.
        consider_attack = true;
        // If we dock our first ship shouldn't spawn for around this many turns
        if (num_players() == 2) {
          for_each_opponent([&consider_attack, &player_info](game::EntityId id) {
            if (player_info[id].total_ships() >= 5) {
              consider_attack = false;
            }
          });
        } else {
          if (player_info[nearest_attacking_opponent].total_ships() > 5) {
            consider_attack = false;
          }
        }
      } else {
        for (const auto player : map_info.players) {
          if (player == local_player_id_)
            continue;

          // Wait until we have at least 5 docked ships
          if (map_info.player_total_docked_ships(local_player_id_) < 5) {
            continue;
          }
          // If we have at least 10% more ships than oppoonent, favour attacking
          consider_attack = map_info.player_docked_ratio(local_player_id_, player) > 1.1;
        }
      }

    } else if (num_players() == 3 || num_players() == 4) {
      // Consider an attack on the 10th ship.
      consider_attack = (player_info[local_player_id_].dock_ratio() > .89);
    }

    // TODO: Add lambda to allow filtering on first run.
    auto potential_targets = find_enemy_ships(enemy_ships_, local_player_id_);
    filter(
      potential_targets,
      [&ship, nearest_attacking_opponent, consider_attack](const Ship& a) {
      // If it would take more than N goes to get there don't bother.
      return a.is_undocked()
        || ship.distance_to_edge_min_turns(a) > (consider_attack ? 13 : 5)
        || (nearest_attacking_opponent != INVALID_ENTITIY_ID && a.owner() != nearest_attacking_opponent);
    }
    );

    if ((num_players() == 2) && !potential_targets.empty()) {
      auto nearest = std::min_element(
        potential_targets.cbegin(),
        potential_targets.cend(),
        [&ship](const Ship& a, const Ship& b) {
        return ship.distance_to_edge(a) < ship.distance_to_edge(b);
      });

      if (ship.distance_to_edge_min_turns(*nearest) < 5) {
        consider_attack = true;
      }
    } else if (potential_targets.empty()) {
      potential_targets = find_enemy_ships(enemy_ships_, local_player_id_);

      filter(
        potential_targets,
        [this, &ship, nearest_attacking_opponent, consider_attack](const Ship& a) {
        if (heading_to_attack_[ship.id()].size() > 2) {
          return true;
        }

        // If it would take more than N goes to get there don't bother.
        return ship.distance_to_edge_min_turns(a) > (consider_attack ? 13 : 5) ||
          (nearest_attacking_opponent != INVALID_ENTITIY_ID && a.owner() != nearest_attacking_opponent);
      }
      );
    }


    if (
      (consider_attack && !potential_targets.empty())
      || potential_planets.empty()
      )
    {
      raf::Log("processing attack");
      //auto potential_targets = find_enemy_ships(enemy_ships_, local_player_id_);
      //filter(
      //  potential_targets,
      //  [&ship](const Ship& a) {
      //  // If it would take more than N goes to get there don't bother.
      //  return a.is_undocked() || ship.distance_min_turns(a) > 10;
      //}
      //);

      // In the case we have no planets and no targets within 13!
      // Head towards this enemy, this should have the unintended consequence that if a planet pops up
      // along the way, that it will get colonised.
      if (potential_targets.empty()) {
        potential_targets = find_enemy_ships(enemy_ships_, local_player_id_);
      }

      std::sort(
        std::begin(potential_targets),
        std::end(potential_targets),
        [&ship](const Ship& a, const Ship& b) {
        return ship.distance_to(a) < ship.distance_to(b);
      });

      for (const auto& target : potential_targets) {
        if (!target.is_undocked() && ship.can_attack(target)) {
          // Already attacking, stay still.
          break;
        }

//#define EXPERIMENTAL
#if defined(EXPERIMENTAL)
        // func, find attack friend.
        double distance_to_nearest_friend = 999999;
        double friend_distance_to_target = 999999;
        EntityId best_friend = -1;
        for (const auto &partner : player_ships_) {
          const auto& partner_ship = partner.second;
          // Can't partner with self.
          if (partner_ship.id() == ship.id()) {
            continue;
          }

          if (!partner_ship.is_undocked()) {
            continue;
          }

          // Ship has already moved
          if (has_already_moved(partner_ship)) {
            continue;
          }

          if (partner_ship.distance_to(ship) > 28.0) {
            continue;
          }

          auto distance = ship.distance_to(partner_ship);
          if (distance < distance_to_nearest_friend) {
            distance_to_nearest_friend = distance;
            best_friend = partner_ship.id();
            friend_distance_to_target = partner_ship.distance_to(target);
          }
        }

        if (best_friend != -1) {
          double our_distance_to_target = ship.distance_to(target);
          auto distance_difference = std::abs(friend_distance_to_target - our_distance_to_target);

          double our_speed = constants::MAX_SPEED, friend_speed = constants::MAX_SPEED;
          // friend is already nearer!
          if (friend_distance_to_target < our_distance_to_target) {
            // eg friend = 15 away, us = 30 away
            // friend speed ratio must be 50% to arrive together
            friend_speed *= friend_distance_to_target / our_distance_to_target;

            // If the distance is large, slow the friend right down.
            if (distance_to_nearest_friend > 14) {
              friend_speed = 1;
            }
          } else {
            our_speed *= our_distance_to_target / friend_distance_to_target;

            // If the distance is large, slow us right down and wait
            if (distance_to_nearest_friend > 14) {
              our_speed = 1;
            }
          }

          const auto velocity =
            navigation::navigate_ship_to_dock(
              planets_vector,
              all_ships,
              ship,
              target,
              our_speed,
              pending_paths_);

          if (velocity.second) {
            move(ship, velocity.first);
            raf::Log("Moving ship ", ship.id(), " towards target", target.id());
            break;
          } else {
            raf::Log("No attack move for Ship!");
          }

          const auto friend_velocity =
            navigation::navigate_ship_to_dock(
              planets_vector,
              all_ships,
              my_ship(best_friend),
              target,
              friend_speed,
              pending_paths_);

          if (friend_velocity.second) {
            move(ship, friend_velocity.first);
            raf::Log("Moving ship ", ship.id(), " towards target", target.id());
            break;
          } else {
            raf::Log("No attack move for Ship!");
          }

        } else {
#endif
          // standard move
          auto navigation_func = target.is_docked() ? navigation::navigate_ship_to_attack : navigation::navigate_ship_to_dock;
          const auto velocity =
            navigation_func(
              planets_vector,
              all_ships,
              ship,
              target,
              constants::MAX_SPEED,
              pending_paths_);

          if (velocity.second) {
            move(ship, velocity.first);
            attack(ship, target);
            raf::Log("Moving ship ", ship.id(), " towards target ", target.id(), " with thrust ", velocity.first.thrust());
            break;
          } else {
            raf::Log("No attack move for Ship!");
          }
        }
#if defined(EXPERIMENTAL)
      }
#endif
    } else {
      raf::Log("processing dock");
      // If there are potential dock planets try and dock.
      for (const auto& planet : potential_planets) {
        if (can_dock(planet, ship)) {
          bool dont_dock_if_under_threat = true;

          if (is_owned_by_opponent(planet, local_player_id_)) {
            const auto &target = get_ship(*planet.docked_ships_.cbegin());
            const auto velocity =
              navigation::navigate_ship_to_attack(
                planets_vector,
                all_ships,
                ship,
                target,
                constants::MAX_SPEED,
                pending_paths_);

            if (velocity.second) {
              move(ship, velocity.first);
              attack(ship, target);
              raf::Log("Moving ship ", ship.id(), " towards target ", target.id(), " with thrust ", velocity.first.thrust());
              break;
            } else {
              raf::Log("No attack move for Ship!");
            }
          } else if (dont_dock_if_under_threat) {
            auto threats_to_dock = ships_within_range(ship, enemy_ships_, local_player_id_, 3);
            if (threats_to_dock.empty()) {
              dock(ship, planet);
            } else {
              // find all nearby friends
              // find all nearby enemies
              std::sort(std::begin(threats_to_dock), std::end(threats_to_dock), [](const Ship& a, const Ship& b) {
                return a.is_undocked() < b.is_undocked();
              });
              const auto &target = threats_to_dock.at(0);
              const auto velocity =
                navigation::navigate_ship_to_attack(
                  planets_vector,
                  all_ships,
                  ship,
                  target,
                  constants::MAX_SPEED,
                  pending_paths_);

              if (velocity.second) {
                move(ship, velocity.first);
                attack(ship, target);
                raf::Log("Moving ship ", ship.id(), " towards target ", target.id(), " with thrust ", velocity.first.thrust());
                break;
              } else {
                raf::Log("No attack move for Ship!");
              }


            }
          } else {
            dock(ship, planet);
          }
          break;
        }
        auto velocity = raf::navigation::navigate_ship_to_dock(
          planets_vector,
          all_ships,
          ship,
          planet,
          constants::MAX_SPEED,
          pending_paths_);

        if (velocity.second) {
          move(ship, velocity.first);
          seek(ship, planet);
          heading_to_planet_[planet.id()].insert(ship.id());
          raf::Log("Moving ship ", ship.id(), " towards planet", planet.id());
          break;
        }
      }
    }

    //std::vector<game::Ship> enemy_ships = map_to_vector(enemy_ships_);
    // dock on planet
    if (0) {
      //game::Planet p;
      //game::Entity e;
      //heading_to_planet_[p.id()].push_back(e.id());
    }
  }
}

std::vector<hlt::Move> MapState::post_frame()
{
  return queued_moves_;
}

void MapState::move(const Ship& ship, const math::Velocity& velocity) {
  pending_paths_.push_back(Path(ship, velocity));
  queued_moves_.push_back(from_velocity(ship, velocity));
  moved_ships_.insert(ship.id());

  // Update ship final location?
}

void MapState::dock(const Ship& ship, const Planet& planet)
{
  heading_to_planet_[planet.id()].insert(ship.id());
  queued_moves_.push_back(hlt::Move::dock(ship.id(), planet.id()));
  moved_ships_.insert(ship.id());

  decisions_.emplace_back(
    Decision::dock(ship.id(), planet.id())
  );
}

void MapState::attack(const Ship& attacker, const Ship& target)
{
  heading_to_attack_[target.id()].insert(attacker.id());
  decisions_.emplace_back(
    Decision::attack(attacker.id(), target.id())
  );
}

void MapState::defend(const Ship& defender, const Ship& defendee, const Ship& target)
{
  decisions_.emplace_back(
    Decision::defend(defender.id(), defendee.id(), target.id())
  );
}

void MapState::seek(const Ship& ship, const Planet& planet)
{
  decisions_.emplace_back(
    Decision::dock(ship.id(), planet.id())
  );
}


void MapState::process_defence(const std::vector<Decision>& all)
{
  auto defenders = select_T(all, [](const Decision& decision) {
    return decision.action == Decision::Action::Defend;
  });

  // split down by defendee
  std::sort(std::begin(defenders), std::end(defenders), [](const Decision& a, const Decision& b) {
    return a.defendee < b.defendee;
  });

  for (
    auto it = std::begin(defenders), first = it;
    // Note <= as std::end is required in the logic below.
    it <= std::end(defenders);
    it++) {

    if (it->defendee != first->defendee) {
      // 'it' could be std::end()
      //defend_it(first, it);
      math::Vec2d defender_location;
      int count = 0;
      for (auto a = first; a < it; a++) {
        const auto& target = player_ships_.at(a->target_id);
        const auto& defendee = player_ships_.at(a->defendee);
        const auto& defender = player_ships_.at(a->unit_id);

        defender_location += defender.current_location();
        count++;
        // Find the mid point between the ship being defended and the aggressor
        auto mid_point = (defendee.current_location() + target.current_location()) / 2;
        auto velocity = navigation::navigate_ship_towards_target(
          get_all_planets(),
          get_all_ships(),
          defender.current_location(),
          mid_point,
          constants::MAX_SPEED,
          true,
          constants::MAX_NAVIGATION_CORRECTIONS,
          2 * M_PI / 180.0,
          pending_paths_);

        if (velocity.second) {
          move(defender, velocity.first);
        }
      }
      defender_location /= count;

      first = it;
    }
  }
  // move to normal intercepting defendee and attacker.

}

void MapState::process_attack(const std::vector<Decision>& all)
{
  auto attackers = select_T(all, [](const Decision& decision) {
    return decision.action == Decision::Action::Attack;
  });

  // split down by attackee
  std::sort(std::begin(attackers), std::end(attackers), [](const Decision& a, const Decision& b) {
    return a.target_id < b.target_id;
  });

  for (
    auto it = std::begin(attackers), first = it;
    it < std::end(attackers);
    it++) {
    const auto& target = enemy_ships_.at(it->target_id);
    const auto& ship = player_ships_.at(it->unit_id);
    auto navigation_func = target.is_docked() ? navigation::navigate_ship_to_attack : navigation::navigate_ship_to_dock;
    const auto velocity =
      navigation_func(
        get_all_planets(),
        get_all_ships(),
        ship,
        target,
        constants::MAX_SPEED,
        pending_paths_);
  }
  // Iterate attack targets
  // Look for nearest friendly ship within 15 radius
  // Move to point which is average of A B and Target?
  // OR move to midpoint of nearest and Target?
  // Make two friendly ships move within 1 distance of each other..

  // E.G if A and B are more than 14 apart then they should head directly towards each other
  // FRIENDLY = {A, B}
  // ENEMY = {T}
  // EXPECTEDDEST = {D}
  // | | | | | | | | | |T| | | | | | | | | |
  // | | | | | | | | | | | | | | | | | | | |
  // | | | | | | | | |a|b| | | | | | | | | |
  // | | | | | | | | |^|^| | | | | | | | | |
  // | | | | | | | | |^|^| | | | | | | | | |
  // | | | | | | | | |^|^| | | | | | | | | |
  // | | | | | | | | |^| |^| | | | | | | | |
  // | | | | | | | | |^| |^| | | | | | | | |
  // |A|-|-|-|-|-|-|a|^| | |b|-|-|-|-|-|-|B|
  //
  // ALGO
  // From origin create circle of radius of max movement
  // IF radii intersect
  // THEN move to closest intersection point to target
  // ELSE simply move towards each other in direction of nearest point.
  // If DIST A, B < 2 then just head towards target without further convergence.
  //
  // V2. Instead of moving towards each other, add en-route convergence based on target distance
  // If Target is 4 turns away, then converge after 2
  //
  // https://gist.github.com/jupdike/bfe5eb23d1c395d8a0a1a4ddd94882ac

}

void MapState::process_dock(const std::vector<Decision>& all)
{
  auto dockers = select_T(all, [](const Decision& decision) -> bool {
    return decision.action == Decision::Action::Dock;
  });

  for (const auto &e : dockers) {
    const auto& ship = player_ships_.at(e.unit_id);
    const auto& planet = planets_.at(e.target_id);
    decisions_.emplace_back(
      Decision::dock(ship.id(), ship.id())
    );
  }

}

void process_decisions(const std::vector<Decision>& all)
{
  // Sort decisions by action.
  auto all_sorted = all;
  std::sort(std::begin(all_sorted), std::end(all_sorted), [](const Decision& a, const Decision& b) {
    return a.action < b.action;
  });

}

} // namespace game
} // namespace raf
