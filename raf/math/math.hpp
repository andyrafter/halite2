#ifndef RAF_MATH_H_
#define RAF_MATH_H_

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include "../game/constants.hpp"

namespace raf {
namespace math {

static int angle_rad_to_deg_clipped(const double angle_rad) {
  const long deg_unclipped = lround(angle_rad * 180.0 / M_PI);
  // Make sure return value is in [0, 360) as required by game engine.
  return static_cast<int>(((deg_unclipped % 360L) + 360L) % 360L);
}

// Improve
// see : https://codereview.stackexchange.com/questions/26608/review-of-2d-vector-class
template<typename T>
class Vec2 {
public:
  Vec2() : Vec2(0, 0) {}
  Vec2(T x, T y) : x_(x), y_(y) {}

  T x() const { return x_; }
  T y() const { return y_; }

  const Vec2<T>& operator+=(const Vec2<T>& rhs) {
    x_ += rhs.x_;
    y_ += rhs.y_;
    return *this;
  }

  const Vec2<T>& operator-=(const Vec2<T>& rhs) {
    x_ -= rhs.x_;
    y_ -= rhs.y_;
    return *this;
  }

  //const Vec2<T>& operator*=(const Vec2<T>& rhs) {
  //  x_ *= rhs.x_;
  //  y_ *= rhs.y_;
  //  return *this;
  //}
  //
  //const Vec2<T>& operator/=(const Vec2<T>& rhs) {
  //  x_ /= rhs.x_;
  //  y_ /= rhs.y_;
  //  return *this;
  //}

  const Vec2<T>& operator/=(double rhs) {
    x_ /= rhs;
    y_ /= rhs;
    return *this;
  }


  friend Vec2<T> operator+(const Vec2<T>& lhs, const Vec2<T>& rhs) {
    return { lhs.x_ + rhs.x_, lhs.y_ + rhs.y_ };
  }

  friend Vec2<T> operator-(const Vec2<T>& lhs, const Vec2<T>& rhs) {
    return { lhs.x_ - rhs.x_, lhs.y_ - rhs.y_ };
  }

  // Multiplication of a Vector by a vector is a dot product
  friend double operator*(const Vec2<T>& lhs, const Vec2<T>& rhs) {
    return lhs.x_ * rhs.x_ + lhs.y_ * rhs.y_;
  }

  // Magnitude extension.
  friend Vec2<T> operator*(const double value, const Vec2<T> rhs) {
    return { value * rhs.x_, value * rhs.y_ };
  }

  friend Vec2<T> operator*(const Vec2<T> rhs, const double value) {
    return { value * rhs.x_, value * rhs.y_ };
  }

  //friend Vec2<T> operator/(const Vec2<T>& lhs, const Vec2<T>& rhs) {
  //  return { lhs.x_ / rhs.x_, lhs.y_ / rhs.y_ };
  //}

  friend Vec2<T> operator/(const Vec2<T>& lhs, double rhs) {
    return { lhs.x_ / rhs, lhs.y_ / rhs };
  }

  Vec2<T> &operator=(const Vec2<T>& rhs) {
    x_ = rhs.x_;
    y_ = rhs.y_;
    return *this;
  }

  friend bool operator==(const Vec2<T>& lhs, const Vec2<T>& rhs) {
    return lhs.x_ == rhs.x_ && lhs.y_ == rhs.y_;
  }



  friend double dot_product(const Vec2<T>& a, const Vec2<T>& b) { return a * b; }
  friend Vec2<T> direction(const Vec2<T>& a, const Vec2<T>& b) { return a - b; }

  double length() const { return sqrt(x_*x_ + y_*y_); }
  double length_squared() const { return x_*x_ + y_*y_; }

  double orient_towards_in_rad(const Vec2<T>& target) const {
    const Vec2<T> dxdy = target - *this;
    return std::atan2(dxdy.y(), dxdy.x()) + 2 * M_PI;
  }


