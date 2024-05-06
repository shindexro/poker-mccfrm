#include "algorithm/trainer_manager.h"
#include "utils/utils.h"
#include "cereal/types/vector.hpp"
#include "cereal/types/unordered_map.hpp"
#include "cereal/types/memory.hpp"
#include "cereal/types/bitset.hpp"
#include "cereal/archives/binary.hpp"
#include <fstream>


TrainerManager::TrainerManager() :
    TrainerManager(1)
{
}

TrainerManager::TrainerManager(int threadCount) :
    threadCount{threadCount},
    iterations{0},
    StrategyIntervalCountdown{StrategyInterval},
    DiscountIntervalCountdown{DiscountInterval},
    SaveToDiskIntervalCountdown{SaveToDiskInterval},
    TestGamesIntervalCountdown{TestGamesInterval}
{
    trainers = vector<Trainer>();
    for (auto i = 0; i < threadCount; i++)
    {
        trainers.push_back(Trainer(i));
    }
}

void TrainerManager::StartTraining()
{
    LoadTrainedData();
    std::cout << "Starting Monte Carlo Counterfactual Regret Minimization (MCCFRM)..." << std::endl;

    // just for debugging purpose, I don't think it's necessary to enumeration the entire action space
    // auto threadFunc = [&](int idx) {
    //     trainers[idx].rootState->CreateChildren();
    //     trainers[idx].rootState->children[0]->CreateChildren();
    //     trainers[idx].EnumerateActionSpace(trainers[idx].rootState->children[0]->children[idx]);
    // };
    // oneapi::tbb::parallel_for(0, Global::NOF_THREADS, threadFunc);

    // TODO: find out whether this line is the bottleneck?
    // although the size of the nodeMap will gradually approach 500M during training?
    // is 500M a good estimate?
    // Global::nodeMap.rehash(500000000);

    oneapi::tbb::parallel_for(0, threadCount,
            [&](int index)
            {
                StartTrainer(index);
            }
    );
}

void TrainerManager::StartTrainer(int index)
{
    auto trainer = &trainers[index];


    chrono::steady_clock::time_point start = chrono::steady_clock::now();
    for (auto t = 1;; t++)
    {
        for (auto traverser = 0; traverser < Global::nofPlayers; traverser++)
        {
            bool pruneEnabled = t > PruneThreshold;
            trainer->TrainOneIteration(traverser, pruneEnabled);
        }

        if (t % CountdownInterval == 0)
        {
            iterations += CountdownInterval;
            std::cout << "Training steps " << iterations << " "
                << "thread " << index
                << std::endl;

            StrategyIntervalCountdown -= CountdownInterval;
            DiscountIntervalCountdown -= CountdownInterval;
            SaveToDiskIntervalCountdown -= CountdownInterval;
            TestGamesIntervalCountdown -= CountdownInterval;

            chrono::steady_clock::time_point end = chrono::steady_clock::now();
            auto elapsed = chrono::duration_cast<std::chrono::seconds>(end - start).count();
            std::cout << "Thread it/s: " << CountdownInterval / (elapsed + 1) << " "
                << "elasped:" << elapsed << "[s] "
                << "strategyCD:" << StrategyIntervalCountdown << " "
                << "discountCD:" << DiscountIntervalCountdown << " "
                << "save2diskCD:" << SaveToDiskIntervalCountdown << " "
                << "testGameCD:" << TestGamesIntervalCountdown << " "
                << std::endl;
            start = end;
        }

        RunSingleThreadTasks(index, t);
    }
}

void TrainerManager::RunSingleThreadTasks(int index, int current_iteration)
{
    auto trainer = &trainers[index];

    if (index == 0)
    {
        if (StrategyIntervalCountdown <= 0)
        {
            StrategyIntervalCountdown = StrategyInterval;
            for (auto traverser = 0; traverser < Global::nofPlayers; traverser++)
            {
                trainer->UpdateStrategy(traverser);
            }
            std::cout << "Updated strategy" << std::endl;
        }
    }

    if (index == 1)
    {
        if (current_iteration < LCFRThreshold && DiscountIntervalCountdown <= 0)
        {
            DiscountIntervalCountdown = DiscountInterval;
            float d = ((float)current_iteration / DiscountInterval) / ((float)current_iteration / DiscountInterval + 1);
            trainer->DiscountInfosets(d);
            std::cout << "Discounted infosets" << std::endl;
        }
    }

    if (index == 2)
    {
        if (TestGamesIntervalCountdown <= 0)
        {
            TestGamesIntervalCountdown = TestGamesInterval;
            trainer->PrintStartingHandsChart();
            trainer->PrintStatistics(iterations);
        }
    }

    if (index == 3)
    {
        if (SaveToDiskIntervalCountdown <= 0)
        {
            SaveToDiskIntervalCountdown = SaveToDiskInterval;
            SaveTrainedData();
        }
    }

}

namespace cereal
{
  template <class Archive, class K, class V, class Hash, class Eq, class A, size_t N, class Mtx_> inline
  void save( Archive &ar,
          phmap::parallel_flat_hash_map<K, V, Hash, Eq, A, N, Mtx_> const &hmap)
  {
    ar(hmap.size());
    for( const auto & i : hmap )
      ar( i.first, i.second );
  }

  template <class Archive, class K, class V, class Hash, class Eq, class A, size_t N, class Mtx_> inline
  void load( Archive &ar,
          phmap::parallel_flat_hash_map<K, V, Hash, Eq, A, N, Mtx_> &hmap)
  {
    hmap.clear();

    size_t sz;
    ar(sz);

    for (size_t i = 0; i < sz; i++)
    {
      K k;
      V v;
      ar( k, v );
      hmap.insert({k, v});
    }
  }

  template <class Archive> inline
  void serialize( Archive &ar,
          Infoset &infoset)
  {
      ar( infoset.regret, infoset.actionCounter );
  }
} // namespace cereal


void TrainerManager::SaveTrainedData()
{
    auto epoch = utils::GetSecondsSinceEpoch();
    ostringstream filename;
    filename << "nodeMap-" <<  epoch << ".bin";
    std::cout << "Saving trained data to file " << filename.str() << std::endl;

    std::ofstream os(filename.str(), std::ios::binary);
    cereal::BinaryOutputArchive ar(os);
    ar(CEREAL_NVP(Global::nodeMap));
    std::cout << "Saved trained data" << std::endl;
}

void TrainerManager::LoadTrainedData()
{
    if (!utils::FileExists("nodeMap.bin"))
        return;
    std::cout << "Loading trained data from file nodeMap.bin..." << std::endl;
    std::ifstream is("nodeMap.bin", std::ios::binary);
    cereal::BinaryInputArchive ar(is);
    ar(CEREAL_NVP(Global::nodeMap));
    std::cout << "Loaded trained data" << std::endl;
}
