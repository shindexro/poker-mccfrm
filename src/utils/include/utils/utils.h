#ifndef __UTILS_H__
#define __UTILS_H__

#include "abstraction/global.h"
#include "game/card.h"
#include "game/hand.h"
#include "utils/random.h"

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/serialization/vector.hpp>
#include <chrono>
#include <indicators/block_progress_bar.hpp>
#include <indicators/cursor_control.hpp>
#include <oneapi/tbb.h>
#include <oneapi/tbb/concurrent_hash_map.h>
#include <tuple>
#include <vector>

using namespace std;
using namespace poker;

namespace utils {
tuple<int, int> GetWorkItemsIndices(int dataCount, int threadCount,
                                    int threadIndex);
int SampleDistribution(vector<float> &probabilities);
int SampleDistribution(vector<double> &probabilities);

vector<Hand> GetStartingHandChart();

template <typename T> void normalise(vector<T> &v) {
  T sum = 0;
  for (auto i = 0ul; i < v.size(); i++)
    sum += v[i];

  for (auto &n : v)
    n /= sum;
}

template <typename T> void SaveToFile(T &obj, const string &filename) {
  cout << "Saving to file " << filename << endl;
  ofstream file(filename);
  boost::archive::binary_oarchive archive(file, std::ios::binary);
  archive << obj;
}

template <typename T> void LoadFromFile(T &obj, const string &filename) {
  cout << "Loading from file " << filename << endl;
  ifstream file(filename);
  boost::archive::binary_iarchive archive(file, std::ios::binary);
  archive >> obj;
}
bool FileExists(const string &filename);

template <typename T> void parallelise(long maxCount, T func) {
  cout << "Processing " << maxCount << " items in " << Global::NOF_THREADS
       << " threads" << endl;
  cout << flush;

  using namespace indicators;
  indicators::show_console_cursor(false);
  BlockProgressBar bar{
      option::BarWidth{80},
      option::Start{"["},
      option::End{"]"},
      option::ForegroundColor{Color::white},
      option::FontStyles{std::vector<FontStyle>{FontStyle::bold}},
      option::ShowElapsedTime{true},
      option::ShowRemainingTime{true},
      option::MaxProgress{maxCount}};

  atomic<long> iterations = 0;
  const long barUpdateInterval = min(max(1L, maxCount / 100), 1000L);

  auto threadFunc = [&](int threadIdx) {
    long threadIterations = 0;
    auto [startItemIdx, endItemIdx] =
        GetWorkItemsIndices(maxCount, Global::NOF_THREADS, threadIdx);
    for (auto i = startItemIdx; i < endItemIdx; i++) {
      if (threadIterations == barUpdateInterval) {
        iterations += threadIterations;
        threadIterations = 0;
        bar.set_option(option::PostfixText{std::to_string(iterations) + "/" +
                                           std::to_string(maxCount)});
        bar.set_progress(iterations);
      }
      func(threadIdx, i);
      threadIterations++;
    }
    iterations += threadIterations;
    bar.set_option(option::PostfixText{std::to_string(iterations) + "/" +
                                       std::to_string(maxCount)});
    bar.set_progress(iterations);
  };

  oneapi::tbb::parallel_for(0, Global::NOF_THREADS, threadFunc);

  indicators::show_console_cursor(true);
  cout << endl;
}

template <typename Base, typename T> inline bool instanceof (const T *ptr) {
  return dynamic_cast<const Base *>(ptr) != nullptr;
}

decltype(std::chrono::seconds().count()) GetSecondsSinceEpoch();
} // namespace utils

namespace boost {
namespace serialization {
//////////////////////////////////////////////////////////////////////
// non-intrusive serialization for oneapi::tbb::concurrent_hash_map
template <class Archive, typename Key, typename T, typename HashCompare,
          typename Allocator>
inline void
save(Archive &ar,
     const oneapi::tbb::concurrent_hash_map<Key, T, HashCompare, Allocator> &t,
     const unsigned int /*file_version*/
) {
  size_t count(t.size());
  size_t bucket_count(t.bucket_count());

  ar << count;
  ar << bucket_count;

  for (auto item : t) {
    ar << item;
  }
}

template <class Archive, typename Key, typename T, typename HashCompare,
          typename Allocator>
inline void
load(Archive &ar,
     oneapi::tbb::concurrent_hash_map<Key, T, HashCompare, Allocator> &t,
     const unsigned int /*file_version*/
) {
  size_t count;
  size_t bucket_count;

  ar >> count;
  ar >> bucket_count;

  t.clear();
  t.rehash(bucket_count);
  while (count-- > 0) {
    pair<Key, T> item;
    ar >> item;
    t.insert(item);
  }
}

// split non-intrusive serialization function member into separate
// non intrusive save/load member functions
template <class Archive, typename Key, typename T, typename HashCompare,
          typename Allocator>
inline void
serialize(Archive &ar,
          oneapi::tbb::concurrent_hash_map<Key, T, HashCompare, Allocator> &t,
          const unsigned int file_version) {
  boost::serialization::split_free(ar, t, file_version);
}
} // namespace serialization
} // namespace boost

#endif
