#include "planet.hpp"

#include <set>

#include "../../hlt/planet.hpp"

namespace raf {
namespace game {

Planet::Planet(const hlt::Planet & planet) :
  Entity(planet),
  num_docking_spots_(planet.docking_spots),
  docked_ships_(std::set<EntityId>(
    std::begin(planet.docked_ships),
    std::end(planet.docked_ships))) {
}

void Planet::update(const hlt::Planet & planet) {
  // Call super impl.
  Entity::update(planet);

  num_docking_spots_ = planet.docking_spots;
  docked_ships_ = std::set<EntityId>(
    std::begin(planet.docked_ships),
    std::end(planet.docked_ships));
}

} // namespace game
} // namespace raf
