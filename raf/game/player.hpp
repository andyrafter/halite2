#ifndef RAF_PLAYER_H_
#define RAF_PLAYER_H_

#include "planet.hpp"
#include "ship.hpp"

#include <algorithm>
#include <map>

namespace raf {
namespace game {

using PlayerId = int;

class Player {
public:
  Player(PlayerId id)
    : id_(id) {
  }

  // distance travelled
  // Total movement last turn by all ships

  // Production rate

  // Total ships
  int total_ships_produced() const { return ships_.size(); }

  // Current ships
  int current_ships() const { return ships_.size(); }

  // Total health of all ships
  int ship_health() const { return ships_.size(); }

  // Total health of all planets
  int planet_health() const { return ships_.size(); }

  // Current planets

  void update(const hlt::Planet& planet);

  void update(const hlt::Ship& ship);

  game::Planet nearest_planet_to(const game::Entity& entity) const {
    auto element = std::min_element(
      planets_.cbegin(),
      planets_.cend(),
      [&entity](const decltype(planets_)::value_type& a, const decltype(planets_)::value_type& b) {
      return entity.distance_to(a.second) < entity.distance_to(b.second);
    });

    return element->second;
  }

  game::Ship nearest_ship_to(const game::Entity& entity) const {
    auto element = std::min_element(
      ships_.cbegin(),
      ships_.cend(),
      [&entity](const decltype(ships_)::value_type& a, const decltype(ships_)::value_type& b) {
      return entity.distance_to(a.second) < entity.distance_to(b.second);
    });

    return element->second;
  }


private:
  PlayerId id_;

  // list of ships....
  std::map<game::EntityId, game::Ship> ships_;
  // list of planets owned.
  std::map<game::EntityId, game::Planet> planets_;

};
}
}
#endif // !RAF_PLAYER_H_
