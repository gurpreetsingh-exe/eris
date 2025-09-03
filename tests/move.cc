#include <gtest/gtest.h>

#include "move.hh"

using namespace eris;

TEST(Move, Construct) {
  for (int i = 0; i < 9; ++i) {
    auto move = Move<3>(PLACE, W_CAP, Square<3>(i));
    EXPECT_EQ(move.square(), Square<3>(i));
  }
}
