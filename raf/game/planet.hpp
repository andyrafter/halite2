#ifndef RAF_PLANET_H_
#define RAF_PLANET_H_

#include "entity.hpp"
#include "ship.hpp"
#include "hlt_fwd.hpp"

#include <iomanip>
#include <set>

namespace raf {
struct PlanetSnapshot;
namespace game {

class Planet : public Entity {
public:
  Planet(EntityId id, EntityId owner_id, const math::Vec2d& initial_location, double radius, int health, int num_docking_spots)
    : Entity(id, owner_id, initial_location, radius, health),
    num_docking_spots_(num_docking_spots) {
  }
  Planet(const hlt::Planet& planet);
  void update(const hlt::Planet& planet);

  bool is_full() const { return docked_ships_.size() == num_docking_spots_; }
  bool is_empty() const { return docked_ships_.empty(); }

  int total_docking_spots() const { return num_docking_spots_; }
  size_t used_docking_spots() const { return docked_ships_.size(); }
  size_t free_docking_spots() const { return num_docking_spots_ - docked_ships_.size(); }

  bool is_docked(EntityId id) const {
    return docked_ships_.find(id) != std::end(docked_ships_);
  }

  void dock_ship(EntityId id) {
    docked_ships_.insert(id);
  }

private:
  int num_docking_spots_;
public:
  std::set<EntityId> docked_ships_;
  // Vector for compatbility for now.
  //std::vector<EntityId> docked_ships_;
};

inline math::Vec2d spawn_point(const game::Planet &planet, int width, int height) {
  const double radius = planet.radius() + constants::MIN_DISTANCE_FOR_CLOSEST_POINT;

  // Make the target look at the subject so we can plot the nearest point from center of
  // target on a vector aiming at the subject.
  const auto target_pos = planet.current_location();
  const math::Vec2d center{ width / 2.0, height / 2.0 };
  const double angle_rad = target_pos.orient_towards_in_rad(center);

  const double x = target_pos.x() + radius * std::cos(angle_rad);
  const double y = target_pos.y() + radius * std::sin(angle_rad);

  return { x, y };
}


inline bool is_dockable(const game::Planet &a, EntityId player_id) {
  if (a.is_owned() && a.owner() != player_id) {
    return false;
  }

  // Planet may or may not be owned at this point, but if its full then give up
  if (a.is_full()) {
    return false;
  }

  return true;
}

inline bool is_owned_by_opponent(const game::Planet &a, EntityId player_id) {
  if (a.is_owned() && a.owner() != player_id) {
    return true;
  }
  return false;
}

inline bool can_dock(const game::Planet& planet, const game::Ship& ship) {
  return ship.distance_to(planet) <= (constants::SHIP_RADIUS + constants::DOCK_RADIUS + planet.radius());
}

// Number of attacks at specified damage required to destroy a planet
inline int attacks_to_destroy(const game::Planet& planet, int damage) {
  // 254 hp / 255 dmg = 0
  // 255 hp / 255 dmg = 1
  // 256 hp / 255 dmg = 1
  // Thus subtract 1 first in before adding 1 to give turns.
  return ((planet.health()-1) / damage) + 1;
}

inline std::ostream& operator<<(std::ostream&os, const Planet& planet) {
  os << std::setprecision(6)
    << "Planet: id=" << planet.id()
    << ", loc=" << planet.current_location()
    << ", owner=" << planet.owner()
    << ", radius=" << planet.radius()
    << "\n";
  return os;
}


}
}
#endif // !RAF_PLANET_H_
