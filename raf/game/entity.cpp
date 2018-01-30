#include "entity.hpp"
#include "constants.hpp"
#include "../math/math.hpp"

#include "../../hlt/entity.hpp"

namespace raf {
namespace game {

Entity::Entity(const hlt::Entity& entity) :
  Entity(
    entity.entity_id,
    entity.owner_id,
    raf::math::Vec2d(entity.location.pos_x, entity.location.pos_y),
    entity.radius,
    entity.health) {
}

void Entity::update(const hlt::Entity& entity) {
  // The following should all be fixed for the lifetime of the Entity.
  //assert(entity.entity_id == id_);
  //assert(entity.owner_id == owner_id);
  //assert(entity.radius == radius);
  update_health(entity.health);
  update_location(raf::math::Vec2d(entity.location.pos_x, entity.location.pos_y));
  owner_ = entity.owner_id;
}

// Probably makes more sense as a matrix at game level?
EntityId Entity::nearest_entity() const {
  // Find the entity with the shortest distance
  const auto min = std::min_element(
    vec_to_.cbegin(),
    vec_to_.cend(),
    [](const decltype(vec_to_)::value_type &lhs, const decltype(vec_to_)::value_type &rhs) -> bool { return lhs.second.length() < rhs.second.length(); }
  );

  if (min != vec_to_.cend())
    return min->first;

  // This shouldn't really happen, but return ourselves if no other entities were found.
  // Candidate for exception throw.
  return id();
}


// Simple collision filter test.
// Given locations, distance travelled and velocity, see if they could possibly ever hit based on the length alone.
bool possible_collision(
  const Entity& a,
  const Entity& b,
  const math::Vec2d& vel_a,
  const math::Vec2d& vel_b) {
  const auto location_delta = a.current_location_ - b.current_location_;
  const auto distance_between = location_delta.length();
  const auto distance_travelled = vel_a.length() + vel_b.length();

  return distance_between < distance_travelled + a.radius_ + b.radius_;
}

bool collision_in_motion(
  const Entity& a,
  const Entity& b,
  const math::Vec2d& vel_a,
  const math::Vec2d& vel_b) {
  auto dist = math::min_dist_squared(a.current_location_, vel_a, b.current_location_ , vel_b);
  return std::sqrt(dist) < a.radius_ + b.radius_;
}

math::Vec2d nearest_attack_point(const Entity & target, const Entity & subject) {
  const double radius = target.radius() + constants::WEAPON_RADIUS;

  // Make the target look at the subject so we can plot the nearest point from center of
  // target on a vector aiming at the subject.
  const auto target_pos = target.current_location();
  const auto subject_pos = subject.current_location();

  const double angle_rad = target_pos.orient_towards_in_rad(subject_pos);

  const double x = target_pos.x() + radius * std::cos(angle_rad);
  const double y = target_pos.y() + radius * std::sin(angle_rad);

  return { x, y };
}

math::Vec2d nearest_dock_point(const Entity & target, const Entity & subject) {
  const double radius = target.radius() + constants::MIN_DISTANCE_FOR_CLOSEST_POINT;

  // Make the target look at the subject so we can plot the nearest point from center of
  // target on a vector aiming at the subject.
  const auto target_pos = target.current_location();
  const auto subject_pos = subject.current_location();

  const double angle_rad = target_pos.orient_towards_in_rad(subject_pos);

  const double x = target_pos.x() + radius * std::cos(angle_rad);
  const double y = target_pos.y() + radius * std::sin(angle_rad);

  return { x, y };
}

} // namespace game
} // namespace raf
