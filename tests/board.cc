#include <gtest/gtest.h>

#include "board.hh"

using namespace eris;

#define FIRST_MOVE(S, tps)                                                     \
  {                                                                            \
    auto board = ::eris::Board<S>::from(tps);                                  \
    auto moves = std::vector<::eris::Move<S>>();                               \
    board.generate_moves(moves);                                               \
    EXPECT_EQ(moves.size(), S* S);                                             \
  }

TEST(Board, FirstMove) {
  FIRST_MOVE(3, "x3/x3/x3 1 1");
  FIRST_MOVE(4, "x4/x4/x4/x4 1 1");
  FIRST_MOVE(5, "x5/x5/x5/x5/x5 1 1");
  FIRST_MOVE(6, "x6/x6/x6/x6/x6/x6 1 1");
  FIRST_MOVE(7, "x7/x7/x7/x7/x7/x7/x7 1 1");
  FIRST_MOVE(8, "x8/x8/x8/x8/x8/x8/x8/x8 1 1");
}

TEST(Board, StoneBitboards) {
  auto board = Board<4>();
  board.put_stone(B_FLAT, Square<4>("a1"));
  board.put_stone(W_FLAT, Square<4>("a2"));
  board.put_stone(W_WALL, Square<4>("b3"));
  board.put_stone(B_CAP, Square<4>("a4"));
  board.put_stone(B_WALL, Square<4>("d1"));

  EXPECT_EQ(board.stones<FLAT>(), (1ULL << 0) | (1ULL << 4));
  EXPECT_EQ(board.stones<WALL>(), (1ULL << 3) | (1ULL << 9));
  EXPECT_EQ(board.stones<CAP>(), (1ULL << 12));

  EXPECT_EQ(board.stones<BLACK>(), (1ULL << 0) | (1ULL << 12) | (1ULL << 3));
  EXPECT_EQ(board.stones<WHITE>(), (1ULL << 4) | (1ULL << 9));

  EXPECT_EQ((board.stones<FLAT, WHITE>()), 1ULL << 4);
  EXPECT_EQ((board.stones<WALL, WHITE>()), 1ULL << 9);
  EXPECT_EQ((board.stones<CAP, WHITE>()), 0);

  EXPECT_EQ((board.stones<FLAT, BLACK>()), 1ULL);
  EXPECT_EQ((board.stones<WALL, BLACK>()), 1ULL << 3);
  EXPECT_EQ((board.stones<CAP, BLACK>()), 1ULL << 12);
}

TEST(Board, PutStone5x5) {
  auto board = Board<5>();
  board.put_stone(W_FLAT, Square<5>("a1"));
  board.put_stone(W_CAP, Square<5>("c3"));
  board.put_stone(B_WALL, Square<5>("b4"));
  board.put_stone(B_FLAT, Square<5>("e5"));
  EXPECT_EQ(board.top(0), W_FLAT);
  EXPECT_EQ(board.top(12), W_CAP);
  EXPECT_EQ(board.top(16), B_WALL);
  EXPECT_EQ(board.top(24), B_FLAT);
}

TEST(Board, StackingWhiteBlackOnSameSqBitboard) {
  auto board = Board<3>();
  auto b2 = Square<3>("b2");
  board.put_stone(B_FLAT, b2);
  board.put_stone(W_CAP, b2);

  EXPECT_EQ(board.stones<BLACK>(), 0);
  EXPECT_EQ(board.stones<B_FLAT>(), 0);
  EXPECT_EQ(board.stones<FLAT>(), 0);

  EXPECT_EQ(board.stones<WHITE>(), b2.as_board());
  EXPECT_EQ(board.stones<W_FLAT>(), 0);
}

TEST(Board, TakeStoneAndBitboard5x5) {
  auto board = Board<5>();
  auto a5 = Square<5>("a5");
  board.put_stone(W_FLAT, a5);
  board.put_stone(B_FLAT, a5);
  board.put_stone(W_FLAT, a5);
  board.put_stone(W_FLAT, a5);
  board.put_stone(B_WALL, a5);

  auto c3 = Square<5>("c3");
  board.put_stone(B_FLAT, c3);
  board.put_stone(W_FLAT, c3);
  board.put_stone(W_FLAT, c3);
  board.put_stone(B_FLAT, c3);
  board.put_stone(W_CAP, c3);

  board.put_stone(B_CAP, Square<5>("d4"));
  board.put_stone(W_WALL, Square<5>("e4"));

  EXPECT_EQ(board.stones<BLACK>(), (1ULL << 20) | (1ULL << 18));
  EXPECT_EQ(board.stones<WHITE>(), (1ULL << 12) | (1ULL << 19));
  EXPECT_EQ(board.stones<W_CAP>(), c3.as_board());

  EXPECT_EQ(board.take_stone(a5), B_WALL);

  EXPECT_EQ(board.stones<BLACK>(), (1ULL << 18));
  EXPECT_EQ(board.stones<WHITE>(), (1ULL << 12) | (1ULL << 19) | (1ULL << 20));

  EXPECT_EQ(board.take_stone(c3), W_CAP);
}

