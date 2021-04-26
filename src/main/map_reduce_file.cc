/**
 * Example run: bazel run src/main/map_reduce_file --cxxopt='-std=c++17' --
 * --input_file='/Users/ari/github/csp-in-cpp/data/romeojulliet.txt'
 */
#include <cassert>
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <random>
#include <set>
#include <string>
#include <thread>
#include <vector>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"
#include "src/lib/csp/csp.h"
#include "src/lib/csp/map_reduce.h"

ABSL_FLAG(std::string, input_file, "", "input file name");

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
/*
 * It will iterate through all the lines in file and
 * put them in given vector
 */
bool getFileContent(std::string& fileName,
                    std::vector<std::string>& vecOfStrs) {
  // Open the File
  std::ifstream in(fileName.c_str());
  // Check if object is valid
  if (!in) {
    std::cerr << "Cannot open the File : " << fileName << std::endl;
    return false;
  }
  std::string str;
  // Read the next line from File untill it reaches the end.
  while (std::getline(in, str)) {
    // Line contains string of length > 0 then save it in vector
    if (str.size() > 0) vecOfStrs.push_back(str);
  }
  // Close The File
  in.close();
  return true;
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

int main(int argc, char* argv[]) {
  absl::ParseCommandLine(argc, argv);

  Channel<>::BeginCSP();

  std::vector<std::string> lines;
  std::string file_name(absl::GetFlag(FLAGS_input_file));
  bool file_read_success = getFileContent(file_name, lines);

  if (!file_read_success) {
    std::cout << "Error in reading the intput file." << std::endl;
    return -1;
  }

  auto const number_of_mappers = lines.size();
  auto number_of_reducers = GetNumberOfUniqueWords(lines);

  std::cout << "number_of_mappers: " << number_of_mappers << std::endl;
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