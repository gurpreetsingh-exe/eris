#include "board.hh"
#include "engine.hh"
#include "perft.hh"
#include "tables.hh"

using namespace eris;

auto main(int argc, char** argv) -> int {
  init_engine();

  auto board = Board<5>();
  board.print();
}
