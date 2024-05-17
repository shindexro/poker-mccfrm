#ifndef __CLASS_TRAINER_H__
#define __CLASS_TRAINER_H__

#include "abstraction/chance_state.h"
#include "abstraction/play_state.h"
#include "abstraction/state.h"
#include "abstraction/terminal_state.h"
#include "enums/action.h"
#include "enums/betting_round.h"
#include "utils/random.h"
#include "utils/utils.h"

#include <fstream>
#include <regex>
#include <string>
#include <vector>

using namespace std;

namespace poker {
class Trainer {
public:
  shared_ptr<State> rootState;

  Trainer();

  void ResetGame();
  void TrainOneIteration(int traverser, bool pruneEnabled);
  void UpdateStrategy(shared_ptr<State> gs, int traverser);
  void UpdateStrategy(int traverser);
  int TraverseMCCFR(int traverser, bool pruned);
  int TraverseMCCFR(shared_ptr<State> gs, int traverser, bool pruned);

  void DiscountInfosets(float d);
  void UpdateInfoset(shared_ptr<State> state, Infoset &infoset);
  Infoset GetInfoset(shared_ptr<State> state);

  void PrintStartingHandsChart();
  void PrintStatistics(long iterations);
  void EnumerateActionSpace(shared_ptr<State> gs);
  void EnumerateActionSpace();

private:
  NodeMap nodeMapBuffer;
};
} // namespace poker

#endif
