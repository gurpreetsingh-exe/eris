#pragma once

#include "board.hh"

// #define BULK_COUNTING

namespace eris {

template <int S>
auto perft(Board<S>& board, int depth) -> usize {
  if (depth == 0) {
    return 1;
  }

  usize nodes = 0;
  auto moves = std::vector<Move<S>>();
  board.generate_moves(moves);

#ifdef BULK_COUNTING
  if (depth == 1) {
    return moves.size();
  }
#endif

  for (const auto move : moves) {
    board.make_move(move);
    nodes += perft(board, depth - 1);
    board.unmake_move(move);
  }
  return nodes;
}

template <int S>
auto perft_driver(Board<S>& board, int depth) -> usize {
  ASSERT(depth >= 1);

  usize nodes = 0;
  auto moves = std::vector<Move<S>>();
  board.generate_moves(moves);

  for (const auto move : moves) {
    board.make_move(move);
    auto n = perft(board, depth - 1);
    nodes += n;
    fmt::println("{}: {}", move.to_string(), n);
    board.unmake_move(move);
  }

  return nodes;
}

} // namespace eris
