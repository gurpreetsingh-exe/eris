#pragma once

#include "square.hh"
#include "types.hh"

namespace eris {

// 000000 000 000000 0
//        ^^^ ^^^^^^ ^ place or move bits
//         |    | square bits
//         | stone bits

enum MoveType : u8 {
  PLACE = 0,
  MOVE,
};

template <int S>
class Move {
public:
  explicit constexpr Move() noexcept = default;
  explicit constexpr Move(MoveType type, Stone stone, Square<S> square)
      : _inner((stone << 7) | (*square << 1) | type) {
    ASSERT(stone != NO_STONE);
    ASSERT(square.ok());
  }

  explicit constexpr Move(MoveType type, Stone stone, Square<S> square,
                          Direction direction)
      : _inner((direction << 10) | (stone << 7) | (*square << 1) | type) {
    ASSERT(square.ok());
  }

  operator int() = delete;

public:
  constexpr auto is_place() const -> bool { return type() == PLACE; }
  constexpr auto type() const -> MoveType { return MoveType(_inner & 1); }
  constexpr auto square() const -> Square<S> {
    return Square<S>((_inner >> 1) & 0b111111);
  }
  constexpr auto stone() const -> Stone { return Stone((_inner >> 7) & 0b111); }

  constexpr auto direction() const -> Direction {
    ASSERT(type() == MOVE);
    return Direction((_inner >> 10) & 0b11);
  }

  auto to_string() const -> std::string {
    auto sq = square();
    auto st = stone_type(stone());
    ASSERT(sq.ok());
    ASSERT(st != NO_STONE_TYPE);
    static const char* s_stone_type[] = { "", "", "S", "C" };
    if (is_place()) {
      return fmt::format("{}{}", s_stone_type[st], sq);
    } else {
      auto dir = direction();
      static const char* s_direction_indicator[] = { "+", ">", "-", "<" };
      return fmt::format("{}{}{}", sq, s_direction_indicator[dir],
                         s_stone_type[st]);
    }
  }

  auto operator*() const -> u16 { return _inner; }

private:
  u16 _inner;
};

} // namespace eris

FMT(eris::Move<3>, "{}", v.to_string());
