#include "csp.h"

#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <optional>
#include <thread>
// const std::optional<std::string>& param = std::nullopt

// std::ostream& print_one(std::ostream& os) { return os; }

// template <class A0, class... Args>
// std::ostream& print_one(std::ostream& os, const A0& a0, const Args&... args) {
//   os << a0;
//   return print_one(os, args...);
// }

// template <class... Args>
// std::ostream& print(std::ostream& os, const Args&... args) {
//   return print_one(os, args...);
// }

// std::mutex& get_cout_mutex() {
//   static std::mutex m;
//   return m;
// }

// template <class... Args>
// std::ostream& print(const Args&... args) {
//   std::lock_guard<std::mutex> _(get_cout_mutex());
//   return print(std::cout, args...);
// }
