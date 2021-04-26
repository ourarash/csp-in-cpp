#ifndef C05C5D92_3C03_4A4B_B5BE_01C8F7163D1A
#define C05C5D92_3C03_4A4B_B5BE_01C8F7163D1A

#include <exception>
#include <map>
#include <vector>

#include "src/lib/csp/csp.h"

namespace csp {
extern bool g_stop;
extern std::mutex g_mutex;
extern std::condition_variable g_cv;
}  // namespace csp
using namespace csp;
Channel<> dummy_test_channel;

class ShuffleOutputWidthException : public std::exception {
  virtual const char *what() const throw() {
    return "Shuffle output channel width is less than the number of keys.";
  }
};

/**
 * \brief: Reads a map from in channel and emits (i.e. writes) a vector of pairs
 * in its output channel.
 */
template <typename TKeyIn, typename TValIn, typename TKeyOut, typename TValOut>
void MapNode(Channel<std::pair<TKeyIn, TValIn>> &in,
             Channel<std::vector<std::pair<TKeyOut, TValOut>>> &out,
             std::function<std::vector<std::pair<TKeyOut, TValOut>>(
                 std::pair<TKeyIn, TValIn> &)>
                 map_function) {
  std::pair<TKeyIn, TValIn> input = in.Read();
  std::vector<std::pair<TKeyOut, TValOut>> output;

  out.Write(map_function(input));
}
//-----------------------------------------------------------------------------
/**
 * \brief: Reads a vector of pairs from each of its input channels, and writes a
 * pair of (key, vector(value)) on each of its output channels.
 */
template <typename TKey, typename TVal>
void ShuffleNode(
    std::vector<Channel<std::vector<std::pair<TKey, TVal>>>> &in,
    std::vector<Channel<std::pair<TKey, std::vector<TVal>>>> &out) {
  std::map<TKey, std::vector<TVal>> allInputs;

  // Iterates through all channels.
  for (Channel<std::vector<std::pair<TKey, TVal>>> &cin : in) {
    auto eachInput = cin.Read();

    // Iterates through the vector of pairs.
    for (auto &[key, val] : eachInput) {
      allInputs[key].push_back(val);
    }
  }

  // Writes each (key, vector(value)) on an output channel
  int counter = 0;
  for (auto &p : allInputs) {
    if (counter > out.size()) {
      ShuffleOutputWidthException ex;
      throw(ex);
    }
    out[counter++].Write(p);
  }
}

//-----------------------------------------------------------------------------
template <typename TKey, typename TVal, typename TResult>
void ReduceNode(Channel<std::pair<TKey, std::vector<TVal>>> &in,
                Channel<std::pair<TKey, TResult>> &out,
                std::function<TResult(std::pair<TKey, std::vector<TVal>> &)>
                    reduce_function) {
  std::pair<TKey, std::vector<TVal>> input = in.Read();
  out.Write(std::pair(input.first, reduce_function(input)));
}
//-----------------------------------------------------------------------------
template <typename TKeyIn, typename TValIn, typename TKeyOut, typename TValOut,
          typename TResult>
void MapReduce(

    std::vector<Channel<std::pair<TKeyIn, TValIn>>> &map_in_channels,
    std::vector<Channel<std::pair<TKeyOut, TResult>>> &reduce_out_channels,
    std::function<
        std::vector<std::pair<TKeyOut, TValOut>>(std::pair<TKeyIn, TValIn> &)>
        map_function,
    std::function<TResult(std::pair<TKeyOut, std::vector<TValOut>> &)>
        reduce_function) {
  auto const number_of_mappers = map_in_channels.size();
  auto const number_of_reducers = reduce_out_channels.size();

  std::vector<std::function<void()>> processes;

  std::vector<Channel<std::vector<std::pair<TKeyOut, TValOut>>>>
      map_out_channels(number_of_mappers);

  std::vector<Channel<std::pair<TKeyOut, std::vector<TValOut>>>>
      reduce_in_channels(number_of_reducers);

  // Mappers
  for (int i = 0; i < number_of_mappers; i++) {
    processes.push_back(std::bind(MapNode<TKeyIn, TValIn, TKeyOut, TValOut>,
                                  std::ref(map_in_channels[i]),
                                  std::ref(map_out_channels[i]), map_function));
  }

  // Shuffler
  processes.push_back(std::bind(ShuffleNode<TKeyOut, TValOut>,
                                std::ref(map_out_channels),
                                std::ref(reduce_in_channels)));

  // Reducers
  for (int i = 0; i < number_of_reducers; i++) {
    processes.push_back(std::bind(
        ReduceNode<TKeyOut, TValOut, TResult>, std::ref(reduce_in_channels[i]),
        std::ref(reduce_out_channels[i]), reduce_function));
  }

  ForkJoin f(processes);
}

#endif /* C05C5D92_3C03_4A4B_B5BE_01C8F7163D1A */
