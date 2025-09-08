#pragma once

#define FANCY

#include <sstream>

namespace eris {

constexpr struct {
  const char* hbar;
  const char* vbar;
  const char* topright;
  const char* topleft;
  const char* bottomright;
  const char* bottomleft;
  const char* zerobit;
  const char* onebit;
  const char* t;
  const char* t0;
} chars = {
#if defined(FANCY)
  .hbar = "│",
  .vbar = "─",
  .topright = "╮",
  .topleft = "╭",
  .bottomright = "╯",
  .bottomleft = "╰",
  .zerobit = "·",
  .onebit = "◘",
  .t = "┬",
  .t0 = "┴",
#else
  .hbar = " ",
  .vbar = " ",
  .topright = " ",
  .topleft = " ",
  .bottomright = " ",
  .bottomleft = " ",
  .zerobit = "0",
  .onebit = "1",
  .t = " ",
  .t0 = " ",
#endif
};

template <typename Out>
inline auto split(const std::string& s, char delim, Out result) -> void {
  std::istringstream iss(s);
  std::string item;
  while (std::getline(iss, item, delim)) { *result++ = item; }
}

inline auto split(const std::string& s, char delim)
    -> std::vector<std::string> {
  std::vector<std::string> elems;
  split(s, delim, std::back_inserter(elems));
  return elems;
}

} // namespace eris
