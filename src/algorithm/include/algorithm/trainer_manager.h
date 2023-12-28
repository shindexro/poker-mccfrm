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

    private:
        void StartTrainer(int index);

        void SaveTrainedData();
        void LoadTrainedData();

    };
} // namespace poker

#endif
