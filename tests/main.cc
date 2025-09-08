#include <gtest/gtest.h>
#include <memory>

#include "engine.hh"

class Environment : public ::testing::Environment {
public:
  ~Environment() override {}

  auto SetUp() -> void override { eris::init_engine(); }
  auto TearDown() -> void override {}
};

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::testing::AddGlobalTestEnvironment(new Environment);
  return RUN_ALL_TESTS();
}
