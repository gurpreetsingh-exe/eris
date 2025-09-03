#pragma once

namespace eris {

constexpr u64 EMPTY = 0ULL;

/// STONE TYPES
enum StoneType : u8 {
  NO_STONE_TYPE = 0,
  FLAT,
  WALL,
  CAP,
  STONE_TYPE_NB = 4,
};

enum Stone : u8 {
  NO_STONE = 0,
  B_FLAT = FLAT,
  B_WALL,
  B_CAP,
  W_FLAT = FLAT + 4,
  W_WALL,
  W_CAP,
  STONE_NB = 8,
};

enum Color : u8 {
  BLACK = 0,
  WHITE,
  COLOR_NB = 2,
};

enum Direction : u8 {
  NORTH,
  EAST,
  SOUTH,
  WEST,
};

constexpr auto operator~(Color c) -> Color { return Color(c ^ WHITE); }

constexpr std::string_view stone_to_unicode[] = {
  " ", "□", "◇", "○", " ", "■", "◆", "●",
};

constexpr std::string_view stone_type_to_unicode[] = { "NONE", "FLAT", "WALL",
                                                       "CAP" };

constexpr auto mk_stone(StoneType st, Color c) -> Stone {
  return Stone((c << 2) + st);
}

template <StoneType St, Color C>
constexpr auto mk_stone() -> Stone {
  return Stone((C << 2) + St);
}

constexpr auto stone_type(Stone st) -> StoneType {
  ASSERT(st != NO_STONE);
  return StoneType(st & 3);
}

constexpr auto stone_color(Stone st) -> Color {
  ASSERT(st != NO_STONE);
  return Color(st >> 2);
}

} // namespace eris

FMT(eris::Stone, "{}", eris::stone_to_unicode[v]);
FMT(eris::StoneType, "{}", eris::stone_type_to_unicode[v]);
FMT(eris::Color, "{}", v ? "WHITE" : "BLACK");