TEST(Board, Stack5x5) {
  auto board = Board<5>();
  auto square = Square<5>("a1");
  board.put_stone(W_FLAT, square);
  EXPECT_EQ(board.top(square), W_FLAT);

  board.put_stone(W_FLAT, square);
  board.put_stone(B_FLAT, square);
  board.put_stone(W_FLAT, square);
  board.put_stone(B_FLAT, square);
  EXPECT_EQ(board.top(square), B_FLAT);

  EXPECT_EQ(board.stack(square).height(), 4);
  EXPECT_EQ(*board.stack(square), 0b1101);

  EXPECT_EQ(board.take_stone(square), B_FLAT);

  EXPECT_EQ(board.stack(square).height(), 3);
  EXPECT_EQ(*board.stack(square), 0b110);

  EXPECT_EQ(board.take_stone(square), W_FLAT);
  EXPECT_EQ(board.take_stone(square), B_FLAT);
  EXPECT_EQ(board.take_stone(square), W_FLAT);

  EXPECT_EQ(board.stack(square).height(), 0);
  EXPECT_EQ(*board.stack(square), 0);

  EXPECT_EQ(board.take_stone(square), W_FLAT);

  EXPECT_EQ(board.stack(square).height(), 0);
  EXPECT_EQ(*board.stack(square), 0);

  EXPECT_EQ(board.take_stone(square), NO_STONE);
}

TEST(Board, RandomPosition) {
  auto board = Board<5>::from("x3,12,2S/x,22S,22C,11,21/121,212,12,1121C,1212S/"
                              "21S,1,21,211S,12S/x,21S,2,x2 1 26");

  auto bb_from_indices = [](std::vector<int> indices) {
    auto b = Bitboard();
    for (const auto idx : indices) { b |= Square<5>(idx); }
    return b;
  };
  EXPECT_EQ(board.stones<BLACK>(),
            bb_from_indices({ 2, 9, 11, 12, 14, 16, 17, 23, 24 }));
  EXPECT_EQ(board.stones<WHITE>(),
            bb_from_indices({ 1, 5, 6, 7, 8, 10, 13, 18, 19 }));
  EXPECT_EQ(board.stones<FLAT>(),
            bb_from_indices({ 2, 6, 7, 10, 11, 12, 18, 19, 23 }));
  EXPECT_EQ(board.stones<WALL>(), bb_from_indices({ 1, 5, 8, 9, 14, 16, 24 }));
  EXPECT_EQ(board.stones<CAP>(), bb_from_indices({ 13, 17 }));
}

TEST(Board, Moves) {
  auto board = Board<5>();
  auto move = Move<5>::place(Square<5>("a1"), FLAT);
  board.make_move(move);
  board.unmake_move(move);
  EXPECT_EQ(board.stones(), 0ULL);
  EXPECT_EQ(board.stones<W_FLAT>(), 0ULL);
}

TEST(Board, SetupGameFromMoves) {
  auto board = Board<5>();
  std::vector<const char*> moves = { "a1",   "a3",  "a2",  "b1",
                                     "c1",   "b1<", "a3-", "b1",
                                     "2a2-", "b1<", "c1<", "5a1>122" };
  for (auto move : moves) { board.make_move(Move<5>(move)); }
  EXPECT_EQ(board.stones<BLACK>(), 0b1010);
  EXPECT_EQ(board.stones<WHITE>(), 0b0100);

  EXPECT_EQ(board.stones<W_FLAT>(), 0b0100);
  EXPECT_EQ(board.stones<B_FLAT>(), 0b1010);
  auto s1 = board.stack(Square<5>("b1"));
  EXPECT_EQ(s1.height(), 1);
  EXPECT_EQ(*s1, 0b1);
  auto s2 = board.stack(Square<5>("c1"));
  EXPECT_EQ(s2.height(), 1);
  EXPECT_EQ(*s2, 0b0);
  auto s3 = board.stack(Square<5>("d1"));
  EXPECT_EQ(s3.height(), 1);
  EXPECT_EQ(*s3, 0b1);
}
