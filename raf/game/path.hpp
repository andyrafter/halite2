#ifndef RAF_GAME_PATH_H_
#define RAF_GAME_PATH_H_

#include "entity.hpp"

#include <iosfwd>

namespace raf {
namespace game {

struct Path {
  // Create a path with an explicit coordinate start and end point.
  Path(game::EntityId id, math::Vec2d start, math::Vec2d end, double radius)
    : ship_id(id),
    radius(radius),
    start_pos(start),
    end_pos(end) {
  }

  // Create a path with an explicit coordinate start and end point derived from velocity
  Path(game::EntityId id, math::Vec2d start, const math::Velocity& velocity, double radius)
    : Path(id, start, start + velocity,radius) {
  }

  // Create a path with from an entity using its current location end point derived from velocity
  Path(const game::Entity& entity, const math::Velocity& velocity)
    : Path(entity.id(), entity.current_location(), velocity, entity.radius()) {
  }

  game::EntityId ship_id;
  math::Vec2d start_pos;
  math::Vec2d end_pos;
  double radius;
};

std::ostream& operator<<(std::ostream&os, const Path& path);

}
}

#endif // !RAF_GAME_PATH_H_
