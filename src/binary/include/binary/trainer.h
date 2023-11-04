#ifndef __CLASS_TRAINER_H__
#define __CLASS_TRAINER_H__

#include "binary/state.h"

#include <vector>
#include <string>

using namespace std;

class Trainer
{
public:
    State rootState;

    Trainer(int threadIndex);

    void ResetGame();
    void UpdateStrategy(State gs, int traverser);
    void UpdateStrategy(int traverser);
    float TraverseMCCFRPruned(int traverser);
    float TraverseMCCFR(int traverser, int iteration);
    void PlayOneGame();
    float PlayOneGame_d(int mainPlayer, bool display);

    void TraverseMCCFRPruned();
    void DiscountInfosets(float d);

    void PrintStartingHandsChart();
    void PrintStatistics(long iterations);
    void EnumerateActionSpace(State gs);
    void EnumerateActionSpace();

private:
    int threadIndex = 0;

    float TraverseMCCFRPruned(State gs, int traverser);
    float TraverseMCCFR(State gs, int traverser, int iteration);
};

#endif
