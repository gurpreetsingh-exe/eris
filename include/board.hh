#pragma once

#include "bitboard.hh"
#include "move.hh"
#include "tables.hh"
#include "types.hh"

namespace eris {

template <int Size>
class Board {
public:
  constexpr Board() noexcept = default;

  static auto from(const std::string& tps) -> Board {
    auto board = Board<Size>();
    board.tps(tps);
    return board;
  }

public:
  auto top() const -> const Stone* { return _top; }
  auto top(Square<Size> sq) const -> Stone { return _top[*sq]; }
  auto top(usize idx) const -> Stone { return _top[idx]; }

  auto stack() const -> const u64* { return _stack; }
  auto stack(Square<Size> sq) const -> u64 { return _stack[*sq]; }
  auto stack(usize idx) const -> u64 { return _stack[idx]; }

  auto stack_height() const -> const u8* { return _stack_height; }
  auto stack_height(Square<Size> sq) const -> u8 { return _stack_height[*sq]; }
  auto stack_height(usize idx) const -> u8 { return _stack_height[idx]; }

  static constexpr auto size() -> usize { return Size; }
  auto movecount() const -> int { return _movecount; }

  template <StoneType St, Color C>
  constexpr auto stones() const -> Bitboard {
    return stones<St>() & stones<C>();
  }

  template <Stone S>
  constexpr auto stones() const -> Bitboard {
    constexpr auto St = stone_type(S);
    constexpr auto C = stone_color(S);
    return stones<St, C>();
  }

  auto stones(Stone s) const -> Bitboard {
    return stones(stone_type(s), stone_color(s));
  }

  auto stones(StoneType st, Color c) const -> Bitboard {
    return _stones[st] & _colors[c];
  }

  template <StoneType St>
  constexpr auto stones() const -> Bitboard {
    return stones(St);
  }

  auto stones(StoneType st) const -> Bitboard { return _stones[st]; }

  constexpr auto stones() const -> Bitboard { return stones<BLACK, WHITE>(); }

  template <Color... C>
  constexpr auto stones() const -> Bitboard {
    return stones(C...);
  }

  template <typename... Color>
  constexpr auto stones(Color... c) const -> Bitboard {
    return (... | _colors[c]);
  }

  auto put_stone(Stone st, Square<Size> sq) -> void {
    auto idx = *sq;
    if (not _top[idx]) {
      _replace_stone_at_top(st, sq);
      return;
    }

    auto sty = stone_type(_top[idx]);
    ASSERT(sty == FLAT, "`{}` found at \"{}\"", sty, sq);
    _stack[idx] <<= 1;
    _stack[idx] |= stone_color(_top[idx]);
    _stack_height[idx] += 1;
    _replace_stone_at_top(st, sq);
  }

  auto take_stone(Square<Size> sq) -> Stone {
    auto idx = *sq;
    if (not _top[idx]) {
      return NO_STONE;
    }

    auto st = _top[idx];
    if (not _stack_height[idx]) {
      _replace_stone_at_top(NO_STONE, sq);
      return st;
    }

    auto new_top_stone = mk_stone(FLAT, Color(_stack[idx] & 1));

    _stack[idx] >>= 1;
    _stack_height[idx] -= 1;
    _replace_stone_at_top(new_top_stone, sq);
    return st;
  }

  auto make_move(Move<Size> move) -> void {
    auto square = move.square();
    ASSERT(square.ok());
    if (move.type() == PLACE) {
      auto stone = move.stone();
      ASSERT(_top[*square] == NO_STONE);
      put_stone(stone, square);
    } else {
      auto direction = move.direction();
      auto taken = take_stone(square);
      put_stone(taken, square + direction);
    }

    _turn = ~_turn;
    _movecount += 1;
  }

