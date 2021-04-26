#include <cassert>
#include <chrono>
#include <functional>
#include <iostream>
#include <random>
#include <set>
#include <string>
#include <thread>
#include <vector>

#include "src/lib/csp/csp.h"
#include "src/lib/csp/map_reduce.h"
// #include "src/lib/utility.h"
namespace csp {
bool g_stop = false;
std::mutex g_mutex;
std::condition_variable g_cv;
}  // namespace csp
using namespace csp;
//-----------------------------------------------------------------------------
std::vector<std::string> Split(const std::string& str, const char delim = ' ') {
  std::vector<std::string> retVal;

  size_t start = 0;
  size_t delimLoc = str.find_first_of(delim, start);
  while (delimLoc != std::string::npos) {
    retVal.emplace_back(str.substr(start, delimLoc - start));
    start = delimLoc + 1;
    delimLoc = str.find_first_of(delim, start);
  }

  retVal.emplace_back(str.substr(start));
  return retVal;
}
//-----------------------------------------------------------------------------
size_t GetNumberOfUniqueWords(const std::vector<std::string>& in) {
  std::set<std::string> s;
  for (auto& line : in) {
    auto words = Split(line);
    for (auto& w : words) {
      s.insert(w);
    }
  }
  return s.size();
}
//-----------------------------------------------------------------------------
/**
 * \brief: maps each line into a list of pairs, where each pair is a word to the
 * number of times that word appeared in the line.
 * Each line is a pair of line number to a string.
 */
std::vector<std::pair<std::string, int>> map_function(
    std::pair<int, std::string>& in) {
  std::map<std::string, int> word_count;
  std::vector<std::pair<std::string, int>> result;

  auto words = Split(in.second);

  for (std::string& word : words) {
    word_count[word]++;
  }

  for (auto& p : word_count) {
    result.push_back(p);
  }

  return result;
}
//-----------------------------------------------------------------------------
int reduce_function(std::pair<std::string, std::vector<int>>& in) {
  return std::accumulate(in.second.begin(), in.second.end(), 0);
}
//-----------------------------------------------------------------------------

int main() {
  Channel<>::BeginCSP();

  std::vector<std::string> lines{"This car is a test test car",
                                 "That car is not a test test car"};

  auto const number_of_mappers = lines.size();
  auto number_of_reducers = GetNumberOfUniqueWords(lines);

  std::cout << "number_of_reducers: " << number_of_reducers << std::endl;

  std::vector<Channel<std::pair<int, std::string>>> map_in_channels(
      number_of_mappers);

  std::vector<Channel<std::pair<std::string, int>>> reduce_out_channels(
      number_of_reducers);

  ForkJoin f{// Create Map reduce
             [&]() {
               MapReduce<int, std::string, std::string, int, int>(
                   map_in_channels, reduce_out_channels, map_function,
                   reduce_function);
             },
             // Write into mappers.
             [&]() {
               for (int i = 0; i < number_of_mappers; i++) {
                 map_in_channels[i].Write(std::pair(i, lines[i]));
               }
             },
             // Read from reducers.
             [&]() {
               for (int i = 0; i < number_of_reducers; i++) {
                 auto p = reduce_out_channels[i].Read();
                 std::cout << p.first << ": " << p.second << std::endl;
               }

               Channel<>::EndCSP();
             }};
}