  static constexpr Vec2 Zero() { return { 0, 0 }; }
  static constexpr Vec2 Min() {
    return { std::numeric_limits<T>::lowest(), std::numeric_limits<T>::lowest() };
  }
  static constexpr Vec2 Max() {
    return { std::numeric_limits<T>::max(), std::numeric_limits<T>::max() };
  }
private:
  T x_;
  T y_;

};

template<typename T>
std::ostream &operator<<(std::ostream& os, const Vec2<T>& vec) {
  os << "(" << vec.x() << ", " << vec.y() << ")";
  return os;
}

using Vec2d = Vec2<double>;
using Vec2f = Vec2<float>;
using Vec2i = Vec2<int>;

//template<typename T>
//double orient_towards_in_rad(const Vec2<T>& target, const Vec2<T>& subject) {
//  const Vec2<T> dxdy = target - subject;
//  return std::atan2(dxdy.y(), dxdy.x()) + 2 * M_PI;
//}

template<typename T>
Vec2<T> get_closest_point(const Vec2<T>& target, const Vec2<T>& subject, const double target_radius) {
  const double radius = target_radius + constants::MIN_DISTANCE_FOR_CLOSEST_POINT;

  // Make the target look at the subject so we can plot the nearest point from center of
  // target on a vector aiming at the subject.
  const double angle_rad = target.orient_towards_in_rad(subject);

  const double x = target.x() + radius * std::cos(angle_rad);
  const double y = target.y() + radius * std::sin(angle_rad);

  return { x, y };
}


class Velocity {
public:
  Velocity(int thrust, int angle_in_degrees) :
    velocity_(thrust_angle_deg_to_vector(thrust, angle_in_degrees)) {
  }

  Velocity(double x, double y)
    : velocity_(x, y) {
  }

  Velocity(const Vec2d& vec)
    : velocity_(vec) {
  }

  //raf::math::Vec2d accelerate_by(const raf::math::Vec2d& position) const
  //{
  //  raf::math::Vec2d new_position = position;
  //  new_position += to_vec();
  //  return new_position;
  //}

  raf::math::Vec2d to_vec() const {
    return velocity_;
  }

  // This should really be some epsilon type adjustment in order to prevent
  // Situations where thrust is x.9999 and rounded down.
  int thrust() const { return static_cast<int>(std::sqrt(thrust_squared()) + 0.5); }
  int angle_in_degrees() const { return angle_rad_to_deg_clipped(angle_rad()); }

  double angle_rad() const {
    return std::atan2(velocity_.y(), velocity_.x()) + 2 * M_PI;
  }

  double thrust_squared() const {
    return velocity_.length_squared();
  }

private:

  static raf::math::Vec2d thrust_angle_deg_to_vector(int thrust, int angle_in_degrees) {
    auto angle_rads = angle_in_degrees * M_PI / 180.0;
    return raf::math::Vec2d{ thrust * std::cos(angle_rads), thrust * std::sin(angle_rads) };
  }

  Vec2d velocity_;
};

template<typename T>
Vec2<T> operator+(const Vec2<T>& pos, const Velocity& velocity) {
  return pos + velocity.to_vec();
}

//template<typename T>
//Velocity operator-(const Vec2<T>& a, const Vec2<T>& b) {
//  return a - b;
//}


class Circle
{
public:
  using value_type = double;
  Circle(value_type radius, Vec2<value_type> origin)
    : radius_(radius),
    origin_(origin) {
    }

