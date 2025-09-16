#pragma once

#include "bitboard.hh"
#include "move.hh"
#include "stack.hh"
#include "tables.hh"
#include "types.hh"

namespace eris {

static constexpr u8 starting_stones[] = { 10, 15, 21, 30, 40, 50 };
static constexpr u8 starting_caps[] = { 0, 0, 1, 1, 2, 2 };

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

  auto stack() const -> const Stack* { return _stack; }
  auto stack(Square<Size> sq) const -> Stack { return _stack[*sq]; }
  auto stack(usize idx) const -> Stack { return _stack[idx]; }

  static constexpr auto size() -> usize { return Size; }
  auto movecount() const -> int { return _movecount; }
  auto first_move() const -> bool { return _movecount == 0 or _movecount == 1; }

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

  template <Color C>
  auto road() -> bool {
    auto grid = stones<FLAT, C>() | stones<CAP, C>();
    constexpr auto side = [](auto fn) {
      auto m = Bitboard();
      for (int i = 0; i < Size * Size; ++i) {
        auto square = Square<Size>(i);
        if (fn(square)) {
          m |= square;
        }
      }
      return m;
    };

    constexpr auto BOTTOM = side([](auto s) { return s.rank() == 0; });
    constexpr auto TOP = side([](auto s) { return s.rank() == Size - 1; });
    constexpr auto LEFT = side([](auto s) { return s.file() == 0; });
    constexpr auto RIGHT = side([](auto s) { return s.file() == Size - 1; });

    auto flood = [&](Bitboard start, Bitboard goal) -> bool {
      auto reach = start & grid;
      auto old = Bitboard();
      while (reach != old) {
        old = reach;
        auto up = reach << Size;
        auto down = reach >> Size;
        auto left = (reach >> 1) & ~RIGHT;
        auto right = (reach << 1) & ~LEFT;
        reach |= (up | down | left | right) & grid;
      }
      return (reach & goal) != 0;
    };

    return flood(TOP, BOTTOM) or flood(LEFT, RIGHT);
  }

  auto road() -> bool { return road<WHITE>() or road<BLACK>(); }

  auto put_stone(Stone st, Square<Size> sq) -> void {
    auto idx = *sq;
    if (not _top[idx]) {
      _replace_stone_at_top(st, sq);
      return;
    }

    auto sty = stone_type(_top[idx]);
    ASSERT(sty == FLAT, "`{}` found at \"{}\"", sty, sq);
    _stack[idx].push(stone_color(_top[idx]));
    _replace_stone_at_top(st, sq);
  }

  auto take_stone(Square<Size> sq) -> Stone {
    auto idx = *sq;
    if (not _top[idx]) {
      return NO_STONE;
    }

    auto st = _top[idx];
    if (not _stack[idx].height()) {
      _replace_stone_at_top(NO_STONE, sq);
      return st;
    }

    auto new_top_stone = mk_stone(FLAT, _stack[idx].pop());
    _replace_stone_at_top(new_top_stone, sq);
    return st;
  }

  auto make_move(Move<Size> move) -> void {
    auto square = move.square();
    ASSERT(square.ok());
    const auto us = first_move() ? ~_turn : _turn;
    if (move.is_place()) {
      auto stone = mk_stone(move.stone(), us);
      ASSERT(_top[*square] == NO_STONE);
      put_stone(stone, square);
      if (move.stone() == CAP) {
        ASSERT(_ncaps[us] > 0);
        _ncaps[us] -= 1;
      } else {
        ASSERT(_nstones[us] > 0);
        _nstones[us] -= 1;
      }
    } else {
      auto origin = square;
      auto direction = move.direction();
      auto spread = move.spread_pattern();

      auto move_to_stack = [&](Square<Size> sq) {
        if (auto top = _top[*sq]) {
          auto ty = stone_type(top);
          // TODO: better assert?
          // ASSERT(ty != CAP and ty != WALL);
          _replace_stone_at_top(NO_STONE, *sq);
          _stack[*sq].push(stone_color(top));
        }
      };

      auto held = 0;
      spread.next(held);

      int to_take;
      auto held_stack = _stack[*square].take(held - 1);

      while (spread.next(to_take)) {
        auto next = find_in_direction(square, direction);
        ASSERT(next != square, "dropping on same square at `{}`", square);
        auto to_drop = held - to_take;
        auto t = held_stack.take_back(to_drop);
        move_to_stack(next);
        _stack[*next].push(t);
        _replace_stone_at_top(mk_stone(FLAT, _stack[*next].pop()), *next);

        square = next;
        held -= to_drop;
      }

      auto taken = take_stone(origin);
      square = find_in_direction(square, direction);
      move_to_stack(square);
      _stack[*square].push(held_stack);
      put_stone(taken, square);
    }

    _turn = ~_turn;
    _movecount += 1;
  }