  auto unmake_move(Move<Size> move) -> void {
    auto square = move.square();
    ASSERT(square.ok());
    if (move.type() == PLACE) {
      auto stone = move.stone();
      auto taken = take_stone(square);
      ASSERT(taken == stone, "`{}` != `{}`", taken, stone);
    } else {
      auto direction = move.direction();
      auto taken = take_stone(square + direction);
      put_stone(taken, square);
    }

    _turn = ~_turn;
    _movecount -= 1;
  }

  template <Color C>
  constexpr auto generate_moves(std::vector<Move<Size>>& moves) const -> void {
    for (const auto square : iter<Size>(~stones())) {
      moves.push_back(Move(PLACE, mk_stone<FLAT, C>(), square));
      moves.push_back(Move(PLACE, mk_stone<WALL, C>(), square));
      if constexpr (Size >= 5) {
        moves.push_back(Move(PLACE, mk_stone<CAP, C>(), square));
      }
    }

    for (const auto square : iter<Size>(stones<C>())) {
      auto stone = _top[*square];
      auto available_squares = ~stones() | stones<FLAT>();
      for (const auto dir :
           IterateBits(orthogonally_adjacent_squares<Size>(square))) {
        auto direction = Direction(dir);
        auto to = square + direction;
        auto to_stone = _top[*to];
        if (to_stone == NO_STONE or stone_type(to_stone) == FLAT) {
          moves.push_back(Move(MOVE, stone, square, direction));
        }
      }
    }
  }

  constexpr auto generate_moves(std::vector<Move<Size>>& moves) const -> void {
    if (_movecount == 0 or _movecount == 1) {
      const auto empty = ~stones();
      for (auto square : iter<Size>(empty)) {
        auto stone = mk_stone(FLAT, _turn);
        moves.push_back(Move(PLACE, stone, square));
      }
      return;
    }

    _turn == WHITE ? generate_moves<WHITE>(moves)
                   : generate_moves<BLACK>(moves);
  }

  auto print() const -> void {
    fmt::print("    {}", chars.topleft);
    for (int i = 0; i < Size * 2 + 1; ++i) { fmt::print(chars.vbar); }
    fmt::println(chars.topright);
    for (int j = Size - 1; j >= 0; --j) {
      fmt::print("  {} {} ", j + 1, chars.hbar);
      for (int i = 0; i < Size; ++i) {
        int idx = i + j * Size;
        const auto stone = _top[idx];
        fmt::print("{} ", stone_to_unicode[stone]);
      }
      fmt::print(chars.hbar);
      if (j == Size - 1) {
        fmt::println(" Turn : {}", _turn);
      } else if (j == Size - 2) {
        fmt::println(" Move : {}", _movecount + 1);
      } else {
        fmt::print("\n");
      }
    }
    fmt::print("    {}", chars.bottomleft);
    for (int i = 0; i < Size * 2 + 1; ++i) { fmt::print(chars.vbar); }
    fmt::println(chars.bottomright);
    fmt::print("     ");
    for (int i = 'a'; i < 'a' + Size; ++i) { fmt::print(" {}", char(i)); }
    fmt::print("\n\n");

    int max_height = 0;
    for (int i = 0; i < Size * Size; ++i) {
      auto height = _stack_height[i];
      max_height = height > max_height ? height : max_height;
    }

    if (not max_height) {
      return;
    }

    fmt::print(" {}", chars.topleft);
    for (int i = 0; i < 4; ++i) { fmt::print("{}", chars.vbar); }
    fmt::print("{}", chars.t);
    for (int i = 0; i < max_height * 2 + 1; ++i) {
      fmt::print("{}", chars.vbar);
    }
    fmt::print("{}\n", chars.topright);

    for (int i = 0; i < Size * Size; ++i) {
      if (auto height = _stack_height[i]) {
        fmt::print(" {} {} {}", chars.hbar, Square<Size>(i), chars.hbar);
        for (int j = height - 1; j >= 0; --j) {
          auto b = Bitboard(_stack[i]);
          fmt::print(" {}", b.get(j) ? chars.onebit : chars.zerobit);
        }
        for (int j = 0; j < max_height - height; ++j) { fmt::print("  "); }
        fmt::print(" {}", chars.hbar);
        fmt::print("\n");
      }
    }

    fmt::print(" {}", chars.bottomleft);
    for (int i = 0; i < 4; ++i) { fmt::print("{}", chars.vbar); }
    fmt::print("{}", chars.t0);
    for (int i = 0; i < max_height * 2 + 1; ++i) {
      fmt::print("{}", chars.vbar);
    }
    fmt::print("{}\n", chars.bottomright);
  }

