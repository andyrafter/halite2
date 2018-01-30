#include "ship.hpp"

#include "../../hlt/ship.hpp"

namespace raf {
namespace game {

static DockingStatus FromHltShipDockingStatus(hlt::ShipDockingStatus status) {
  switch (status) {
  case hlt::ShipDockingStatus::Undocked: return DockingStatus::Undocked;
  case hlt::ShipDockingStatus::Docking: return DockingStatus::Docking;
  case hlt::ShipDockingStatus::Docked: return DockingStatus::Docked;
  case hlt::ShipDockingStatus::Undocking: return DockingStatus::Undocking;
  }
}


Ship::Ship(const hlt::Ship& ship) :
  Entity(ship),
  docking_progress(ship.docking_progress),
  docking_status_(FromHltShipDockingStatus(ship.docking_status)) {
}

void Ship::update(const hlt::Ship& ship) {

  // Call super impl.
  Entity::update(ship);
  docking_progress = ship.docking_progress;

  docking_status_ = FromHltShipDockingStatus(ship.docking_status);
}


} // namespace game
} // namespace raf
