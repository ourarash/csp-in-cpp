#include <condition_variable>  // std::condition_variable, std::cv_status
#include <future>
#include <iostream>
#include <mutex>  // For std::unique_lock
#include <numeric>
#include <random>
#include <thread>
#include <vector>

#include "src/lib/utility.h"
std::mutex mutex;
std::condition_variable cl;
bool ready = false;

// A demo for unique_lock: similar to lock_guard, but it can
// be lock and unlock multiple times.

// Run this using one of the following methods:
//  1. With bazel: bazel run src/main/mutex:{THIS_FILE_NAME_WITHOUT_EXTENSION}
//  2. With plain g++: g++ -std=c++17 -lpthread
//  src/main/mutex/{THIS_FILE_NAME}.cc  -I ./
void consumer() {
  int receivedCount = 0;

  while (true) {
    std::unique_lock<std::mutex> ul(mutex);

    // if blocked, ul.unlock() is automatically called.
    std::cout << "waiting..." << std::endl;
    cl.wait(ul, []() { return ready; });
    // if blocked, ul.lock() is automatically called

    receivedCount++;
    std::cout << "got ready: " << ready << std::endl;
    std::cout << "receivedCount: " << receivedCount << std::endl;
    ready = false;
  }
}

void producer() {
  auto gen = std::bind(std::uniform_int_distribution<>(0, 1),
                       std::default_random_engine());
  int sentCount = 0;

  while (true) {
    std::unique_lock<std::mutex> ul(mutex);

    ready = (gen() == 1);

    if (ready) {
      sentCount++;
      std::cout << "sentCount: " << sentCount << std::endl;
    }
    ul.unlock();
    cl.notify_one();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    ul.lock();
  }
}

int main() {
  std::thread t1(consumer);
  std::thread t2(producer);
  t1.join();
  t2.join();
  return 0;
}