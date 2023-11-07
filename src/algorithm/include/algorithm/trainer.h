#ifndef __CLASS_TRAINER_H__
#define __CLASS_TRAINER_H__

#include "abstraction/state.h"
#include "utils/utils.h"
#include "enums/action.h"

#include <vector>
#include <string>
#include <fstream>
#include <regex>

using namespace std;
using namespace poker;

class Trainer
{
public:
    shared_ptr<State> rootState;

    Trainer(int threadIndex);

    void ResetGame();
    void UpdateStrategy(shared_ptr<State> gs, int traverser);
    void UpdateStrategy(int traverser);
    float TraverseMCCFRPruned(int traverser);
    float TraverseMCCFR(int traverser, int iteration);
    void PlayOneGame();
    float PlayOneGame_d(int mainPlayer, bool display);

    void TraverseMCCFRPruned();
    void DiscountInfosets(float d);

    void PrintStartingHandsChart();
    void PrintStatistics(long iterations);
    void EnumerateActionSpace(shared_ptr<State> gs);
    void EnumerateActionSpace();

private:
    int threadIndex = 0;

    float TraverseMCCFRPruned(shared_ptr<State> gs, int traverser);
    float TraverseMCCFR(shared_ptr<State> gs, int traverser, int iteration);
};

#endif