  auto unmake_move(Move<Size> move) -> void {
    auto square = move.square();
    ASSERT(square.ok());
    if (move.is_place()) {
      auto stone = mk_stone(
          move.stone(), _movecount == 1 or _movecount == 2 ? _turn : ~_turn);
      auto taken = take_stone(square);
      ASSERT(taken == stone, "`{}` != `{}`", taken, stone);
    } else {
      auto direction = move.direction();
      auto spread = move.spread_pattern();
      auto taken = take_stone(square + direction);
      auto end_square = square.move_in(direction, spread.size());
      fmt::println("{}", end_square);
      put_stone(taken, square);
    }

    _turn = ~_turn;
    _movecount -= 1;
  }

  template <Color C, StoneType St>
  constexpr auto
  generate_spread_moves(Square<Size> origin, Square<Size> square,
                        Direction direction, int carried, Spread<Size> partial,
                        ArrayVec<Spread<Size>, 255>& spread) const -> void {
    auto to = find_in_direction(square, direction);
    if (to == square) {
      return;
    }

    auto to_stone = _top[*to];
    if constexpr (St == CAP) {
      if (to_stone and stone_type(to_stone) == CAP) {
        return;
      }
    } else {
      if (to_stone and stone_type(to_stone) != FLAT) {
        return;
      }
    }

    generate_spread_moves_impl<C, St>(origin, square, direction, carried,
                                      partial, spread);
  }

  template <Color C, StoneType St>
  constexpr auto
  generate_spread_moves_impl(Square<Size> origin, Square<Size> square,
                             Direction direction, int carried,
                             Spread<Size> partial,
                             ArrayVec<Spread<Size>, 255>& spread) const
      -> void {
    auto held = square == origin ? Size : carried;
    auto max_stones_to_take = square == origin ? std::min(carried, Size)
                                               : std::min(carried - 1, Size);

    auto generate = [&] {
      for (int to_take = 1; to_take <= max_stones_to_take; ++to_take) {
        auto new_spread = partial;
        new_spread.push(to_take, held);
        generate_spread_moves<C, St>(origin, square + direction, direction,
                                     to_take, new_spread, spread);
        new_spread.push(0, to_take);
        spread.push_back(new_spread);
      }
    };

    if constexpr (St == CAP) {
      const auto neighbour = _top[*(square + direction)];
      if (neighbour and stone_type(neighbour) == WALL and
          max_stones_to_take > 0) {
        auto new_spread = partial;
        new_spread.push(1, held);
        spread.push_back(new_spread);
      } else {
        generate();
      }

      return;
    }

    generate();
  }

  template <Color C>
  constexpr auto generate_moves(MoveList<Size>& moves) const -> void {
    for (const auto square : iter<Size>(~stones())) {
      if (_nstones[C] > 0) {
        moves.push_back(Move<Size>::place(square, FLAT));
        moves.push_back(Move<Size>::place(square, WALL));
      }
      if constexpr (Size >= 5) {
        if (_ncaps[C] > 0) {
          moves.push_back(Move<Size>::place(square, CAP));
        }
      }
    }

    for (const auto square : iter<Size>(stones<C>())) {
      auto stone = _top[*square];
      ASSERT(stone != NO_STONE);
      auto available_squares = ~stones() | stones<FLAT>();
      auto height = _stack[*square].height() + 1;
      for (const auto dir :
           IterateBits(orthogonally_adjacent_squares<Size>(square))) {
        auto spread = ArrayVec<Spread<Size>, 255>();
        auto direction = Direction(dir);
        if (stone_type(stone) == CAP) {
          generate_spread_moves<C, CAP>(square, square, direction, height, {},
                                        spread);
        } else {
          generate_spread_moves<C, FLAT>(square, square, direction, height, {},
                                         spread);
        }

        for (auto mv : spread) {
          moves.push_back(Move<Size>::spread(square, direction, mv));
        }
      }
    }
  }

  constexpr auto generate_moves(MoveList<Size>& moves) const -> void {
    if (first_move()) {
      const auto empty = ~stones();
      for (auto square : iter<Size>(empty)) {
        moves.push_back(Move<Size>::place(square, FLAT));
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
      auto height = _stack[i].height();
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
      if (auto height = _stack[i].height()) {
        fmt::print(" {} {} {}", chars.hbar, Square<Size>(i), chars.hbar);
        for (int j = height - 1; j >= 0; --j) {
          auto b = Bitboard(*_stack[i]);
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

    _nstones[0] = starting_stones[Size - 3];
    _nstones[1] = starting_stones[Size - 3];

    _ncaps[0] = starting_caps[Size - 3];
    _ncaps[1] = starting_caps[Size - 3];

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
  Stack _stack[usize(Size * Size)] = {};

  u8 _nstones[COLOR_NB] = { starting_stones[Size - 3],
                            starting_stones[Size - 3] };
  u8 _ncaps[COLOR_NB] = { starting_caps[Size - 3], starting_caps[Size - 3] };

  Color _turn = WHITE;
  int _movecount = 0;
};

} // namespace eris

FMT(eris::Stack, "{}", v.to_string());
