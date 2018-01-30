#include "path.hpp"

#include "gtest/gtest.h"

using raf::game::Path;
using raf::math::angle_rad_to_deg_clipped;
using raf::math::min_dist_squared;
using raf::math::Vec2d;
using raf::math::Velocity;

TEST(raf_path, collision_basic)
{
  const Vec2d a_start(0, 0);
  const Vec2d a_vel(1, 1);

  const Vec2d b_start(2, 0);
  const Vec2d b_vel(-1, 1);

  ASSERT_EQ(2, a_vel.length_squared());
  ASSERT_DOUBLE_EQ(0, min_dist_squared(a_start, a_vel, b_start, b_vel));
}


TEST(raf_path, collision_varying_velocity)
{
  const Vec2d a_start(0, 0);
  const Vec2d a_end(1, 1);
  const Path ent_a{ 0, a_start, a_end, 0.5 };

  const Vec2d b_start(10, 0);
  const Vec2d b_end(1, 1);
  const Path ent_b{ 0, b_start, b_end, 0.5 };

  const Velocity a_vel = a_end - a_start;
  const Velocity b_vel = b_end - b_start;
  
  ASSERT_EQ(2, a_vel.thrust_squared());
  ASSERT_EQ(45, angle_rad_to_deg_clipped(a_vel.angle_rad()));
  std::cout << min_dist_squared(ent_a.start_pos, a_vel, ent_b.start_pos, b_vel);
  ASSERT_DOUBLE_EQ(0, min_dist_squared(a_start, a_vel, b_start, b_vel));

}
