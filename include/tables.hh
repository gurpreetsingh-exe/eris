#pragma once

#include "bitboard.hh"
#include "square.hh"

namespace eris {

template <int S>
struct SquareCache {
  Square<S> adjacent[4] = {};
  u8 direction_bits = {};
};

#define SQUARE_CACHE(_S) square_cache##_S

#define X(_S) inline SquareCache<_S> SQUARE_CACHE(_S)[_S * _S] = {};
BOARD_SIZE_ITER
#undef X

template <int S>
constexpr auto orthogonally_adjacent_squares(int i) -> u8 {
#define X(_S)                                                                  \
  if constexpr (_S == S) {                                                     \
    return SQUARE_CACHE(_S)[i].direction_bits;                                 \
  }
  BOARD_SIZE_ITER
#undef X

  auto s = S;
  ASSERT(false, "unexpected board size `{}`", s);
}

template <int S>
constexpr auto orthogonally_adjacent_squares(Square<S> square) -> u8 {
  return orthogonally_adjacent_squares<S>(*square);
}

template <int S>
auto find_in_direction(Square<S> square, Direction direction) -> Square<S> {
#define X(_S)                                                                  \
  if constexpr (_S == S) {                                                     \
    return SQUARE_CACHE(_S)[*square].adjacent[direction];                      \
  }
  BOARD_SIZE_ITER
#undef X
}

template <int S>
auto init_square_cache() -> void {
  SquareCache<S>* arr = nullptr;
#define X(_S)                                                                  \
  if constexpr (_S == S) {                                                     \
    arr = SQUARE_CACHE(_S);                                                    \
  }
  BOARD_SIZE_ITER
#undef X

  for (const auto square : iter<S>(nbitmask(S * S))) {
    u8 bits = 0;
    auto& square_cache_entry = arr[*square];
    for (int i = 0; i < 4; ++i) { square_cache_entry.adjacent[i] = square; }

    if (square.rank() != 0) {
      bits |= 1 << SOUTH;
      square_cache_entry.adjacent[SOUTH] = square + SOUTH;
    }
    if (square.rank() != S - 1) {
      bits |= 1 << NORTH;
      square_cache_entry.adjacent[NORTH] = square + NORTH;
    }
    if (square.file() != 0) {
      bits |= 1 << WEST;
      square_cache_entry.adjacent[WEST] = square + WEST;
    }
    if (square.file() != S - 1) {
      bits |= 1 << EAST;
      square_cache_entry.adjacent[EAST] = square + EAST;
    }

    square_cache_entry.direction_bits = bits;
  }
}

} // namespace eris
