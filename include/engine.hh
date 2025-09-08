#pragma once

#include "tables.hh"

namespace eris {

auto init_engine() -> void {
#define X(_S) init_square_cache<_S>();
  BOARD_SIZE_ITER
#undef X
}

} // namespace eris
