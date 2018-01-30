#ifndef RAF_ENTITY_SNAPSHOT_H_
#define RAF_ENTITY_SNAPSHOT_H_

#include "entity.hpp"
#include "planet.hpp"

namespace raf {
namespace game {

struct EntitySnapshot {
  EntityId entity_id;
  PlayerId owner_id;
  math::Vec2d position;
  int health;
  double radius;
};

struct PlanetSnapshot : EntitySnapshot {
  bool owned;
  /// The remaining resources.
  int remaining_production;
  /// The currently expended resources.
  int current_production;
  /// The maximum number of ships that may be docked.
  int docking_spots;
  /// Contains IDs of all ships in the process of docking or undocking,
  /// as well as docked ships.
  std::vector<game::EntityId> docked_ships;

  void update_planet(game::Planet& planet) const {
    planet.update_health(health);
    planet.update_location(position);

    for (auto & e : docked_ships) {
      planet.dock_ship(e);
    }

  }

};

struct ShipSnapshot : EntitySnapshot {
  game::DockingStatus docking_status;
  int docking_progress;
  game::EntityId docked_planet;

  void update_ship(game::Ship& ship) const {
    ship.update_health(health);
    ship.update_location(position);
  }
};

}
}

#endif // !RAF_MAP_STATE_H_