  auto clear() -> void {
    std::memset(_colors, 0, sizeof(_colors));
    std::memset(_stones, 0, sizeof(_stones));
    std::memset(_top, 0, sizeof(_top));
    std::memset(_stack, 0, sizeof(_stack));
    std::memset(_stack_height, 0, sizeof(_stack_height));

    _nstones[0] = 0;
    _nstones[1] = 0;

    _ncaps[0] = 0;
    _ncaps[1] = 0;

    _turn = WHITE;
    _movecount = 0;
  }

  auto tps(const std::string& tps) -> void {
    clear();

    auto start_square = Square<Size>("a" + std::to_string(Size));
    auto rank = start_square.rank();
    auto ss = split(tps, ' ');
    const auto rows = split(ss[0], '/');
    ASSERT(rows.size() == Size);
    for (const auto& row : rows) {
      const auto cols = split(row, ',');
      auto file = start_square.file();
      for (const auto& col : cols) {
        if (col[0] == 'x') {
          if (col.size() > 1 and isdigit(col[1])) {
            file += col[1] - '0';
          } else {
            file += 1;
          }
        } else {
          usize i = 0;
          char token = col[i];
          while ((token = col[i]) and token == '1' or token == '2') {
            auto next_token = i < col.size() - 1 ? col[i + 1] : '\0';
            auto square = Square<Size>(rank, file);
            if (token == '1') {
              if (next_token == 'S') {
                put_stone(W_WALL, square);
              } else if (next_token == 'C') {
                put_stone(W_CAP, square);
              } else {
                put_stone(W_FLAT, square);
              }
            } else if (token == '2') {
              if (next_token == 'S') {
                put_stone(B_WALL, square);
              } else if (next_token == 'C') {
                put_stone(B_CAP, square);
              } else {
                put_stone(B_FLAT, square);
              }
            }
            i += 1;
          }
          file += 1;
        }
      }
      rank -= 1;
    }

    ASSERT(ss[1].size() == 1);
    auto turn = ss[1][0];
    ASSERT(turn == '1' or turn == '2');

    _turn = Color('2' - turn);
    _movecount = std::stoi(ss[2]) - 1;
  }

  auto turn() const -> Color { return _turn; }

private:
  auto _replace_stone_at_top(Stone st, Square<Size> sq) -> void {
    if (auto tmp_st = _top[*sq]) {
      _colors[tmp_st >> 2].template pop<Size>(sq);
      _stones[tmp_st & 3].template pop<Size>(sq);
    }

    if (st) {
      _colors[st >> 2] |= sq;
      _stones[st & 3] |= sq;
    } else {
      _colors[0].template pop<Size>(sq);
      _colors[1].template pop<Size>(sq);
    }

    _top[*sq] = st;
  }

private:
  static_assert(Size >= 3 and Size <= 8);

  Bitboard _colors[COLOR_NB] = {};
  Bitboard _stones[STONE_TYPE_NB] = {};

  Stone _top[usize(Size * Size)] = {};
  u64 _stack[usize(Size * Size)] = {};
  u8 _stack_height[usize(Size * Size)] = {};

  u8 _nstones[COLOR_NB] = { 21, 21 };
  u8 _ncaps[COLOR_NB] = { 1, 1 };

  Color _turn = WHITE;
  int _movecount = 0;
};

} // namespace eris
