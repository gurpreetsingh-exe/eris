#pragma once

#include "bits.hh"
#include "square.hh"
#include "types.hh"
#include "utils.hh"

namespace eris {

template <int S>
class BitboardIter {
public:
  BitboardIter() = delete;
  explicit BitboardIter(u64 bits) : _bits(nbitmask(S * S) & bits) {}

  auto begin() const -> BitIterator<Square<S>> { return _bits; }
  auto end() const -> BitIterator<Square<S>> { return 0; }

private:
  u64 _bits;
};

class Bitboard {
public:
  constexpr Bitboard() = default;
  constexpr Bitboard(const Bitboard&) = default;
  constexpr auto operator=(const Bitboard&) -> Bitboard& = default;
  constexpr Bitboard(u64 bits) : _bits(bits) {}
  constexpr auto operator*() const -> u64 { return _bits; }
  constexpr auto get(int index) const -> bool {
    ASSERT(index >= 0 and index < 64, "`{}`", index);
    return _bits & (1ULL << index);
  }

  template <int S>
  constexpr auto get(Square<S> sq) const -> bool {
    return get(u8(*sq));
  }

  constexpr auto pop(u8 index) -> void { _bits &= ~(1ULL << index); }

  template <int S>
  constexpr auto pop(Square<S> sq) -> void {
    pop(u8(*sq));
  }

  template <int S>
  constexpr auto operator|=(Square<S> sq) -> void {
    _bits |= sq.as_board();
  }

  template <int S>
  constexpr auto operator^=(Square<S> sq) -> void {
    _bits ^= sq.as_board();
  }

  constexpr auto operator^=(u64 n) -> void { _bits ^= n; }
  constexpr auto operator|=(u64 n) -> void { _bits |= n; }
  constexpr auto operator|=(Bitboard other) -> void { _bits |= other._bits; }
  constexpr auto operator>>(int n) const -> Bitboard { return _bits >> n; }
  constexpr auto operator~() const -> Bitboard { return ~_bits; }
  constexpr auto operator<<(int n) const -> Bitboard { return _bits << n; }
  constexpr auto more_than_one() const -> bool { return _bits & (_bits - 1); }
  constexpr auto count() const -> int { return __builtin_popcountll(_bits); }
  constexpr auto empty() const -> bool { return _bits == 0ULL; }

  auto operator&=(const Bitboard& a) -> Bitboard& {
    _bits &= a._bits;
    return *this;
  }

  friend auto swap(Bitboard& a, Bitboard& b) -> void {
    std::swap(a._bits, b._bits);
  }

  friend auto operator|(const Bitboard& a, const Bitboard& b) -> Bitboard {
    return { a._bits | b._bits };
  }

  friend auto operator&(const Bitboard& a, const Bitboard& b) -> Bitboard {
    return { a._bits & b._bits };
  }

  friend auto operator^(const Bitboard& a, const Bitboard& b) -> Bitboard {
    return { a._bits ^ b._bits };
  }

  template <int S>
  friend auto operator^(const Bitboard& a, const Square<S>& b) -> Bitboard {
    return { a._bits ^ b.as_board() };
  }

  template <int S>
  friend auto operator&(const Bitboard& a, const Square<S>& b) -> Bitboard {
    return { a._bits & b.as_board() };
  }

  template <int S>
  friend auto operator|(const Bitboard& a, const Square<S>& b) -> Bitboard {
    return { a._bits | b.as_board() };
  }

  constexpr auto operator==(Bitboard other) const -> bool {
    return _bits == other._bits;
  }

  template <int S>
  auto to_string() const -> std::string {
    auto s = std::string();
    for (int j = S - 1; j >= 0; --j) {
      for (int i = 0; i < S; ++i) {
        s += fmt::format("{} ", get(i + j * S) ? chars.onebit : chars.zerobit);
      }
      s += "\n";
    }
    return s;
  }

  template <int S>
  auto print() const -> void {
    fmt::print("    {}", chars.topleft);
    for (int i = 0; i < S * 2 + 1; ++i) { fmt::print(chars.vbar); }
    fmt::println(chars.topright);
    for (int j = S - 1; j >= 0; --j) {
      fmt::print("  {} {} ", j + 1, chars.hbar);
      for (int i = 0; i < S; ++i) {
        fmt::print("{} ", get(i + j * S) ? chars.onebit : chars.zerobit);
      }
      fmt::println(chars.hbar);
    }
    fmt::print("    {}", chars.bottomleft);
    for (int i = 0; i < S * 2 + 1; ++i) { fmt::print(chars.vbar); }
    fmt::println(chars.bottomright);
    fmt::print("     ");
    for (int i = 'a'; i < 'a' + S; ++i) { fmt::print(" {}", char(i)); }
    fmt::print("\n\n");
  }

  template <int S>
  auto iter() const -> BitboardIter<S> {
    return BitboardIter<S>(_bits);
  }

private:
  u64 _bits;

  friend struct fmt::formatter<Bitboard>;
};

template <int S>
auto iter(Bitboard b) -> BitboardIter<S> {
  return BitboardIter<S>(*b);
}

} // namespace eris

FMT(eris::Bitboard, "{}", std::bitset<64>(v._bits).to_string());
