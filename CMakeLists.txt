cmake_minimum_required(VERSION 3.20.0)
project(eris VERSION 0.1)

find_program(CCACHE_PROGRAM ccache)
if (CCACHE_PROGRAM)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

if(CMAKE_BUILD_TYPE EQUAL "DEBUG")
  message("debug mode")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)

set(ENABLE_TESTS OFF)

if (BUILD_TESTS)
  set(ENABLE_TESTS ON)
endif()

include(FetchContent)

FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt
  GIT_TAG        40626af88bd7df9a5fb80be7b25ac85b122d6c21) # 11.2.0
FetchContent_MakeAvailable(fmt)

file(GLOB_RECURSE ENGINE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/lib/engine/*.cc)
add_library(eris ${ENGINE_SRC})
target_precompile_headers(eris PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/base.hh)
target_include_directories(eris PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(eris PUBLIC fmt)
target_link_options(eris PUBLIC $<$<CONFIG:Debug>:-fsanitize=address>)

target_compile_options(eris PUBLIC
  -march=native
  -ggdb3
  -Wundef
  -Wcast-align
  -Wsign-conversion
  -Wformat=2
  -Wconversion
  -Wshadow

  -Werror=nonnull
  -Werror=address
  -Werror=init-self
  -Werror=uninitialized
  -Werror=return-type
  -Werror=pointer-arith
  -Werror=implicit-fallthrough
  -Werror=missing-include-dirs
  -Werror=string-compare
  -Werror=switch
  -Werror=switch-enum
  -Werror=write-strings

  -Werror=missing-field-initializers

  $<$<CONFIG:Debug>:-fsanitize=address>
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(eris PUBLIC
    -Wlogical-op
    -Werror=invalid-memory-model
    -Werror=maybe-uninitialized
    -Werror=missing-requires
    -Werror=return-local-addr
  )
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(eris PUBLIC
    -Werror=dangling
    -Werror=return-stack-address
  )
endif()

add_executable(tei ${CMAKE_CURRENT_SOURCE_DIR}/lib/tei/main.cc)
target_link_libraries(tei eris)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest
  GIT_TAG        52eb8108c5bdec04579160ae17225d66034bd723) # 1.17.0

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

if (ENABLE_TESTS)
  enable_testing()
  include(GoogleTest)

  file(GLOB_RECURSE TEST_SRC ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cc)
  add_executable(tests ${TEST_SRC})
  target_link_libraries(tests eris GTest::gtest)

  gtest_discover_tests(tests)
endif()
