#ifndef RAF_ENTITY_H_
#define RAF_ENTITY_H_

#include "constants.hpp"
#include "hlt_fwd.hpp"
#include "../math/math.hpp"

#include <algorithm>
#include <map>

namespace raf {
namespace game {
using EntityId = int;
constexpr EntityId INVALID_ENTITIY_ID = -1;

class Entity {
public:
  // Construct an entity with an initial location
  Entity(EntityId id, EntityId owner_id, const math::Vec2d& initial_location, double radius, int health) :
    id_(id),
    owner_(owner_id),
    current_location_(initial_location),
    // Previous initialises to current so that the velocity at start is always 0.
    previous_location_(initial_location),
    velocity_(math::Vec2d::Zero()),
    previous_velocity_(math::Vec2d::Zero()),
    radius_(radius),
    health_(health) {
  }
  Entity(const hlt::Entity& entity);

  void update(const hlt::Entity& entity);
  void update_location(const math::Vec2d& new_location) {
    // Save off current values
    previous_location_ = current_location_;
    previous_velocity_ = velocity_;

    current_location_ = new_location;
    velocity_ = current_location_ - previous_location_;
  }

  void update_health(int new_health) {
    health_ = new_health;
  }

  const math::Vec2d& velocity() const {
    return velocity_;
  }

  EntityId id() const {
    return id_;
  }

  bool is_alive() const {
    return health_ > 0;
  }

  int health() const { return health_; }

  bool is_owned() const {
    return owner_ != INVALID_ENTITIY_ID;
  }

  EntityId owner() const {
    return owner_;
  }

  double radius() const {
    return radius_;
  }

  double distance_to(const math::Vec2d& a) const {
    return (current_location_ - a).length();
  }

  double distance_to(const Entity& a) const {
    return (current_location_ - a.current_location_).length();
  }

  double distance_to_edge(const Entity& a) const {
    return (current_location_ - a.current_location_).length() - a.radius() - radius();
  }

  double distance_to_edge_min_turns(const Entity& a) const {
    return (distance_to_edge(a) / raf::constants::MAX_SPEED) + 1;
  }

  double distance_min_turns(const Entity& a) const {
    return (distance_to(a) / raf::constants::MAX_SPEED) + 1;
  }

  math::Vec2d current_location() const { return current_location_; }

  void update_distance(const Entity& a) {
    // Do we want this to be to origin, or edge... i.e. include radius?
    vec_to_[a.id()] = a.current_location_ - current_location_;
  }

  // Probably makes more sense as a matrix at game level?
  // Returns the EntityId of the nearest entity "as the crow flies"
  EntityId nearest_entity() const;

  friend bool possible_collision(
    const Entity& a,
    const Entity& b,
    const math::Vec2d& vel_a,
    const math::Vec2d& vel_b);

  friend bool collision_in_motion(
    const Entity& a,
    const Entity& b,
    const math::Vec2d& vel_a,
    const math::Vec2d& vel_b);

private:
  EntityId id_;
  EntityId owner_;
  int health_;

  math::Vec2d current_location_;
  math::Vec2d previous_location_;
  math::Vec2d velocity_;
  math::Vec2d previous_velocity_;
  double radius_;

  // vector offset to get to another entity.
  std::map<EntityId, math::Vec2d> vec_to_;
};

math::Vec2d nearest_attack_point(const Entity& target, const Entity& subject);
math::Vec2d nearest_dock_point(const Entity& target, const Entity& subject);


}
}
#endif // !RAF_ENTITY_H_
