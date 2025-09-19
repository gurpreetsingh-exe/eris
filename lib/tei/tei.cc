#include "tei/tei.hh"

#include <iostream>

namespace eris {

Tei::Tei() {
  _thread = std::thread([&] {
    for (;;) {
      auto command = std::string();
      std::cin >> std::ws;
      std::getline(std::cin, command);
      _ch.send(command);
      std::this_thread::sleep_for(chr::milliseconds(1));
    }
  });
}

Tei::~Tei() { _thread.join(); }

auto Tei::tick() -> void {
  for (;;) {
    auto command = _ch.receive();
    auto tokens = split(command, ' ');
    auto cmd = tokens[0];

    if (cmd == "tei") {
      write("id name Eris");
      write("id author Gurpreet Singh");
      write("teiok");
    } else if (cmd == "isready") {
      write("readyok");
    } else if (cmd == "setoption") {
      // TODO: set options
    } else if (cmd == "teinewgame") {
      auto size = std::stoi(tokens[1]);
      switch (size) {
#define X(_S)                                                                  \
  case _S: {                                                                   \
    teinewgame<_S>();                                                          \
  } break;
        BOARD_SIZE_ITER
#undef X
        default:
          PANIC("unexpected size `{}`", size);
      }
    } else {
      fmt::println(stderr, "unknown command: `{}`", cmd);
    }
  }
}

} // namespace eris
