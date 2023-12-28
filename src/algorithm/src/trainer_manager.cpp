#include "algorithm/trainer_manager.h"
#include "utils/utils.h"

TrainerManager::TrainerManager() :
    TrainerManager(1)
{
}

TrainerManager::TrainerManager(int threadCount) :
    threadCount{threadCount},
    iterations{0}
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

    const long StrategyInterval = max(1, 10000 / Global::NOF_THREADS); // bb rounds before updating player strategy (recursive through tree) 10k
    const long LCFRThreshold = 20000000 / Global::NOF_THREADS;         // bb rounds when to stop discounting old regrets, no clue what it should be
    const long DiscountInterval = 1000000 / Global::NOF_THREADS;       // bb rounds, discount values periodically but not every round, 10 minutes
    const long SaveToDiskInterval = 100000 / Global::NOF_THREADS;
    const long TestGamesInterval = 100000 / Global::NOF_THREADS;
    const long PruneThreshold = 20000000 / Global::NOF_THREADS;  // bb rounds after this time we stop checking all actions, 200 minutes


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

            chrono::steady_clock::time_point end = chrono::steady_clock::now();
            auto elapsed = chrono::duration_cast<std::chrono::seconds>(end - start).count();
            std::cout << "Iterations per second: " << iterations / (elapsed + 1) << std::endl;
        }

        if (index != 0)
            continue;

        for (auto traverser = 0; traverser < Global::nofPlayers; traverser++)
        {
            if (t % StrategyInterval == 0)
            {
                trainer.UpdateStrategy(traverser);
            }
        }

        if (t % TestGamesInterval == 0) // implement progress bar later
        {
            trainer.PrintStartingHandsChart();
            trainer.PrintStatistics(iterations);

            std::cout << "Sample games (against self)" << std::endl;
            for (auto z = 0; z < 20; z++)
            {
                trainer.PlayOneGame();
            }
        }
        if (t % SaveToDiskInterval == 0)
        {
            SaveTrainedData();
        }

        // discount all infosets (for all players)
        if (t < LCFRThreshold && t % DiscountInterval == 0)
        {
            float d = ((float)t / DiscountInterval) / ((float)t / DiscountInterval + 1);
            trainer.DiscountInfosets(d);
        }
    }
}

void TrainerManager::SaveTrainedData()
{
    std::cout << "Saving dictionary to file nodeMap.txt" << std::endl;
    utils::SaveToFile(Global::nodeMap, "nodeMap.txt");
}

void TrainerManager::LoadTrainedData()
{
    if (!utils::FileExists("nodeMap.txt"))
        return;
    std::cout << "Loading nodes from file nodeMap.txt..." << std::endl;
    utils::LoadFromFile(Global::nodeMap, "nodeMap.txt");
}
