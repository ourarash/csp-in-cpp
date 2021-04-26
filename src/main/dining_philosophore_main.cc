#include <cassert>
#include <chrono>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

#include "src/lib/csp/csp.h"
#include "src/lib/csp/dining_philosophors.h"
#include "src/lib/utility.h"
namespace csp {
bool g_stop = false;
std::mutex g_mutex;
std::condition_variable g_cv;
}  // namespace csp
using namespace csp;

int main() {
  Channel<>::BeginCSP();

  const int N = 5;
  Channel<> test_channel;

  ForkJoin f{// Create table
             [&]() { Table(N, true, std::ref(test_channel)); },
             // Read from test channel
             [&]() {
               for (int i = 0; i < 100; i++) {
                 auto seated = test_channel.Read();

                 std::cout << "i: " << i << ", seated: " << seated << std::endl;
                 assert(seated < N);
               }
               Channel<>::EndCSP();
             }};
z}