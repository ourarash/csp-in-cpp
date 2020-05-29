#include "src/lib/csp/dining_philosophors.h"

#include <vector>

#include "gtest/gtest.h"
#include "src/lib/csp/csp.h"

inline void setup(int N) {
  Channel<> test_channel;

  auto unit_under_test = std::thread(college, N, true, std::ref(test_channel));
  auto environment = std::thread([&]() {
    for (int i = 0; i < 10; i++) {
      auto seated = test_channel.Read();
      std::cout << "seated: " << seated << std::endl;
      EXPECT_LT(seated, N);
    }
    unit_under_test.detach();
  });

  environment.join();
  if (unit_under_test.joinable()) {
    unit_under_test.join();
  }
}

TEST(CSPTest, DiningPhilosophors_5) {
  const int N = 5;
  setup(N);
}

TEST(CSPTest, DiningPhilosophors_3) {
  const int N = 10;
  setup(N);
  // EXPECT_EQ(2,2);
}