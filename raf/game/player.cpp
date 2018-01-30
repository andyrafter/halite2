#include "player.hpp"

#include "../../hlt/planet.hpp"
#include "../../hlt/ship.hpp"
namespace raf {
namespace game {

void Player::update(const hlt::Planet & planet) {
  auto result = planets_.insert(std::make_pair(planet.entity_id, planet));

  if (!result.second) {
    result.first->second.update(planet);
  }
}
void Player::update(const hlt::Ship & ship) {
  auto result = ships_.insert(std::make_pair(ship.entity_id, ship));

  if (!result.second) {
    result.first->second.update(ship);
  }
}

} // namespace game
} // namespace raf
