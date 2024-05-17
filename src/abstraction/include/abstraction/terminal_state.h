#ifndef __CLASS_TERMINAL_STATE_H__
#define __CLASS_TERMINAL_STATE_H__

#include "abstraction/state.h"
#include "tables/hand_indexer.h"

using namespace std;

namespace poker {
class TerminalState : public State {
public:
  static const string type;
  shared_ptr<Evaluator> evaluator;

  TerminalState();
  TerminalState(CommunityInfo &community, vector<PlayerInfo> &players,
                vector<Action> &history);

  float GetReward(int player) override;
  void CreateChildren() override;

  ostream &Print(ostream &out) const override;

private:
  bool rewardGenerated;

  void CreateRewards();
};
} // namespace poker

#endif
