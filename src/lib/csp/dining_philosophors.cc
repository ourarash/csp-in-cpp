#ifndef A26A8D6B_086E_45B3_8258_A6B837E30ADF
#define A26A8D6B_086E_45B3_8258_A6B837E30ADF

#include <cassert>
#include <chrono>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

#include "src/lib/csp/csp.h"
// #include "../utility.h"
extern bool g_stop;
extern std::mutex g_mutex;
extern std::condition_variable g_cv;
using namespace csp;
// std::mutex PrintThread::_mutexPrint{};
auto gen = std::bind(std::uniform_int_distribution<>(0, 10),
                     std::default_random_engine());

void fork(Channel<> &left, Channel<> &right, int delay_ms, int number) {
  while (!g_stop) {
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

void phil(Channel<> &left, Channel<> &right, Channel<> &up, Channel<> &down,
          int delay_ms, int number) {
  while (!g_stop) {
    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    down.Write();
    // print(number, ". Thinking.\n");

    ForkJoin pickup_forks{[&]() { left.Write(); }, [&]() { right.Write(); }};
    // print(number, ". Eating.\n");

    ForkJoin putdown_forks{[&]() { left.Write(); }, [&]() { right.Write(); }};
    up.Write();
  }
}

void butler(std::vector<Channel<>> &up, std::vector<Channel<>> &down,
            int delay_ms, int N, bool testing, Channel<> &test_channel) {
  int number_of_seated = 0;
  while (!g_stop) {
    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));

    if (number_of_seated < N - 1) {
      Choice c(down);
      int select = c.Select();
      if (select != -1) {
        down[select].Read();
        number_of_seated++;
        // print("number_of_seated: ", number_of_seated, "\n");
        std::cout << "number_of_seated: " << number_of_seated << std::endl;

        if (testing) {
          // std::cout << "Writing!" << std::endl;
          test_channel.Write(number_of_seated);
        }
      }
    } else {
      Choice c(up);
      int select = c.Select();
      if (select != -1) {
        up[select].Read();
        number_of_seated--;
        // print("number_of_seated: ", number_of_seated, "\n");
        std::cout << "number_of_seated: " << number_of_seated << std::endl;

        if (testing) {
          test_channel.Write(number_of_seated);
        }
      }
    }
  }
}

void college(const int N, bool testing, Channel<> &test_channel) {
  std::vector<Channel<>> left(N);
  std::vector<Channel<>> right(N);
  std::vector<Channel<>> up(N);
  std::vector<Channel<>> down(N);

  std::vector<std::function<void()>> processes;

  for (int i = 0; i < N; i++) {
    processes.push_back(std::bind(phil, std::ref(left[i]), std::ref(right[i]),
                                  std::ref(up[i]), std::ref(down[i]), 0, i));
  }

  for (int i = 0; i < N; i++) {
    processes.push_back(
        std::bind(fork, std::ref(right[i]), std::ref(left[(i + 1) % N]), 0, i));
  }

  processes.push_back(std::bind(butler, std::ref(up), std::ref(down), 0, N,
                                testing, std::ref(test_channel)));

  ForkJoin f(processes);
}
#endif /* A26A8D6B_086E_45B3_8258_A6B837E30ADF */
