#ifndef RAF_SHIP_H_
#define RAF_SHIP_H_

#include "constants.hpp"
#include "entity.hpp"
#include "hlt_fwd.hpp"
#include <iomanip>

namespace raf {
namespace game {

enum class DockingStatus {
  Undocked = 0,
  Docking = 1,
  Docked = 2,
  Undocking = 3,
};

class Ship : public Entity {
public:
  Ship(EntityId id, EntityId owner_id, const math::Vec2d& initial_location, double radius, int health)
    : Entity(id, owner_id, initial_location, radius, health) {
  }

  Ship(const hlt::Ship& ship);
  void update(const hlt::Ship& ship);

  bool is_undocked() const { return docking_status_ == DockingStatus::Undocked; }
  bool is_docking() const { return docking_status_ == DockingStatus::Docking; }
  bool is_docked() const { return docking_status_ == DockingStatus::Docked; }
  bool is_undocking() const { return docking_status_ == DockingStatus::Undocking; }

  bool can_attack(const Ship& target) const {
    return distance_to(target) < constants::WEAPON_RADIUS;
  }

  // Return the minimum number of turns before a ship could be Undocked.
  int turns_to_undock() const {
    switch (docking_status_) {
    case DockingStatus::Undocked:
      return 0;

    case DockingStatus::Docking:
      // If it is docking, then it has to complete the dock + undock!
      return docking_progress + constants::DOCK_TURNS;

    case DockingStatus::Docked:
      return constants::DOCK_TURNS;

    case DockingStatus::Undocking:
      return docking_progress;
    }
  }

private:
  DockingStatus docking_status_;
  int docking_progress;
};

inline std::ostream& operator<<(std::ostream&os, const Ship& ship) {
  os << std::setprecision(6)
    << "Ship: id=" << ship.id()
    << ", loc=" << ship.current_location()
    << ", owner=" << ship.owner()
    << ", radius=" << ship.radius()
    << "\n";
  return os;
}


enum ShipState {
  Spawned,
  TravellingToPlanet,
  UnderAttack,
  DefendingPlanet,
  AttackingSolo,
  AttackingWithSquad,
};

class LocalShip : public Ship {
public:
  int last_moved() const { return last_moved_; }
  int set_moved(int frame) { last_moved_ = frame; }
private:
  //
  ShipState current_state_;
  // Upgrade to stack if full behaviour memory is required.
  ShipState previous_state_;

  // Used to flag last time this entity was moved.
  int last_moved_;

};

}
}
#endif // !RAF_SHIP_H_
