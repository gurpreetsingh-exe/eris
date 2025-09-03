#pragma once

#include <bitset>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <execinfo.h>
#include <experimental/source_location>
#include <filesystem>
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <format>
#include <ranges>
#include <span>
#include <vector>

#define STR2(s) #s
#define STR(v) STR2(v)

#define UNREACHABLE() __builtin_unreachable()

#define LOC __FILE__ ":" STR(__LINE__)

#define PRAGMA(S) _Pragma(#S)
#define PUSH_IGNORE_WARNING(W)                                                 \
  PRAGMA(GCC diagnostic push) PRAGMA(GCC diagnostic ignored W)
#define POP_IGNORE_WARNING() PRAGMA(GCC diagnostic pop)

#define ASAN_BREAKPOINT(c)                                                     \
  if (not c) {                                                                 \
    int a##__COUNTER__ = *(int*)0;                                             \
  }

#define DISALLOW_COPY_AND_ASSIGN(T)                                            \
  T(T&&) = delete;                                                             \
  T(const T&) = delete;                                                        \
  T& operator=(const T&) = delete;                                             \
  T& operator=(T&&) = delete;

#define PANIC(...)                                                             \
  ::eris::__panic(::eris::Location::current(), "\n    " __VA_ARGS__)

#define TODO()                                                                 \
  PANIC("TODO: not implemented in `{}`", __func__);                            \
  UNREACHABLE()

// #if NDEBUG
// #  define ASSERT(cond, ...)
// #else
#define ASSERT(cond, ...)                                                      \
  (((cond) ? static_cast<void>(0)                                              \
           : PANIC("assertion `" #cond "` failed\n    " __VA_ARGS__)))
// #endif

#define FMT(type, fmt_str, ...)                                                \
  template <>                                                                  \
  struct fmt::formatter<type> {                                                \
    constexpr auto parse(format_parse_context& ctx)                            \
        -> format_parse_context::iterator {                                    \
      return ctx.begin();                                                      \
    }                                                                          \
    auto format(type v, format_context& ctx) const                             \
        -> format_context::iterator {                                          \
      return fmt::format_to(ctx.out(), fmt_str, __VA_ARGS__);                  \
    }                                                                          \
  }

#define TIME(name, ...)                                                        \
  do {                                                                         \
    auto timer##__COUNTER__ = ::eris::Timer(name);                             \
    __VA_ARGS__                                                                \
  } while (0)

namespace eris {

namespace rng = std::ranges;
namespace chr = std::chrono;

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using u128 = __uint128_t;
using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using usize = size_t;
using f32 = float;
using f64 = double;
using Location = std::experimental::source_location;

class Timer {
public:
  Timer(std::string name)
      : _name(name), _start(chr::high_resolution_clock::now()) {}
  ~Timer() {
    auto end = chr::high_resolution_clock::now();
    auto duration_ns = chr::duration_cast<chr::nanoseconds>(end - _start);
    auto duration_µs = duration_ns / 1000.0f;
    auto duration_ms = duration_ns / 1000000.0f;
    fmt::println("{}: {} ns, {:.2f} µs, {:.3f} ms", _name, duration_ns.count(),
                 duration_µs.count(), duration_ms.count());
  }

private:
  std::string _name;
  chr::time_point<chr::high_resolution_clock> _start;
};

auto print_backtrace(int /* skip_value */ = 0) -> void;

template <typename... T>
auto __panic(Location loc, fmt::format_string<T...> fmt, T&&... args) {
  fmt::print(stderr, "panic at {}:{}", loc.file_name(), loc.line());
  fmt::println(stderr, fmt, args...);
  fmt::println(stderr, "BACKTRACE:");
  print_backtrace(2);
  exit(1);
}

using Location = std::experimental::source_location;

constexpr bool backtrace_addr2line = true;
constexpr std::string_view backtrace_addr2line_options = "-p -C -f";
constexpr bool backtrace_llvm_symbolizer = false;
constexpr std::string_view backtrace_llvm_symbolizer_options =
    "-s -p -C -i --color --output-style=GNU";

inline auto print_backtrace(int skip_value) -> void {
  constexpr usize size = 128;
  void* trace[size];
  int n = backtrace(trace, size);
  char** strings = backtrace_symbols(&trace[0], size);

  if constexpr (backtrace_addr2line) {
    for (int i = skip_value; i < n; ++i) {
      trace[i] = (void*)((std::uintptr_t)trace[i] - 1);
    }

    std::span<void*> trace_view { &trace[skip_value], &trace[n] };

    auto command =
        fmt::format("addr2line {} -e {} {}", backtrace_addr2line_options,
                    std::filesystem::canonical("/proc/self/exe").native(),
                    fmt::join(trace_view, " "));
    std::system(command.data());
  }

  std::free(strings);
}

} // namespace eris
