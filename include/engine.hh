#pragma once

#include "tables.hh"

namespace eris {

auto init_engine() -> void {
  init_orthogonally_adjacent_squares<3>();
  init_orthogonally_adjacent_squares<4>();
  init_orthogonally_adjacent_squares<5>();
  init_orthogonally_adjacent_squares<6>();
  init_orthogonally_adjacent_squares<7>();
  init_orthogonally_adjacent_squares<8>();
}

} // namespace eris
