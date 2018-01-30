#ifndef RAF_MAP_STATE_H_
#define RAF_MAP_STATE_H_

#include "decision.hpp"
#include "entity.hpp"
#include "path.hpp"
#include "planet.hpp"
#include "player.hpp"
#include "ship.hpp"
#include "../types.hpp"

#include "navigation.hpp"
#include "../util.hpp"

#include <algorithm>
#include <functional>
#include <map>
#include <vector>

namespace raf {
namespace game {

hlt::Move from_velocity(const game::Entity& entity, const math::Velocity& vel);

class RoundStartEntityState {
public:
  std::vector<Ship> all_ships() const { return std::vector<Ship>(); }
  std::vector<Ship> enemy_ships() const { return std::vector<Ship>(); }
  std::vector<Ship> player_ships() const { return std::vector<Ship>(); }

  std::set<game::EntityId> valid_planet_ids() const { return valid_planet_ids_; }
  std::set<game::EntityId> valid_ship_ids() const { return valid_ship_ids_; }
  std::set<game::EntityId> valid_player_ids() const { return valid_player_ids_; }


  void update(const hlt::Planet& planet) {
    const auto& p = planets_.insert({ planet.entity_id, planet });
    if (!p.second) {
      p.first->second.update(planet);
    }

    mark_valid(p.first->second);
  }

  void update(const hlt::Ship& ship) {
    const auto &s = (ship.owner_id == local_player_id_) ?
      player_ships_.insert({ ship.entity_id, ship }) :
      enemy_ships_.insert({ ship.entity_id, ship });

    if (!s.second) {
      s.first->second.update(ship);
    }

    mark_valid(s.first->second);
  }

  void mark_valid(const Planet& planet) {
    valid_planet_ids_.insert(planet.id());
  }

  void mark_valid(const Ship& ship) {
    valid_ship_ids_.insert(ship.id());
    valid_player_ids_.insert(ship.owner());
  }

  bool is_valid(const Planet& planet) const {
    return valid_planet_ids_.find(planet.id()) != std::end(valid_planet_ids_);
  }

  bool is_valid(const Ship& ship) const {
    return valid_ship_ids_.find(ship.id()) != std::end(valid_ship_ids_);
  }

  void prune_dead_entities() {
    map_erase_if(player_ships_, [this](const Ship& ship) { return !is_valid(ship); });
    map_erase_if(enemy_ships_, [this](const Ship& ship) { return !is_valid(ship); });
    map_erase_if(planets_, [this](const Planet& planet) { return !is_valid(planet); });
  }

private:
  int round_number;
  PlayerId local_player_id_;
  std::map<game::EntityId, game::Ship> enemy_ships_;
  std::map<game::EntityId, game::Ship> player_ships_;
  std::map<game::EntityId, game::Planet> planets_;

  std::set<game::EntityId> valid_planet_ids_;
  std::set<game::EntityId> valid_ship_ids_;
  std::set<game::PlayerId> valid_player_ids_;
};

class MapState {
public:
  MapState(raf::math::Vec2i dimensions, game::EntityId local_player_id, int initial_players)
    : current_round_(0),
    local_player_id_(local_player_id),
    dimensions_(dimensions),
    initial_players_(initial_players) {
  }

  void BeginRound(int round_number) {
    current_round_ = round_number;
    valid_planets_.clear();
    valid_ships_.clear();
    valid_players_.clear();
  }

#if 0
  game::Planet nearest_planet_to(const game::Entity& entity) {
    return players_.at(0).nearest_planet_to(entity);
  }
#endif

  // Update
  // Take a new snapshot of data and apply it to existing persistant data
  // If the new entity does not exist then create it, else update it.
  void update(const hlt::Planet& planet);
  void update(const hlt::Ship& ship);
  void mark_valid(const Planet& planet);
  void mark_valid(const Ship& ship);
  bool is_valid(const Planet& planet) const;
  bool is_valid(const Ship& ship) const;
  void pre_frame();
  void run_frame();
  std::vector<hlt::Move> post_frame();

  int num_players() const {
    return valid_players_.size();
  }

private:
  bool can_dock_more(game::EntityId planet_id) const;
  void prune_dead_entities();

  bool has_already_moved(const Ship& ship) const {
    return moved_ships_.count(ship.id());
  }

  const Ship& my_ship(EntityId id) const {
    return player_ships_.at(id);
  }

  const Ship& enemy_ship(EntityId id) const {
    return enemy_ships_.at(id);
  }

  void move(const Ship& ship, const math::Velocity& velocity);
  void dock(const Ship& ship, const Planet& planet);

  void attack(const Ship& attacker, const Ship& target);
  void defend(const Ship& defender, const Ship& defendee, const Ship& target);
  // rename dock when other dock is refactored.
  void seek(const Ship& ship, const Planet& planet);
  void process_ship(const Ship& ship);

  template<typename T>
  void for_each_opponent(T func) {
    for (auto e : valid_players_) {
      if (e == local_player_id_) {
        continue;
      }
      func(e);
    }
  };

  const Ship& get_ship(EntityId id) const {
    // Check player ships first
    bool is_player_ship = player_ships_.find(id) != std::end(player_ships_);
    if (is_player_ship) {
      return my_ship(id);
    }
    // Will throw if not found.
    return enemy_ship(id);
  }

  void process_defence(const std::vector<Decision>& all);
  void process_attack(const std::vector<Decision>& all);
  void process_dock(const std::vector<Decision>& all);

  std::vector<Ship> get_all_ships() const {
    const auto local_ship_vector = map_to_vector(player_ships_);
    const auto enemy_vector = map_to_vector(enemy_ships_);
    std::vector<Ship> result;
    result.reserve(local_ship_vector.size() + enemy_vector.size());
    result.insert(std::end(result), std::begin(local_ship_vector), std::end(local_ship_vector));
    result.insert(std::end(result), std::begin(enemy_vector), std::end(enemy_vector));

    return result;
  }

  std::vector<Planet> get_all_planets() const {
    return map_to_vector(planets_);
  }

  int current_round_;
  int initial_players_;
  raf::math::Vec2i dimensions_;
  game::EntityId local_player_id_;

  // These need combining...
  std::map<game::EntityId, game::Ship> enemy_ships_;
  std::map<game::EntityId, game::Ship> player_ships_;
  std::map<game::EntityId, game::Planet> planets_;

  std::set<game::EntityId> valid_planets_;
  std::set<game::EntityId> valid_ships_;
  std::set<game::PlayerId> valid_players_;

  // Planet Id to List of ships heading there
  std::map<game::EntityId, std::set<game::EntityId>> heading_to_planet_;
  std::map<game::EntityId, std::set<game::EntityId>> heading_to_attack_;

  // Paths pending by game entities.
  // Should be checked against before moving a ship.
  std::vector<Path> pending_paths_;
  // 
  std::vector<hlt::Move> queued_moves_;

  // class Round{}?
  std::set<game::EntityId> moved_ships_;

  // Decision
  std::vector<Decision> decisions_;
};


std::vector<game::Planet> dockable_planets(
  const std::map<game::EntityId, game::Planet>& planets_,
  game::PlayerId player_id);

std::vector<game::Planet> opponent_planets(
  const std::map<game::EntityId, game::Planet>& planets_,
  game::PlayerId player_id);

std::vector<Ship> find_enemy_ships(
  const std::map<EntityId, Ship>& ships,
  hlt::PlayerId local_player_id);

}
}

#endif // !RAF_MAP_STATE_H_
