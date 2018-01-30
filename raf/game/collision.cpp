#include "collision.hpp"
#include <algorithm>

namespace raf {
namespace collision {

bool segment_circle_intersect(const Vec2d & start, const Vec2d & end, const Vec2d & object, const double fudge, const double circle_radius)
{
  // https://stackoverflow.com/questions/1073336/circle-line-segment-collision-detection-algorithm
  // Parameterize the segment as start + t * (end - start),
  // and substitute into the equation of a circle
  // Solve for t
  //const double circle_radius = circle.radius;
  //const double dx = end_x - start_x;
  //const double dy = end_y - start_y;

  //const double a = square(dx) + square(dy);

  const Vec2d dxdy = end - start;
  const Vec2d fxfy = start - object;
  const double a = dxdy.length_squared();

  if (std::fabs(a) < 0.000001) {
    // Start and end are the same point

    // If start vs. object are closer that radius already then not sure what we can do
    // Generally this is not a valid game state without death occurings.
    return fxfy.length() <= circle_radius + fudge;
  }

  const double b =
    -2 * (square(start.x()) - (start.x() * end.x())
      - (start.x() * object.x()) + (end.x() * object.x())
      + square(start.y()) - (start.y() * end.y())
      - (start.y() * object.y()) + (end.y() * object.y()));

  // Time along segment when closest to the circle (vertex of the quadratic)
  const double t = std::min(-b / (2 * a), 1.0);
  if (t < 0) {
    return false;
  }

  const double closest_x = start.x() + dxdy.x() * t;
  const double closest_y = start.y() + dxdy.y() * t;
  const double closest_distance = (Vec2d{ closest_x, closest_y } -object).length();

  return closest_distance <= circle_radius + fudge;
}

} // namespace collision
} // namespace raf
