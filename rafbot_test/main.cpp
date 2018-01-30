#include "gtest/gtest.h"

#include "raf/log.hpp"

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);

  hlt::Log::open("bottest.log");
  return RUN_ALL_TESTS();
}
