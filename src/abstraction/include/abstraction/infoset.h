#ifndef __CLASS_INFOSET_H__
#define __CLASS_INFOSET_H__

#include "enums/betting_round.h"
#include "utils/random.h"
#include <cereal/archives/binary.hpp>
#include <string>
#include <vector>

using namespace std;

namespace poker {
class Infoset {
public:
  vector<int> regret;
  vector<int> actionCounter;

  Infoset();
  Infoset(int actions);
  Infoset(int actions, BettingRound round);

  vector<float> CalculateStrategy();
  vector<float> GetFinalStrategy();
  int SampleAction();
  int SampleAction(bool final);
};
} // namespace poker
#endif
