#include <gtest/gtest.h>

#include "move.hh"

using namespace eris;

TEST(Move, Construct) {
  for (int i = 0; i < 9; ++i) {
    auto move = Move<3>::place(Square<3>(i), CAP);
    EXPECT_EQ(move.square(), Square<3>(i));
  }
}

TEST(Move, TEST1) {
  auto move = Move<8>::place(Square<8>("a5"), CAP);
  EXPECT_TRUE(move.is_place());
  EXPECT_EQ(move.stone(), CAP);

  auto spread = Spread<8>();
  spread.push(1, 1);

  move = Move<8>::spread(Square<8>("a5"), NORTH, spread);
  EXPECT_TRUE(not move.is_place());
  EXPECT_EQ(move.direction(), NORTH);
  EXPECT_EQ(move.square(), Square<8>("a5"));
}
