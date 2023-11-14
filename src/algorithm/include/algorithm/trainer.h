#ifndef __CLASS_TRAINER_H__
#define __CLASS_TRAINER_H__

#include "abstraction/state.h"
#include "abstraction/play_state.h"
#include "abstraction/chance_state.h"
#include "abstraction/terminal_state.h"
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
    int TraverseMCCFR(int traverser, bool pruned);
    void PlayOneGame();

    void DiscountInfosets(float d);

    void PrintStartingHandsChart();
    void PrintStatistics(long iterations);
    void EnumerateActionSpace(shared_ptr<State> gs);
    void EnumerateActionSpace();

private:
    int threadIndex = 0;

    int TraverseMCCFR(shared_ptr<State> gs, int traverser, bool pruned);
};

#endif
