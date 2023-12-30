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

    // trainers[0].EnumerateActionSpace(); // just for debugging purpose, I don't think it's necessary to enumeration the entire action space
    oneapi::tbb::parallel_for(0, threadCount,
            [&](int index)
            {
                StartTrainer(index);
            }
    );
}

void TrainerManager::StartTrainer(int index)
{
    auto trainer = trainers[index];

    chrono::steady_clock::time_point start = chrono::steady_clock::now();

    for (auto t = 1;; t++)
    {

        for (auto traverser = 0; traverser < Global::nofPlayers; traverser++)
        {
            bool pruneEnabled = t > PruneThreshold;
            trainer.TrainOneIteration(traverser, pruneEnabled);
        }

        if (t % 10000 == 0)
        {
            iterations += 10000;
            std::cout << "Training steps " << iterations << " "
                << "thread " << index
                << std::endl;

            StrategyIntervalCountdown -= 10000;
            DiscountIntervalCountdown -= 10000;
            SaveToDiskIntervalCountdown -= 10000;
            TestGamesIntervalCountdown -= 10000;

            chrono::steady_clock::time_point end = chrono::steady_clock::now();
            auto elapsed = chrono::duration_cast<std::chrono::seconds>(end - start).count();
            std::cout << "Iterations per second: " << iterations / (elapsed + 1) << std::endl;
        }

        if (index != 0)
            continue;

        for (auto traverser = 0; traverser < Global::nofPlayers; traverser++)
        {
            if (StrategyIntervalCountdown <= 0)
            {
                trainer.UpdateStrategy(traverser);
                StrategyIntervalCountdown = StrategyInterval;
            }
        }

        // discount all infosets (for all players)
        if (t < LCFRThreshold && DiscountIntervalCountdown <= 0)
        {
            float d = ((float)t / DiscountInterval) / ((float)t / DiscountInterval + 1);
            trainer.DiscountInfosets(d);
            DiscountIntervalCountdown = DiscountInterval;
        }

        if (TestGamesIntervalCountdown <= 0) // implement progress bar later
        {
            trainer.PrintStartingHandsChart();
            trainer.PrintStatistics(iterations);
            TestGamesIntervalCountdown = TestGamesInterval;

            // std::cout << "Sample games (against self)" << std::endl;
            // for (auto z = 0; z < 20; z++)
            // {
            //     trainer.PlayOneGame();
            // }
        }
        if (SaveToDiskIntervalCountdown <= 0)
        {
            SaveTrainedData();
            SaveToDiskIntervalCountdown = SaveToDiskInterval;
        }
    }
}

void TrainerManager::SaveTrainedData()
{
    std::cout << "Saving trained data to file nodeMap.txt" << std::endl;
    utils::SaveToFile(Global::nodeMap, "nodeMap.txt");
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
