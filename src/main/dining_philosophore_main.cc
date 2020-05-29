#include <cassert>
#include <chrono>
#include <iostream>
#include <random>
#include <sstream>
#include <thread>
#include <vector>

#include "src/lib/csp/csp.h"
#include "src/lib/utility.h"
/** Thread safe cout class
 * Exemple of use:
 *    PrintThread{} << "Hello world!" << std::endl;
 */
class PrintThread : public std::ostringstream {
 public:
  PrintThread() = default;

  ~PrintThread() {
    std::lock_guard<std::mutex> guard(_mutexPrint);
    auto end = std::chrono::system_clock::now();
    std::time_t end_time = std::chrono::system_clock::to_time_t(end);

    std::cout << end_time << ". " << this->str();
  }

 private:
  static std::mutex _mutexPrint;
};

std::mutex PrintThread::_mutexPrint{};
auto gen = std::bind(std::uniform_int_distribution<>(0, 10),
                     std::default_random_engine());

// A demo for unique_lock: similar to lock_guard, but it can
// be lock and unlock multiple times.

// Run this using one of the following methods:
//  1. With bazel: bazel run src/main/mutex:{THIS_FILE_NAME_WITHOUT_EXTENSION}
//  2. With plain g++: g++ -std=c++17 -lpthread
//  src/main/mutex/{THIS_FILE_NAME}.cc  -I ./

void fork(Channel &left, Channel &right, int delay_ms, int number) {
  while (true) {
    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    Choice c{!left.IsIdle(), !right.IsIdle()};
    int select = c.Select();

    switch (select) {
      case 0:
        left.Read();
        left.Read();
        break;

      case 1:
        right.Read();
        right.Read();
        break;
    }
  }
}

void phil(Channel &left, Channel &right, Channel &up, Channel &down,
          int delay_ms, int number) {
  while (true) {
    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    down.Write();
    PrintThread{} << number << ". Thinking!" << std::endl;

    ForkJoin pickup_forks{[&]() { left.Write(); }, [&]() { right.Write(); }};

    PrintThread{} << number << ". Eating!" << std::endl;

    ForkJoin putdown_forks{[&]() { left.Write(); }, [&]() { right.Write(); }};
    up.Write();
  }
}

void butler(std::vector<Channel> &up, std::vector<Channel> &down, int delay_ms,
            int N) {
  int number_of_seated = 0;
  while (true) {
    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));

    if (number_of_seated < N - 1) {
      Choice c(down);
      int select = c.Select();
      if (select != -1) {
        down[select].Read();
        number_of_seated++;
        PrintThread{} << "number_of_seated: " << number_of_seated << std::endl;
      }
    } else {
      Choice c(up);
      int select = c.Select();
      if (select != -1) {
        up[select].Read();
        number_of_seated--;
        PrintThread{} << "number_of_seated: " << number_of_seated << std::endl;
      }
    }
  }
}

int main() {
  const int N = 5;
  PrintThread{} << "Start!" << std::endl;
  std::vector<Channel> left(N);
  std::vector<Channel> right(N);
  std::vector<Channel> up(N);
  std::vector<Channel> down(N);

  std::vector<std::function<void()>> processes;

  for (int i = 0; i < N; i++) {
    processes.push_back(std::bind(phil, std::ref(left[i]), std::ref(right[i]),
                                  std::ref(up[i]), std::ref(down[i]), 1000, i));
  }

  for (int i = 0; i < N; i++) {
    processes.push_back(
        std::bind(fork, std::ref(right[i]), std::ref(left[(i + 1) % N]), 0, i));
  }

  processes.push_back(std::bind(butler, std::ref(up), std::ref(down), 0, N));

  ForkJoin f(processes);
}