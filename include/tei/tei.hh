#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

#include "board.hh"

namespace eris {

template <typename T>
class Channel {
public:
  auto send(T value) -> void {
    _mtx.lock();
    _queue.push(std::move(value));
    _mtx.unlock();
    _cv.notify_one();
  }

  auto receive() -> T {
    auto lock = std::unique_lock<std::mutex>(_mtx);
    _cv.wait(lock, [this] { return not _queue.empty(); });
    auto value = std::move(_queue.front());
    _queue.pop();
    return value;
  }

private:
  std::queue<T> _queue;
  std::mutex _mtx;
  std::condition_variable _cv;
};

struct GoCommand {
  int depth = 0;
  int nodes = 0;
};

class Tei {
public:
  Tei();
  ~Tei();

public:
  auto tick() -> void;

  template <int S>
  auto teinewgame() -> void {
    auto board = Board<S>();
    for (;;) {
      auto command = _ch.receive();
      auto tokens = split(command, ' ');
      auto cmd = tokens[0];
      if (cmd == "position") {
        if (tokens[1] == "startpos") {
          ASSERT(tokens[2] == "moves");
          for (usize i = 3; i < tokens.size(); ++i) {
            board.make_move(Move<S>(tokens[i]));
          }
        } else if (tokens[1] == "tps") {
          auto tps = fmt::format(
              "{}", fmt::join({ tokens[2], tokens[3], tokens[4] }, " "));
          board.tps(tps);
        }
      } else if (cmd == "stop") {
        return;
      } else if (cmd == "isready") {
        write("readyok");
      } else if (cmd == "go") {
        auto get_n = [&](usize n) { return std::stoi(tokens[n]); };
        auto gocmd = GoCommand();
        usize i = 1;
        while (i < tokens.size()) {
          const auto& sub = tokens[i];
          if (sub == "depth") {
            gocmd.depth = get_n(i + 1);
            i += 2;
            continue;
          } else if (sub == "nodes") {
            gocmd.nodes = get_n(i + 1);
            i += 2;
            continue;
          } else {
            fmt::println(stderr, "unknown subcommand in go: `{}`", sub);
          }
          i += 1;
        }
        go(board, gocmd);
      } else {
        fmt::println(stderr, "unknown command: `{}`", cmd);
      }
    }
  }

  template <int S>
  auto go(Board<S>& board, GoCommand cmd) -> void {
    auto moves = MoveList<S>();
    board.generate_moves(moves);
    fmt::println(stderr, "{} {}", cmd.depth, cmd.nodes);
  }

private:
  auto write(std::string msg) -> void { fmt::println("{}", msg); }

private:
  Channel<std::string> _ch = {};
  std::thread _thread;
};

} // namespace eris
