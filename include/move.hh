#pragma once

#include "square.hh"
#include "types.hh"

namespace eris {

/// Adapted from tiltak
/// https://github.com/MortenLohne/tiltak/blob/f1ec5e196f035594182f038b23397e7aa0152af8/src/position/utils.rs#L365
template <int S>
class Spread {
public:
  Spread() = default;
  Spread(u8 data) : _inner(data) {}

  auto first() -> int {
    ASSERT(_inner != 0);
    return 8 - __builtin_clz(((int)_inner) << 24);
  }

  auto size() -> int { return std::popcount(_inner); }

  auto push(int to_take, int held) -> void {
    ASSERT(held > 0);

    auto to_drop = held - to_take;

    if (_inner != 0) {
      _inner <<= to_drop - 1;
    }

    if (to_take > 0) {
      _inner <<= 1;
      _inner |= 1;
    }
  }

  auto next(int& take) -> bool {
    if (_inner == 0) {
      return false;
    } else {
      auto to_take = first();
      _inner &= ~u8(1 << (to_take - 1));
      take = to_take;
      return true;
    }
  }

  auto operator*() const -> u8 { return _inner; }

public:
  u8 _inner = 0;
};

template <int S>
class Move {
public:
  explicit constexpr Move() noexcept = default;
  explicit constexpr Move(u16 data) : _inner(data) {}
  constexpr Move(const std::string& move) {
    auto is_square_start = move[0] >= 'a' and move[0] < 'a' + S;
    if (is_square_start and move.size() == 2) {
      *this = place(Square<S>(move), FLAT);
      return;
    }

    auto parse_direction = [](char c) {
      return Direction(c == '>'   ? EAST
                       : c == '<' ? WEST
                       : c == '+' ? NORTH
                       : c == '-' ? SOUTH
                                  : -1);
    };

    if (is_square_start and move.size() == 3) {
      auto pattern = Spread<S>();
      pattern.push(1, 1);
      *this = spread(Square<S>(move.substr(0, 2)), parse_direction(move[2]),
                     pattern);
      return;
    }

    auto is_wall = move[0] == 'S';
    auto is_cap = move[0] == 'C';
    if (is_wall or is_cap) {
      auto square = Square<S>(is_wall or is_cap ? move.substr(1, 3) : move);
      *this = place(square, is_cap ? CAP : is_wall ? WALL : FLAT);
      return;
    }

    if (std::isdigit(move[0])) {
      auto held = move[0] - '0';
      if (held > S) {
        PANIC("unexpected spread stack size: found `{}`, but board size is "
              "`{}`",
              move[0] - '0', S);
      }
    }
    auto held = move[0] - '0';
    auto square = Square<S>(move.substr(1, 3));
    auto direction = parse_direction(move[3]);
    auto pattern = Spread<S>();
    if (move.size() == 4) {
      pattern.push(held, S);
      pattern.push(0, held);
    } else {
      usize i = 4;
      pattern.push(held, S);
      while (i < move.size() - 1) {
        auto take = move[i++] - '0';
        pattern.push(held - take, held);
        held -= take;
      }
      pattern.push(0, held);
    }

    *this = spread(square, direction, pattern);
  }

  operator int() = delete;

  static constexpr auto place(Square<S> square, StoneType st) -> Move {
    ASSERT(st != NO_STONE_TYPE);
    ASSERT(square.ok());
    return Move(u16(st << 6 | *square));
  }

  static constexpr auto spread(Square<S> square, Direction direction,
                               Spread<S> spread) -> Move {
    ASSERT(square.ok());
    return Move(u16(*spread) << 8 | direction << 6 | *square);
  }

public:
  constexpr auto is_place() const -> bool { return _inner >> 8 == 0; }
  constexpr auto square() const -> Square<S> { return Square<S>(_inner & 63); }
  constexpr auto stone() const -> StoneType {
    ASSERT(is_place());
    return StoneType((_inner >> 6) & 3);
  }

  constexpr auto direction() const -> Direction {
    ASSERT(not is_place());
    return Direction((_inner >> 6) & 3);
  }

  constexpr auto spread_pattern() const -> Spread<S> {
    ASSERT(not is_place());
    return Spread<S>(u8(_inner >> 8) & 0xff);
  }

  auto to_string() const -> std::string {
    auto sq = square();
    ASSERT(sq.ok());
    static const char* s_stone_type[] = { "", "", "S", "C" };
    if (is_place()) {
      auto st = stone();
      ASSERT(st != NO_STONE_TYPE);
      return fmt::format("{}{}", s_stone_type[st], sq);
    } else {
      auto dir = direction();
      auto spread = spread_pattern();
      auto held = 0;
      spread.next(held);

      static const char* s_direction_indicator[] = { "+", ">", "-", "<" };
      auto s = fmt::format("{}{}{}",
                           held == 1 ? std::string() : std::to_string(held), sq,
                           s_direction_indicator[dir]);

      if (spread.size() == 0) {
        return s;
      }

      int to_take;
      std::vector<int> drops = {};
      while (spread.next(to_take)) {
        auto to_drop = held - to_take;
        drops.push_back(to_drop);
        held -= to_drop;
      }

      return fmt::format("{}{}{}", s, fmt::join(drops, ""), held);
    }
  }

  auto operator*() const -> u16 { return _inner; }

private:
  u16 _inner;
};

} // namespace eris

#define X(_S) FMT(eris::Move<_S>, "{}", v.to_string());
BOARD_SIZE_ITER
#undef X
