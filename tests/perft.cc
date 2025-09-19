#include <gtest/gtest.h>

#include "perft.hh"

using namespace eris;

TEST(Perft, Board3) {
  usize perf_results[] = {
    1, 9, 72, 1200, 17792, 271812, 3712952, 52364896, 679639648, 9209357840,
  };

  for (int depth = 0; depth < 9; ++depth) {
    auto board = Board<3>();
    EXPECT_EQ(perft<3>(board, depth), perf_results[depth]);
  }
}

TEST(Perft, Board4) {
  usize perf_results[] = {
    1, 16, 240, 7440, 216464, 6468872, 181954216, 5231815136,
  };

  for (int depth = 0; depth < 8; ++depth) {
    auto board = Board<4>();
    EXPECT_EQ(perft<4>(board, depth), perf_results[depth]);
  }
}

TEST(Perft, Board5) {
  usize perf_results[] = {
    1, 25, 600, 43320, 2999784, 187855252, 11293470152,
  };

  for (int depth = 0; depth < 7; ++depth) {
    auto board = Board<5>();
    EXPECT_EQ(perft<5>(board, depth), perf_results[depth]);
  }
}

TEST(Perft, Board6) {
  usize perf_results[] = {
    1, 36, 1260, 132720, 13586048, 1253506520,
  };

  for (int depth = 0; depth < 6; ++depth) {
    auto board = Board<6>();
    EXPECT_EQ(perft<6>(board, depth), perf_results[depth]);
  }
}

TEST(Perft, Board7) {
  usize perf_results[] = {
    1, 49, 2352, 339696, 48051008, 6813380628,
  };

  for (int depth = 0; depth < 6; ++depth) {
    auto board = Board<7>();
    EXPECT_EQ(perft<7>(board, depth), perf_results[depth]);
  }
}

TEST(Perft, Board8) {
  usize perf_results[] = {
    1, 64, 4032, 764064, 142512336, 26642455192,
  };

  for (int depth = 0; depth < 6; ++depth) {
    auto board = Board<8>();
    EXPECT_EQ(perft<8>(board, depth), perf_results[depth]);
  }
}
