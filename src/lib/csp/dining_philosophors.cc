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
namespace csp {
extern bool g_stop;
extern std::mutex g_mutex;
extern std::condition_variable g_cv;
}  // namespace csp
using namespace csp;
// std::mutex PrintThread::_mutexPrint{};
auto gen = std::bind(std::uniform_int_distribution<>(0, 10),
                     std::default_random_engine());

/**
 * \brief: Makes a choice between left and right and syncs on the result.
 */
void Fork(Channel<> &left, Channel<> &right, int delay_ms, int number) {
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

/**
 * \brief: Sits down, picks up left and right forks, and then stands up.
 *         left and right communication are with forks.
 *         up and down communications are with the waiter.
 */
void Phil(Channel<> &left, Channel<> &right, Channel<> &up, Channel<> &down,
          int delay_ms, int number) {
  while (!g_stop) {
    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    // Sits down.
    down.Write();

    // Picks up forks.
    ForkJoin pickup_forks{[&]() { left.Write(); }, [&]() { right.Write(); }};

    // Eats...

    // Puts down forks.
    ForkJoin putdown_forks{[&]() { left.Write(); }, [&]() { right.Write(); }};

    // Stands up.
    up.Write();
  }
}

/**
 * \brief: Allows only N-1 philosophors to sit down.
 *         If there is N-1 seated ones, it waits and blocks all sit-down
 *         requests until at least one philosophor stands up.
 */
void Waiter(std::vector<Channel<>> &up, std::vector<Channel<>> &down,
            int delay_ms, int N, bool testing, Channel<> &test_channel) {
  int number_of_seated = 0;
  while (!g_stop) {
    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));

    if (number_of_seated < N - 1) {
      // Select one of the sit-down requests.
      Choice c(down);
      int select = c.Select();

      if (select != -1) {
        down[select].Read();
        number_of_seated++;
        if (testing) {
          test_channel.Write(number_of_seated);
        }
      }
    } else {
      // Wait until at least one philosophor stands up.
      Choice c(up);
      int select = c.Select();

      if (select != -1) {
        up[select].Read();
        number_of_seated--;
        if (testing) {
          test_channel.Write(number_of_seated);
        }
      }
    }
  }
}

/**
 * \brief: Connects forks, philosophors, and the waiter.
 */
void Table(const int N, bool testing, Channel<> &test_channel) {
  std::vector<Channel<>> left(N);
  std::vector<Channel<>> right(N);
  std::vector<Channel<>> up(N);
  std::vector<Channel<>> down(N);

  std::vector<std::function<void()>> processes;

  // Philosophers
  for (int i = 0; i < N; i++) {
    processes.push_back(std::bind(Phil, std::ref(left[i]), std::ref(right[i]),
                                  std::ref(up[i]), std::ref(down[i]),
                                  /*delay_ms=*/0, /*number=*/i));
  }

  // Forks
  for (int i = 0; i < N; i++) {
    processes.push_back(std::bind(Fork, std::ref(right[i]),
                                  std::ref(left[(i + 1) % N]), /*delay_ms=*/0,
                                  i));
  }

  // Waiter
  processes.push_back(std::bind(Waiter, std::ref(up), std::ref(down),
                                /*delay_ms=*/0, N, testing,
                                std::ref(test_channel)));

  ForkJoin f(processes);
}
#endif /* A26A8D6B_086E_45B3_8258_A6B837E30ADF */
