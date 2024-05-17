#include "algorithm/ai_player.h"

namespace poker {
AIPlayer::AIPlayer(int id, int stack) : Player(id, stack) {}

Action AIPlayer::NextAction(shared_ptr<PlayState> state,
                            shared_ptr<PlayState> roundStartState) {
  auto validActions = state->GetValidActions();
  if (!validActions.size()) {
    throw invalid_argument("There are no valid actions.");
  }

  auto trainer = Trainer();
  if (state->BettingRound() == BettingRound::Preflop) {
    // use average strategy
    auto infoset = trainer.GetInfoset(state);
    return validActions[infoset.SampleAction(true)];
  }

  // use real-time search based on blue-print strategy
  /// TOOD: make number for iterations more dynamic?
  for (int i = 0; i < 10000; i++) {
    trainer.TraverseMCCFR(roundStartState, id, false);
  }

  auto infoset = trainer.GetInfoset(state);
  return validActions[infoset.SampleAction()];
}
} // namespace poker
