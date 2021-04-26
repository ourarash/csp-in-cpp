#ifndef D33FF845_A9B2_4D52_AD68_A63295B8923F
#define D33FF845_A9B2_4D52_AD68_A63295B8923F

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <optional>
#include <random>
#include <sstream>
#include <thread>
#include <vector>

//-----------------------------------------------------
namespace csp {

extern bool g_stop;
extern std::mutex g_mutex;
extern std::condition_variable g_cv;

//-----------------------------------------------------
enum class ChannelStatus { idle, r_pend, w_pend, s12m_pend };
//-----------------------------------------------------
/**
 * A class describing CSP channels.
 */
template <typename T = int>
class Channel {
 public:
  Channel()
      : sentry_(
            // A thread that unblocks Read/Write when g_stop is true;
            std::thread([&]() {
              std::unique_lock<std::mutex> ul(g_mutex);
              g_cv.wait(ul, [&]() { return g_stop == true; });
              cl_.notify_all();
            })) {}
  // Use this if it's a shared 1-to-many channel with multiple receivers.
  Channel(int number_of_receivers) : Channel() {
    number_of_receivers_ = number_of_receivers;
  }

  ~Channel() {
    if (sentry_.joinable()) {
      sentry_.join();
    }
  }

  // Performs CSP Write on the channel.
  void Write(T data = T()) {
    try {
      // Write data and set req.
      std::unique_lock<std::mutex> ul(mutex_);
      req_ = !req_;
      data_ = data;
      status_ = ChannelStatus::w_pend;
      ul.unlock();
      cl_.notify_all();
      ul.lock();

      // Wait for ack.
      cl_.wait(ul, [=]() { return ack_ != prev_ack_ || g_stop; });
      status_ = ChannelStatus::idle;
      prev_ack_ = ack_;
    } catch (const std::exception& e) {
      std::cout << "Error in Write:" << e.what() << std::endl;
    }
  }

  T Read() {
    T data;
    try {
      std::unique_lock<std::mutex> ul(mutex_);

      status_ = ChannelStatus::r_pend;
      // if blocked, ul.unlock() is automatically called.
      cl_.wait(ul, [=]() { return req_ != prev_req_ || g_stop; });
      // if unblocks, ul.lock() is automatically called
      data = data_;

      if (receive_counter_ == number_of_receivers_ - 1) {
        ack_ = !ack_;
        status_ = ChannelStatus::idle;
        receive_counter_ = 0;
        prev_req_ = req_;
        ul.unlock();
        cl_.notify_all();
        ul.lock();
      } else {
        status_ = ChannelStatus::s12m_pend;
        receive_counter_++;
        cl_.wait(ul, [=]() { return receive_counter_ == 0 || g_stop; });
      }

    } catch (const std::exception& e) {
      std::cout << "Error in Read:" << e.what() << std::endl;
    }
    return data;
  }
  ChannelStatus getStatus() { return status_; }
  bool IsIdle() const { return status_ == ChannelStatus::idle; }
  bool IsBusy() const { return status_ != ChannelStatus::idle; }

  static void BeginCSP() {
    std::unique_lock<std::mutex> ul(g_mutex);
    g_stop = false;
    g_cv.notify_all();
  }
  static void EndCSP() {
    std::unique_lock<std::mutex> ul(g_mutex);
    g_stop = true;
    g_cv.notify_all();
  }

 private:
  std::mutex mutex_;
  std::condition_variable cl_;
  bool req_ = false;
  bool ack_ = false;
  bool prev_req_ = false;
  bool prev_ack_ = false;
  T data_;
  int number_of_receivers_ = 1;
  int receive_counter_ = 0;
  ChannelStatus status_ = ChannelStatus::idle;
  std::thread sentry_;
};  // namespace csp
//-----------------------------------------------------
/**
 * \class Choice
 * \brief Implements CSP choice construct.
 *        Randomly selects an true item from a list of boolean variables.
 * \author Ari Saif (https://www.youtube.com/c/arisaif)
 */
class Choice {
 private:
  std::vector<bool> conditions_;

 public:
  /**
   * \brief: creates a new Choice from an initialization list of boolean values
   */
  Choice(std::initializer_list<bool> init_list) noexcept {
    for (const auto& entry : init_list) {
      conditions_.push_back(entry);
    }
  }

