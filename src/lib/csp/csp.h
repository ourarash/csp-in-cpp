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

// std::ostream& print_one(std::ostream& os);

// template <class A0, class... Args>
// std::ostream& print_one(std::ostream& os, const A0& a0, const Args&... args);

// template <class... Args>
// std::ostream& print(std::ostream& os, const Args&... args) ;

// std::mutex& get_cout_mutex() ;

// template <class... Args>
// std::ostream& print(const Args&... args) ;
//-----------------------------------------------------
enum class ChannelStatus { idle, r_pend, w_pend, s12m_pend };
//-----------------------------------------------------

template <typename T = int>
class Channel {
 public:
  Channel() {}
  // Use this if it's a shared 1-to-many channel with multiple receivers.
  Channel(int number_of_receivers)
      : _number_of_receivers(number_of_receivers) {}

  void Write(T data = 0) {
    try {
      std::unique_lock<std::mutex> ul(_mutex);
      _req = !_req;
      _data = data;
      _status = ChannelStatus::w_pend;
      ul.unlock();
      _cl.notify_all();
      ul.lock();

      _cl.wait(ul, [=]() { return _ack != _prev_ack; });
      _status = ChannelStatus::idle;
      _prev_ack = _ack;
    } catch (const std::exception& e) {
      std::cout << "Got this error:" << e.what() << std::endl;
    }
  }

  T Read() {
    int data;
    try {
      std::unique_lock<std::mutex> ul(_mutex);

      _status = ChannelStatus::r_pend;
      // if blocked, ul.unlock() is automatically called.
      _cl.wait(ul, [=]() { return _req != _prev_req; });
      // if unblocks, ul.lock() is automatically called
      data = _data;

      if (_receive_counter == _number_of_receivers - 1) {
        _ack = !_ack;
        _status = ChannelStatus::idle;
        _receive_counter = 0;
        _prev_req = _req;
        ul.unlock();
        _cl.notify_all();
        ul.lock();
      } else {
        _status = ChannelStatus::s12m_pend;
        _receive_counter++;
        _cl.wait(ul, [=]() { return _receive_counter == 0; });
      }

    } catch (const std::exception& e) {
      std::cout << "Got this error:" << e.what() << std::endl;
    }
    return data;
  }
  ChannelStatus getStatus() { return _status; }
  bool IsIdle() const { return _status == ChannelStatus::idle; }
  bool IsBusy() const { return _status != ChannelStatus::idle; }

 private:
  std::mutex _mutex;
  std::condition_variable _cl;
  bool _req = false;
  bool _ack = false;
  bool _prev_req = false;
  bool _prev_ack = false;
  T _data;
  int _number_of_receivers = 1;
  int _receive_counter = 0;
  ChannelStatus _status = ChannelStatus::idle;
};  // namespace csp
//-----------------------------------------------------
/**
 * \class Choice
 * \brief Implements CSP choice construct
 * \author Ari Saif (https://www.youtube.com/channel/UCuRf9tqJaRgXLyl85Nf-Vtg)
 */
class Choice {
 private:
  std::vector<bool> _conditions;

 public:
  /**
   * \brief: creates a new Choice from an initialization list of boolean values
   */
  Choice(std::initializer_list<bool>&& init_list) noexcept {
    for (const auto& entry : init_list) {
      _conditions.push_back(entry);
    }
  }

  /**
   * \brief: creates a new Choice from an initialization list of channels
   */
  Choice(std::initializer_list<Channel<>>&& init_list) noexcept {
    for (const auto& entry : init_list) {
      _conditions.push_back(!entry.IsIdle());
    }
  }

  /**
   * \brief: creates a new Choice from a vector of channels
   */
  Choice(std::vector<Channel<>>& init_list) noexcept {
    for (const auto& entry : init_list) {
      _conditions.push_back(!entry.IsIdle());
    }
  }

  /**
   * \brief Selects between enabled choices, returns -1 if none enabled
   */
  int Select() {
    std::vector<int> enable_indices;

    for (int i = 0; i < _conditions.size(); i++) {
      if (_conditions[i]) {
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
  ForkJoin(std::initializer_list<std::function<void()>>&& init_list) noexcept {
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
  ParBlock(std::initializer_list<std::function<void()>>&& init_list) noexcept {
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
    std::cout << "here2!" << std::endl;
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
