#include "pch.h"

TEST(TestCaseName, TestName) {
  EXPECT_EQ(1, 1);
  EXPECT_TRUE(true);
}


struct LocationFinderParams {
  int range_to_search = 5;
};

class Location {
public:
  Location(double x, double y) : x(x), y(y) {}
  friend Location operator-(const Location& lhs, const Location& rhs) {
    return { lhs.x - rhs.x, lhs.y - rhs.y };
  }

  double length_squared() const {
    return x * x + y * y;
  }
private:
  double x;
  double y;
};

double distance(const Location& a, const Location& b) {
  const auto delta = a - b;
  return std::sqrt(delta.length_squared());
}

