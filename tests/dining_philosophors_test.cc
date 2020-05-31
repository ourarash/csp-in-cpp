#include "src/lib/csp/dining_philosophors.h"

#include <vector>

#include "gtest/gtest.h"
#include "src/lib/csp/csp.h"
bool g_stop = false;
std::mutex g_mutex;
std::condition_variable g_cv;

inline void setup(int N) {
  std::unique_lock<std::mutex> ul(g_mutex);
  g_stop = false;
  ul.unlock();
  g_cv.notify_all();
  Channel<> test_channel;

  auto unit_under_test = std::thread(college, N, true, std::ref(test_channel));
  auto environment = std::thread([&]() {
    for (int i = 0; i < 1000; i++) {
      auto seated = test_channel.Read();
      // std::cout << "seated: " << seated << std::endl;
      EXPECT_LT(seated, N);
    }
    std::unique_lock<std::mutex> ul(g_mutex);
    g_stop = true;
    ul.unlock();
    g_cv.notify_all();
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
}