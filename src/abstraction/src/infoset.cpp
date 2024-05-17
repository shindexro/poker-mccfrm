#include "abstraction/infoset.h"

namespace poker {
Infoset::Infoset() : regret(), actionCounter() {}

Infoset::Infoset(int actions) : regret(actions), actionCounter(actions) {}

Infoset::Infoset(int actions, BettingRound round)
    : regret(actions), actionCounter() {
  if (round == BettingRound::Preflop) {
    actionCounter.resize(actions);
  }
}

vector<float> Infoset::CalculateStrategy() {
  int sum = 0;
  auto moveProbs = vector<float>(regret.size());
  for (auto a = 0UL; a < regret.size(); ++a) {
    sum += max(0, regret[a]);
  }
  for (auto a = 0UL; a < regret.size(); ++a) {
    if (sum > 0.00001) {
      moveProbs[a] = (float)max(0, regret[a]) / sum;
    } else {
      moveProbs[a] = 1.0f / regret.size();
    }
  }
  return moveProbs;
}

vector<float> Infoset::GetFinalStrategy() {
  int sum = 0;
  auto moveProbs = vector<float>(regret.size());
  for (auto a = 0UL; a < regret.size(); ++a) {
    sum += actionCounter[a];
  }
  for (auto a = 0UL; a < regret.size(); ++a) {
    if (sum > 0.00001) {
      moveProbs[a] = (float)actionCounter[a] / sum;
    } else {
      moveProbs[a] = 1.0f / regret.size();
    }
  }
  return moveProbs;
}

int Infoset::SampleAction() { return SampleAction(false); }

int Infoset::SampleAction(bool final) {
  auto sigma = final ? GetFinalStrategy() : CalculateStrategy();
  auto actionIdx = SampleDistribution(sigma);
  return actionIdx;
}
} // namespace poker