  // Taken from http://paulbourke.net/geometry/circlesphere/tvoght.c
  friend bool intersects(
    const Circle& a,
    const Circle& b,
    std::array< Vec2<value_type>, 2>& intersections) {
    const auto dxdy = b.origin_ - a.origin_;

    // Distance is more than sum of radii
    // Impossible for intersection to occur
    const auto distance = dxdy.length();
    if (distance > a.radius_ + b.radius_) {
      return false;
    }

    // smaller circle is within a bigger circle
    if (distance < fabs(a.radius_ - b.radius_)) {
      return false;
    }
    const auto ar_squared = a.radius_ * a.radius_;
    const auto br_squared = b.radius_ * b.radius_;
    const auto distance_squared = distance * distance;

    const auto p0_to_p2 = (ar_squared - br_squared + distance_squared) / (2.0 * distance);

    const Vec2d offset = dxdy * (p0_to_p2 / distance);
    const Vec2d position = a.origin_ + offset;

    const auto height= sqrt(ar_squared - (p0_to_p2 * p0_to_p2));

    const Vec2d rxy(
      -dxdy.y() * (height / distance),
      dxdy.x() * (height / distance));

    intersections[0] = position + rxy;
    intersections[1] = position - rxy;

    return true;
  }

private:
  value_type radius_;
  Vec2<value_type> origin_;
};



static double min_dist_squared(
  const raf::math::Vec2d& a,
  const raf::math::Vec2d& a_vel,
  const raf::math::Vec2d& b,
  const raf::math::Vec2d& b_vel)
{
  // # p is 1st point
  // # u is distance moved by p in 1 time step
  // # q is 2nd point
  // # v is distance moved by q in 1 time step
  //
  // # interested in points at time t {0..1}:
  // #
  // # P = p + u * t
  // # Q = q + v * t
  // #
  // # vector separating points
  // #
  // # D = P - Q = p-q + (u-v)* t
  // #
  // # Let pq = p - q and uv = u - v
  // #
  // # D = pq + uv * t
  // #
  // # D.x = pq.x + uv.x * t
  // # D.y = pq.y + uv.y * t
  // #
  // # dist_squared = D**2 = (pq.x + uv.x * t)**2 + (pq.y + uv.y * t)**2
  // #                     = pq.x**2 + 2*pq.x*uv.x*t + uv.x**2 * t**2 +
  // #                       pq.y**2 + 2*pq.y*uv.y*t + uv.y**2 * t**2
  // #                     = (uv.x**2 + uv.y**2)       * t**2 +
  // #                       2*(pq.x*uv.x + pq.y*uv.y) * t +
  // #                       (pq.x**2 + pq.y**2)
  // #
  // # let A = uv.x**2 + uv.y**2
  // # let B = 2*(pq.x*uv.x + pq.y*uv.y)
  // # let C = pq.x**2 + pq.y**2
  // #
  // # D**2 = A * t**2 + B * t + C
  // #
  // # derivative: d(D**2)/dt = 2*A * t + B
  // # is 0 at minimum.
  // #
  // # 2*A * t + B == 0
  // # t = -B / (2 * A)
  // #
  // # if t is -ve, then cannot hit if not already.
  // # if t is > 1, then use distance at t == 1.
  const auto diff_position = a - b;
  const auto diff_velocity = a_vel - b_vel;

  // A = uvx * uvx + uvy * uvy
  auto A = diff_velocity.length_squared();
  // B = 2 * (pqx * uvx + pqy * uvy)
  auto B = 2 * dot_product(diff_position, diff_velocity);
  // C = pqx * pqx + pqy * pqy
  auto C = diff_position.length_squared();
  if (A == 0) {
    // # velocity vectors are parallel and have same magnitude.
    // # distance apart is constant, so use distance at t == 0.
    return C;
  }
  // # limit t to be no further away than 1 time step into future.
  auto t = std::min(1.0, -B / (2 * A));

  // # for - ve t return dist at t == 0.
  if (t < 0) {
    return C;
  }

  // # work out distance apart at time t.
  return t * t * A + t * B + C;

  //dist = math.sqrt(min_dist_squared(0, 0, 7, 0, 0, 2, 6, -1))
  //print dist
}

static double min_dist_squared(
  const raf::math::Vec2d & a,
  const Velocity &a_vel,
  const raf::math::Vec2d& b,
  const Velocity &b_vel)
{
  return min_dist_squared(a, a_vel.to_vec(), b, b_vel.to_vec());
}


constexpr double degrees_to_rads(double degrees) {
  return degrees * M_PI / 180.0;
}
}

}

#endif // !RAF_MATH_H_

