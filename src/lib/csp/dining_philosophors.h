#ifndef A26A8D6B_086E_45B3_8258_A6B837E30ADF
#define A26A8D6B_086E_45B3_8258_A6B837E30ADF

#include <cassert>
#include <chrono>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

#include "src/lib/csp/csp.h"

using namespace csp;
Channel<> dummy_test_channel;

void Fork(Channel<> &left, Channel<> &right, int delay_ms, int number);
void Phil(Channel<> &left, Channel<> &right, Channel<> &up, Channel<> &down,
          int delay_ms, int number);
void Waiter(std::vector<Channel<>> &up, std::vector<Channel<>> &down,
            int delay_ms, int N, bool testing = false,
            Channel<> &test_channel = dummy_test_channel);

void Table(const int N = 5, bool testing = false,
           Channel<> &test_channel = dummy_test_channel);

#endif /* A26A8D6B_086E_45B3_8258_A6B837E30ADF */
