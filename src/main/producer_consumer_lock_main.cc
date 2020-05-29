#include <condition_variable>  // std::condition_variable, std::cv_status
#include <iostream>
#include <mutex>  // For std::unique_lock
#include <random>
#include <thread>

// Create a function to generate a random value between 0 and 10
auto GenRandomValue = std::bind(std::uniform_int_distribution<>(0, 10),
                                std::default_random_engine());

std::mutex g_mutex;
bool g_ready = false;
int g_data = 0;

// A demo for unique_lock: similar to lock_guard, but it can
// be lock and unlock multiple times.

// Run this using one of the following methods:
//  1. With bazel: bazel run src/main/g_mutex:{THIS_FILE_NAME_WITHOUT_EXTENSION}
//  2. With plain g++: g++ -std=c++17 -lpthread
//  src/main/g_mutex/{THIS_FILE_NAME}.cc  -I ./
void consumer() {
  int receivedCount = 0;

  while (true) {
    std::unique_lock<std::mutex> ul(g_mutex);
    std::cout << "waiting..." << std::endl;
    while (!g_ready) {
      ul.unlock();
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      ul.lock();
    }
    receivedCount++;
    std::cout << "got g_data: " << g_data << std::endl;
    std::cout << "receivedCount: " << receivedCount << std::endl;
    g_ready = false;
  }
}

void producer() {
  int sentCount = 0;

  while (true) {
    std::unique_lock<std::mutex> ul(g_mutex);

    g_data = GenRandomValue();
    g_ready = true;
    sentCount++;
    std::cout << "sentCount: " << sentCount << std::endl;
    ul.unlock();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
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