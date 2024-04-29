#ifndef __CLASS_TRAINER_MANAGER_H__
#define __CLASS_TRAINER_MANAGER_H__

#include "algorithm/trainer.h"
#include "utils/utils.h"

using namespace std;

namespace poker
{
    class TrainerManager
    {
    public:
        const int threadCount;
        atomic<long> iterations;
        vector<Trainer> trainers;

        TrainerManager();
        TrainerManager(int threadCount);

        void StartTraining();
        void SaveTrainedData();
        void LoadTrainedData();

    private:
        static const int CountdownInterval = 10000;

        static const long StrategyInterval = 10000;         // bb rounds before updating player strategy (recursive through tree) 10k
        static const long LCFRThreshold = 60000000;         // bb rounds when to stop discounting regrets, 400 minutes
        static const long DiscountInterval = 1500000;       // bb rounds, discount values periodically but not every round, 10 minutes
        static const long SaveToDiskInterval = 5000000L;
        static const long TestGamesInterval = 1000000;
        static const long PruneThreshold = 20000000;  // bb rounds after this time we stop checking all actions, 200 minutes

        atomic<long> StrategyIntervalCountdown;
        atomic<long> DiscountIntervalCountdown;
        atomic<long> SaveToDiskIntervalCountdown;
        atomic<long> TestGamesIntervalCountdown;

        void StartTrainer(int index);
        void RunSingleThreadTasks(int index, int current_iterations);
    };
} // namespace poker

#endif
