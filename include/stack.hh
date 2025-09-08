#pragma once

#include "types.hh"

namespace eris {

class Stack {
public:
  constexpr Stack() noexcept = default;
  constexpr Stack(u64 stack, int height);

  auto top() const -> Color { return Color(_stack & 1); }
  auto height() const -> u8 { return _height; }
  auto operator*() const -> u64 { return _stack; }

  auto push(Color c) -> void;
  auto push(Stack other) -> void;
  auto take(int n) -> Stack;
  auto take_back(int n) -> Stack;
  auto pop(int n = 1) -> Color;
  auto pop_back(int n = 1) -> Color;
  auto to_string() const -> std::string;

private:
  u64 _stack = 0ULL;
  u8 _height = 0;
};

} // namespace eris