  /**
   * \brief: creates a new Choice from an initialization list of channels
   */
  Choice(std::initializer_list<Channel<>> init_list) noexcept {
    for (const auto& entry : init_list) {
      conditions_.push_back(entry.IsBusy());
    }
  }

  /**
   * \brief: creates a new Choice from a vector of channels
   */
  Choice(std::vector<Channel<>>& init_list) noexcept {
    for (const auto& entry : init_list) {
      conditions_.push_back(entry.IsBusy());
    }
  }

  /**
   * \brief Selects between enabled choices, returns -1 if none enabled
   */
  int Select() {
    std::vector<int> enable_indices;

    for (int i = 0; i < conditions_.size(); i++) {
      if (conditions_[i]) {
        enable_indices.push_back(i);
      }
    }
    if (enable_indices.empty()) {
      return -1;
    } else {
      // Will be used to obtain a seed for the random number engine
      std::random_device rd;
      // Standard mersenne_twister_engine seeded with rd()
      std::mt19937 gen(rd());

      // Generate a random value between 0 and enable_indices.size() - 1
      std::uniform_int_distribution<> dis(0, enable_indices.size() - 1);

      return enable_indices[dis(gen)];
    }
  }
};

class ForkJoin {
 private:
  std::vector<std::thread> _threads;
  std::vector<std::function<void()>> _funcs;

 public:
  ForkJoin(std::initializer_list<std::function<void()>> init_list) noexcept {
    for (auto& entry : init_list) {
      _threads.push_back(std::thread(entry));
    }

    for (auto& t : _threads) {
      if (t.joinable()) {
        t.join();
      }
    }
  }

  ForkJoin(std::vector<std::function<void()>>& init_list) noexcept {
    for (auto& entry : init_list) {
      _threads.push_back(std::thread(entry));
    }
    for (auto& t : _threads) {
      if (t.joinable()) {
        t.join();
      }
    }
  }
  void Detach(int i) {
    std::cout << "Detaching!" << std::endl;
    std::cout << "_threads.size(): " << _threads.size() << std::endl;
    std::cout << "i: " << i << std::endl;
    // for (auto& t : _threads) {
    //   std::cout << "HI" << std::endl;
    //   t.detach();
    // }
    std::thread::id this_id = _threads[i].get_id();
    std::cout << "this_id: " << this_id << std::endl;

    _threads[i].detach();
  }
};

class ParBlock {
 private:
  std::vector<std::thread> _threads;
  std::vector<std::function<void()>> _funcs;

 public:
  ParBlock(std::initializer_list<std::function<void()>> init_list) noexcept {
    for (auto& entry : init_list) {
      _funcs.push_back(entry);
    }
  }

  ParBlock(std::vector<std::function<void()>>& init_list) noexcept {
    for (auto& entry : init_list) {
      _funcs.push_back(entry);
    }
  }
  void ForkJoin() {
    for (auto& entry : _funcs) {
      _threads.push_back(std::thread(entry));
    }

    for (auto& t : _threads) {
      if (t.joinable()) {
        t.join();
      }
    }
  }

  void Fork() {
    for (auto& entry : _funcs) {
      _threads.push_back(std::thread(entry));
    }
  }

  void Join() {
    for (auto& t : _threads) {
      if (t.joinable()) {
        t.join();
      }
    }
  }

  void Detach(int i) {
    std::cout << "Detaching!" << std::endl;
    std::cout << "_threads.size(): " << _threads.size() << std::endl;
    std::cout << "i: " << i << std::endl;
    // for (auto& t : _threads) {
    //   std::cout << "HI" << std::endl;
    //   t.detach();
    // }

    _threads[i].detach();
  }
};

//-----------------------------------------------------------------------------
template <typename T>
void Source(Channel<T>& out, T& data) {
  out.Write(data);
}
//-----------------------------------------------------------------------------

/** Thread safe cout class
 * Exemple of use:
 *    PrintThread{} << "Hello world!" << std::endl;
 */
// class PrintThread : public std::ostringstream {
//  public:
//   PrintThread() = default;

//   ~PrintThread() {
//     std::lock_guard<std::mutex> guard(_mutexPrint);
//     auto end = std::chrono::system_clock::now();
//     std::time_t end_time = std::chrono::system_clock::to_time_t(end);

//     std::cout << end_time << ". " << this->str();
//   }

//  private:
//   static std::mutex _mutexPrint;
// };
}  // namespace csp
#endif /* D33FF845_A9B2_4D52_AD68_A63295B8923F */
