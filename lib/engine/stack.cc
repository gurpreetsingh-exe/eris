#include "stack.hh"
#include "bitboard.hh"

namespace eris {

constexpr Stack::Stack(u64 stack, int height)
    : _stack(stack), _height(u8(height)) {
  ASSERT(height >= 0 and height <= 64);
}

auto Stack::push(Color c) -> void {
  _stack <<= 1;
  _stack |= c;
  _height += 1;
}

auto Stack::push(Stack other) -> void {
  if (not _height) {
    *this = other;
    return;
  }

  _stack <<= other.height();
  _stack |= *other;
  _height += other.height();
}

auto Stack::take(int n) -> Stack {
  auto taken = Stack(_stack & nbitmask(n), n);
  pop(n);
  return taken;
}

auto Stack::take_back(int n) -> Stack {
  auto taken = Stack(_stack >> (_height - n), n);
  pop_back(n);
  return taken;
}

auto Stack::pop(int n) -> Color {
  auto c = top();
  _stack >>= n;
  _height -= u8(n);
  return c;
}

auto Stack::pop_back(int n) -> Color {
  auto c = top();
  _stack &= nbitmask(_height - n);
  _height -= u8(n);
  return c;
}

auto Stack::to_string() const -> std::string {
  if (not _height) {
    return "-";
  }
  auto s = std::string();
  for (int i = _height - 1; i >= 0; --i) {
    s += std::to_string(int(Bitboard(_stack).get(i)));
  }
  return s;
}

} // namespace eris
