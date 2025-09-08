#include <gtest/gtest.h>

#include "perft.hh"

using namespace eris;

TEST(Perft, Board3) {
  eris::usize perf_results[] = {
    1, 9, 72, 1200, 17792, 271812, 3712952, 52364896, 679639648, 9209357840,
  };

  for (int depth = 0; depth < 6; ++depth) {
    auto board = Board<3>();
    EXPECT_EQ(perft<3>(board, depth), perf_results[depth]);
  }
}
