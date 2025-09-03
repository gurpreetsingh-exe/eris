#include "board.hh"
#include "engine.hh"
#include "perft.hh"
#include "tables.hh"

using namespace eris;

auto main() -> int {
  init_engine();

  auto board = Board<3>();
  board.print();
  TIME("Perft", { fmt::println("Total Nodes: {}", perft<3>(board, 5)); });
}
