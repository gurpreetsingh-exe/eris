#pragma once

#include "board.hh"

#define BULK_COUNTING

namespace eris {

template <int S>
auto perft(Board<S>& board, int depth) -> usize {
  if (depth == 0) {
    return 1;
  }

  if (board.road()) {
    return 0;
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
    auto tmp = board;
    board.make_move(move);
    nodes += perft(board, depth - 1);
    board = tmp;
    // board.unmake_move(move);
  }
  return nodes;
}

template <int S>
auto perft_driver(Board<S>& board, int depth) -> usize {
  ASSERT(depth >= 1);

  usize nodes = 0;
  auto moves = std::vector<Move<S>>();
  board.generate_moves(moves);

  if (depth == 1) {
    return moves.size();
  }

  for (const auto move : moves) {
    auto tmp = board;
    board.make_move(move);
    auto n = perft(board, depth - 1);
    nodes += n;
    fmt::println("{}: {}", move.to_string(), n);
    board = tmp;
    // board.unmake_move(move);
  }

  return nodes;
}

template <int S>
auto perft(int depth) -> void {
  auto board = Board<S>();
  auto [duration, nodes] = timeit<int>([&] { return perft<S>(board, depth); });
  fmt::println("{}: depth {}, {:.3f} ms, {:.1f} Mnps", nodes, depth,
               duration.millis(), f32(nodes) / duration.micros());
}

} // namespace eris
