#pragma once

#include "bitboard.hh"

namespace eris {

inline u8 orthogonally_adjacent_squares3x3[9] = {};
inline u8 orthogonally_adjacent_squares4x4[16] = {};
inline u8 orthogonally_adjacent_squares5x5[25] = {};
inline u8 orthogonally_adjacent_squares6x6[36] = {};
inline u8 orthogonally_adjacent_squares7x7[49] = {};
inline u8 orthogonally_adjacent_squares8x8[64] = {};

template <int S>
constexpr auto orthogonally_adjacent_squares(int i) -> u8 {
  if constexpr (S == 3) {
    return orthogonally_adjacent_squares3x3[i];
  } else if constexpr (S == 4) {
    return orthogonally_adjacent_squares4x4[i];
  } else if constexpr (S == 5) {
    return orthogonally_adjacent_squares5x5[i];
  } else if constexpr (S == 6) {
    return orthogonally_adjacent_squares6x6[i];
  } else if constexpr (S == 7) {
    return orthogonally_adjacent_squares7x7[i];
  } else if constexpr (S == 8) {
    return orthogonally_adjacent_squares8x8[i];
  }

  auto s = S;
  ASSERT(false, "unexpected board size `{}`", s);
}

template <int S>
constexpr auto orthogonally_adjacent_squares(Square<S> square) -> u8 {
  return orthogonally_adjacent_squares<S>(*square);
}

template <int S>
auto init_orthogonally_adjacent_squares() -> void {
  u8* arr = nullptr;
  if constexpr (S == 3) {
    arr = orthogonally_adjacent_squares3x3;
  } else if constexpr (S == 4) {
    arr = orthogonally_adjacent_squares4x4;
  } else if constexpr (S == 5) {
    arr = orthogonally_adjacent_squares5x5;
  } else if constexpr (S == 6) {
    arr = orthogonally_adjacent_squares6x6;
  } else if constexpr (S == 7) {
    arr = orthogonally_adjacent_squares7x7;
  } else if constexpr (S == 8) {
    arr = orthogonally_adjacent_squares8x8;
  }

  for (const auto square : iter<S>(nbitmask(S * S))) {
    u8 bits = 0;
    if (square.rank() != 0) {
      bits |= 0b0100;
    }
    if (square.rank() != S - 1) {
      bits |= 0b0001;
    }
    if (square.file() != 0) {
      bits |= 0b1000;
    }
    if (square.file() != S - 1) {
      bits |= 0b0010;
    }

    arr[*square] = bits;
  }
}

} // namespace eris
