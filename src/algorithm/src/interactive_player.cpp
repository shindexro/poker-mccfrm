#include "algorithm/interactive_player.h"

namespace poker {
InteractivePlayer::InteractivePlayer(int id, int stack) : Player(id, stack) {}

Action InteractivePlayer::NextAction(shared_ptr<PlayState> state) {
  auto validActions = state->GetValidActions();
  std::cout << "No. of valid actions: " << validActions.size() << std::endl;

  std::cout << "Enter a integer bet size or fold (-1): ";
  int bet;
  std::cin >> bet;

  if (bet == -1) {
    return Action::Fold;
  }

  auto action = TranslateAction(state, bet);
  std::cout << "Mapped to abstraction as " << action << std::endl;
  return action;
}

Action InteractivePlayer::NextAction(shared_ptr<PlayState> state,
                                     shared_ptr<PlayState>) {
  return NextAction(state);
}

Action InteractivePlayer::TranslateAction(shared_ptr<PlayState> state,
                                          int actualBet) {
  auto abstractActions = state->GetValidActions();
  auto abstractBets = state->GetValidBetSizes();

  if (abstractActions.back() == Action::Fold) {
    abstractActions.pop_back();
    abstractBets.pop_back();
  }

  if (!(abstractBets[0] <= actualBet && actualBet <= abstractBets.back())) {
    throw invalid_argument("Bet size not in allowed range.");
  }

  auto it = lower_bound(abstractBets.begin(), abstractBets.end(), actualBet);
  auto idx = it - abstractBets.begin();
  if (*it == actualBet) {
    return abstractActions[idx];
  }

  float lowBet = abstractBets[idx - 1];
  float highBet = abstractBets[idx];

  // random pseudo-harmonic action translation
  float lowBetProb = (highBet - actualBet) * (1 + lowBet) / (highBet - lowBet) /
                     (1 + actualBet);
  if (randDouble() < lowBetProb) {
    return abstractActions[idx - 1];
  } else {
    return abstractActions[idx];
  }
}
} // namespace poker
