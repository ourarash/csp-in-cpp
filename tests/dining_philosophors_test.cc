#include "src/lib/csp/dining_philosophors.h"

#include <vector>

#include "glog/logging.h"
#include "glog/stl_logging.h"
#include "gtest/gtest.h"
#include "src/lib/csp/csp.h"

namespace csp {
bool g_stop = false;
std::mutex g_mutex;
std::condition_variable g_cv;
}  // namespace csp

inline void setup(int N, bool debugLog = false) {
  Channel<>::BeginCSP();
  Channel<> test_channel;

  ForkJoin f{// Unit under test
             std::bind(Table, N, true, std::ref(test_channel)),
             // Testing environment
             [&]() {
               for (int i = 0; i < 100; i++) {
                 auto seated = test_channel.Read();
                 if (debugLog) {
                   LOG(INFO)
                       << "N: " << N << ", seated: " << seated << std::endl;
                 }

                 EXPECT_LT(seated, N);
               }
               Channel<>::EndCSP();
             }};
}

TEST(CSPTest, DiningPhilosophors) {
  for (int n = 2; n < 100; n++) {
    setup(n);
  }
}
