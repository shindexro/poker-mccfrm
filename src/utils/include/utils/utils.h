#ifndef __RANDON_H__
#define __RANDON_H__

#include "utils/random.h"
#include "game/hand.h"
#include "game/card.h"
#include "abstraction/global.h"

#include <tuple>
#include <vector>
#include <boost/serialization/vector.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <indicators/block_progress_bar.hpp>
#include <indicators/cursor_control.hpp>
#include <oneapi/tbb.h>

using namespace std;

namespace utils
{
    tuple<int, int> GetWorkItemsIndices(int dataCount, int threadCount, int threadIndex);
    int SampleDistribution(vector<float> &probabilities);
    int SampleDistribution(vector<double> &probabilities);

    vector<Hand> GetStartingHandChart();

    template <typename T>
    void SaveToFile(T &obj, const string &filename)
    {
        cout << "Saving to file " << filename << endl;
        ofstream file(filename);
        boost::archive::binary_oarchive archive(file);
        archive << obj;
    }

    template <typename T>
    void LoadFromFile(T &obj, const string &filename)
    {
        cout << "Loading from file " << filename << endl;
        ifstream file(filename);
        boost::archive::binary_iarchive archive(file);
        archive >> obj;
    }
    bool FileExists(const string &filename);

    template <typename T>
    void parallelise(long maxCount, T func)
    {
        cout << "Processing " << maxCount << " items in " << Global::NOF_THREADS << " threads" << endl;
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
        const long barUpdateInterval = max(1L, maxCount / 10000 / Global::NOF_THREADS);
        oneapi::tbb::parallel_for(0, Global::NOF_THREADS,
                                  [&](int threadIdx)
                                  {
                                      long threadIterations = 0;
                                      auto [startItemIdx, endItemIdx] = GetWorkItemsIndices(maxCount, Global::NOF_THREADS, threadIdx);
                                      for (int i = startItemIdx; i < endItemIdx; i++)
                                      {
                                          if (threadIterations == barUpdateInterval)
                                          {
                                              iterations += threadIterations;
                                              threadIterations = 0;
                                              bar.set_progress(iterations);
                                          }
                                          func(threadIdx, i);
                                          threadIterations++;
                                      }
                                      iterations += threadIterations;
                                      bar.set_progress(iterations);
                                  });

        indicators::show_console_cursor(true);
        cout << endl;
    }

    template <typename Base, typename T>
    inline bool instanceof(const T *ptr)
    {
        return dynamic_cast<const Base *>(ptr) != nullptr;
    }
}

#endif