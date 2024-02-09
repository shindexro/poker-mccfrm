#include "algorithm/trainer_manager.h"
#include "utils/utils.h"

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

    Global::nodeMap.rehash(500000000);

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

        static const int CountdownInterval = 10000;
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
            std::cout << "Iterations per second: " << iterations / (elapsed + 1) << " "
                << "elasped time: " << elapsed << "[s] "
                << "strategy countdown" << StrategyIntervalCountdown << " "
                << "discount countdown" << DiscountIntervalCountdown << " "
                << "save2disk countdown" << SaveToDiskIntervalCountdown << " "
                << "test game countdown" << TestGamesIntervalCountdown << " "
                << std::endl;
        }

        if (index != 0)
            continue;

        if (StrategyIntervalCountdown <= 0)
        {
            StrategyIntervalCountdown = StrategyInterval;
            for (auto traverser = 0; traverser < Global::nofPlayers; traverser++)
            {
                trainer->UpdateStrategy(traverser);
            }
            std::cout << "Updated strategy" << std::endl;
        }

        // if (t < LCFRThreshold && DiscountIntervalCountdown <= 0)
        // {
        //     DiscountIntervalCountdown = DiscountInterval;
        //     float d = ((float)t / DiscountInterval) / ((float)t / DiscountInterval + 1);
        //     trainer->DiscountInfosets(d);
        //     std::cout << "Discounted infosets" << std::endl;
        // }

        if (TestGamesIntervalCountdown <= 0)
        {
            TestGamesIntervalCountdown = TestGamesInterval;
            trainer->PrintStartingHandsChart();
            trainer->PrintStatistics(iterations);
        }
        if (SaveToDiskIntervalCountdown <= 0)
        {
            SaveToDiskIntervalCountdown = SaveToDiskInterval;
            SaveTrainedData();
        }
    }
}

void TrainerManager::SaveTrainedData()
{
    auto epoch = utils::GetSecondsSinceEpoch();
    ostringstream filename;
    filename << "nodeMap-" <<  epoch << ".txt";
    std::cout << "Saving trained data to file " << filename.str() << std::endl;
    utils::SaveToFile(Global::nodeMap, filename.str());
    std::cout << "Saved trained data" << std::endl;
}

void TrainerManager::LoadTrainedData()
{
    if (!utils::FileExists("nodeMap.txt"))
        return;
    std::cout << "Loading trained data from file nodeMap.txt..." << std::endl;
    utils::LoadFromFile(Global::nodeMap, "nodeMap.txt");
    std::cout << "Loaded trained data" << std::endl;
}
