#pragma once

#include "types.hh"

namespace eris {

template <int Size>
class Square {
public:
  constexpr Square(int idx) {
    static_assert(Size >= 3 and Size <= 8);
    ASSERT(idx >= 0 and idx < Size * Size, "{}", idx);
    _idx = u8(idx);
  }

  constexpr Square() : _idx(Size * Size) {}
  constexpr Square(int rank, int file) : Square(file + rank * Size) {
    ASSERT(rank < Size and file < Size, "{} - {}", rank, file);
  }
  constexpr Square(const std::string& str, bool black = false)
      : Square(black ? '8' - str[1] : str[1] - '1', str[0] - 'a') {}
  constexpr auto as_board() const -> u64 { return 1ULL << _idx; }
  constexpr auto operator*() const -> u8 { return _idx; }
  constexpr auto rank() const -> int { return _idx / Size; }
  constexpr auto file() const -> int { return _idx % Size; }

  constexpr auto operator==(Square<Size> other) const -> bool {
    return _idx == other._idx;
  }
  constexpr auto ok() const -> bool {
    return *this != none() and ok(rank(), file());
  }

  static constexpr auto none() -> Square<Size> { return Square(); }
  static constexpr auto ok(int rank, int file) -> bool {
    return is_in_range(rank) and is_in_range(file);
  }

  static constexpr auto is_in_range(int n) -> bool {
    return n >= 0 and n < Size;
  }

  constexpr auto move_in(Direction direction, int n = 1) const -> Square<Size> {
    constexpr int offset[] = { Size, 1, -Size, -1 };
    ASSERT(direction == NORTH   ? rank() < Size - 1
           : direction == SOUTH ? rank() > 0
           : direction == EAST  ? file() < Size - 1
                                : file() > 0);
    return Square<Size>(_idx + u8(offset[direction] * n));
  }

  constexpr auto operator+=(Direction direction) -> void {
    constexpr int offset[] = { Size, 1, -Size, -1 };
    ASSERT(direction == NORTH   ? rank() < Size - 1
           : direction == SOUTH ? rank() > 0
           : direction == EAST  ? file() < Size - 1
                                : file() > 0);
    _idx += u8(offset[direction]);
  }

  friend constexpr auto operator+(Square<Size> square, Direction direction)
      -> Square<Size> {
    square += direction;
    return square;
  }

  auto to_string() const -> std::string {
    return std::format("{}{}", char(file() + 'a'), char(rank() + '1'));
  }

private:
  u8 _idx;
};

} // namespace eris

#define X(_S) FMT(eris::Square<_S>, "{}", v.to_string());
BOARD_SIZE_ITER
#undef X
