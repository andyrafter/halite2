#ifndef RAF_GAME_COLLISION_H_
#define RAF_GAME_COLLISION_H_

#include <algorithm>

#include "../math/math.hpp"

namespace raf {
namespace collision {
  using math::Vec2d;
  static double square(const double num) {
    return num * num;
  }

  /**
    * Test whether a given line segment intersects a circular area.
    *
    * @param start  The start of the segment.
    * @param end    The end of the segment.
    * @param circle The circle to test against.
    * @param fudge  An additional safety zone to leave when looking for collisions. Probably set it to ship radius.
    * @return true if the segment intersects, false otherwise
    */
  bool segment_circle_intersect(
    const Vec2d& start,
    const Vec2d& end,
    const Vec2d& object,
    const double fudge,
    const double circle_radius);

}
}

#endif // RAF_GAME_COLLISION_H_
