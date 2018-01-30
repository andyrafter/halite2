#include "math.hpp"

#include "gtest/gtest.h"

using namespace raf::math;

TEST(raf_math, basic_vec2_a)
{
  Vec2d src{ 0, 0 };
  Vec2d dest{ 0, 0 };

  ASSERT_EQ(0, src.x());
  ASSERT_EQ(0, src.y());
  ASSERT_EQ(0, dest.x());
  ASSERT_EQ(0, dest.y());

  auto diff = dest - src;
  ASSERT_DOUBLE_EQ(0, diff.length());
  ASSERT_DOUBLE_EQ(0, diff.length_squared());

  src += {1, 1};

  ASSERT_DOUBLE_EQ(2, src.length_squared());
}

TEST(raf_math, basic_vec2_addition)
{
  const Vec2d src{ 0, 0 };
  const Vec2d dest{ 2, 2 };

  auto sum = src + dest;
  ASSERT_EQ(2, sum.x());
  ASSERT_EQ(2, sum.y());
  ASSERT_DOUBLE_EQ(8, sum.length_squared());
  ASSERT_DOUBLE_EQ(8, sum.length_squared());

  auto sum2 = src + dest + dest + dest;
  ASSERT_EQ(6, sum2.x());
  ASSERT_EQ(6, sum2.y());
  ASSERT_DOUBLE_EQ(72, sum2.length_squared());
  ASSERT_DOUBLE_EQ(72, sum2.length_squared());
}

TEST(raf_math, basic_vec2_subtraction)
{
  const Vec2d src{ 0, 0 };
  const Vec2d dest{ 2, 2 };

  auto diff = dest - src;
  ASSERT_DOUBLE_EQ(8, diff.length_squared());
  diff = src - dest;
  ASSERT_DOUBLE_EQ(8, diff.length_squared());
}

TEST(raf_math, basic_vec2_multiplication)
{
  const Vec2d src{ 3, 3 };
  const Vec2d dest{ 6, 6 };

  auto extended_vector = src * 2.0;
  ASSERT_DOUBLE_EQ(6, extended_vector.x());
  ASSERT_DOUBLE_EQ(6, extended_vector.y());

  auto dot_product = src * dest;
  ASSERT_DOUBLE_EQ(36, dot_product);
}

TEST(raf_math, basic_vec2_mid_point_mul)
{
  const Vec2d src{ 2, 2 };
  const Vec2d dest{ 4, 4 };

  const Vec2d midpoint = (src + dest) / 2;
  ASSERT_DOUBLE_EQ(3, midpoint.x());
  ASSERT_DOUBLE_EQ(3, midpoint.y());
}

TEST(raf_math, basic_vec2_mid_point_diff)
{
  const Vec2d src{ 2, 2 };
  const Vec2d dest{ 4, 4 };
  const Vec2d diff = dest - src;
  const Vec2d midpoint_with_diff = (src + (diff / 2));
  ASSERT_DOUBLE_EQ(3, midpoint_with_diff.x());
  ASSERT_DOUBLE_EQ(3, midpoint_with_diff.y());
}

TEST(raf_math, circle_intersection)
{
  // https://www.geogebra.org/geometry
  const Vec2d src{ 0, 0 };
  const Vec2d dest{ 0, 2 };
  const double radius = 5.0;
  const auto expected_intersection_values = {
    Vec2d{ 1.000, 4.898979485},
    Vec2d{ 1.000, 4.898979485},
    };

  // http://www.analyzemath.com/CircleEq/circle_intersection.html
  // https://math.stackexchange.com/questions/256100/how-can-i-find-the-points-at-which-two-circles-intersect?newreg=ff3d16a7224046ef86be9762ab320387
  // https://stackoverflow.com/questions/8727065/intersection-points-of-two-circles-in-c
  // http://paulbourke.net/geometry/circlesphere/tvoght.c
  // http://www.cpphub.com/2015/03/intersection-of-circles-implementation.html

  Circle a(radius, src);
  Circle b(radius, dest);

  std::array<Vec2d, 2> intersections;
  if (intersects(a, b, intersections)) {
    std::cout << intersections[0] << intersections[1] << std::endl;
  }
}

// Add test for inner tangent
// Can be used for perfect path finding!
// http://www.ambrsoft.com/TrigoCalc/Circles2/Circles2Tangent_.htm

TEST(raf_math, orientation)
{
  const Vec2d src{ 3, 3 };
  const Vec2d dest{ 6, 6 };

  auto src_to_dest = src.orient_towards_in_rad(dest);
  auto dest_to_src = dest.orient_towards_in_rad(src);

  ASSERT_DOUBLE_EQ(45, angle_rad_to_deg_clipped(src_to_dest));
}

TEST(raf_math, velocity_simple)
{
  Velocity a{ 7, 45 };

  ASSERT_EQ(7, a.thrust());
  ASSERT_EQ(45, a.angle_in_degrees());
}

TEST(raf_math, velocity_looped)
{
  for (int thrust = -7; thrust <= 7; thrust++) {
    for (int angle_deg = 0; angle_deg < 180; angle_deg++) {
      Velocity a{ thrust, angle_deg };

      // In this application thrust is always positive by definition as angle
      // provides direction
      if (thrust < 0) {
        ASSERT_EQ(std::abs(thrust), a.thrust());
        ASSERT_EQ(angle_deg+180, a.angle_in_degrees());
      } else if (thrust > 0) {
        ASSERT_EQ(thrust, a.thrust());
        ASSERT_EQ(angle_deg, a.angle_in_degrees());
      } else {
        ASSERT_EQ(0, a.thrust());
        // FIXME
        // Signed zero (-0) in some vectors breaks this test.
        //ASSERT_EQ(0, a.angle_in_degrees());
      }
    }
  }
}

TEST(raf_math, velocity_angle_wrapped)
{
  for (int thrust = 1; thrust < 2; thrust++) {
    for (int angle_deg = 0; angle_deg < 360; angle_deg++) {
      Velocity a{ thrust, angle_deg };
      ASSERT_EQ(thrust, a.thrust());
      if (thrust < 0) {
        ASSERT_EQ(angle_deg + 180, a.angle_in_degrees());
      } else {
        ASSERT_EQ(angle_deg, a.angle_in_degrees());
      }
    }
  }
}



