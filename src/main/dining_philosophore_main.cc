#include <cassert>
#include <chrono>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

#include "src/lib/csp/csp.h"
#include "src/lib/csp/dining_philosophors.h"
#include "src/lib/utility.h"
bool g_stop = false;
std::mutex g_mutex;
std::condition_variable g_cv;

using namespace csp;

int main() {
  const int N = 5;
  Channel<> test_channel;
  auto unit_under_test = std::thread(college, N, true, std::ref(test_channel));
  auto environment = std::thread([&]() {
    for (int i = 0; i < 10; i++) {
      auto seated = test_channel.Read();
      std::cout << "seated: " << seated << std::endl;
      assert(seated < N);
    }

    std::unique_lock<std::mutex> ul(g_mutex);
    g_stop = true;
    g_cv.notify_all();
  });

  environment.join();
  if (unit_under_test.joinable()) {
    unit_under_test.join();
  }
}