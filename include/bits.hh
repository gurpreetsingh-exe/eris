#pragma once

namespace eris {

inline auto nbitmask(int n) -> u64 {
  ASSERT(n >= 0 and n <= 64);
  return (1ULL << n) - 1;
}

inline auto lsb(u64 n) -> int {
  ASSERT(n);
#if defined(__GNUC__)
  return __builtin_ctzll(n);
#elif defined(_MSC_VER)
  unsigned long idx;
  _BitScanForward64(&idx, n);
  return idx;
#else
  static_assert(false, "unsupported compiler");
#endif
}

inline auto popcnt(u64 bits) -> int {
#if defined(__GNUC__)
  return __builtin_popcountll(bits);
#elif defined(_MSC_VER)
  return __popcnt(bits)
#else
  static_assert(false, "unsupported compiler");
#endif
}

template <typename T>
class BitIterator {
public:
  using iterator_category = std::input_iterator_tag;
  using difference_type = T;
  using value_type = T;
  using pointer = T*;
  using reference = T&;

  BitIterator(u64 value) : _value(value) {};
  auto operator!=(const BitIterator& other) -> bool {
    return _value != other._value;
  }

  auto operator++() -> void { _value &= (_value - 1); }
  auto operator*() const -> T { return lsb(_value); }

private:
  u64 _value;
};

class IterateBits {
public:
  IterateBits(u64 value) : _value(value) {}
  auto begin() -> BitIterator<int> { return _value; }
  auto end() -> BitIterator<int> { return 0; }

private:
  u64 _value;
};

} // namespace eris
