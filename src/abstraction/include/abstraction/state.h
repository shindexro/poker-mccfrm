#ifndef __CLASS_STATE_H__
#define __CLASS_STATE_H__

#include "abstraction/community_info.h"
#include "abstraction/global.h"
#include "abstraction/infoset.h"
#include "abstraction/player_info.h"
#include "enums/action.h"
#include "enums/betting_round.h"
#include "tables/emd_table.h"
#include "tables/ochs_table.h"
#include "utils/utils.h"

#include <boost/algorithm/string.hpp>
#include <oneapi/tbb/concurrent_hash_map.h>
#include <string>
#include <vector>

using namespace std;

namespace poker {
class State {
public:
  CommunityInfo community;
  vector<PlayerInfo> players;
  vector<Action> history;
  vector<shared_ptr<State>> children;
  string infosetString;

  State();
  State(CommunityInfo &community, vector<PlayerInfo> &players,
        vector<Action> &history);

  int NextActivePlayer();
  int PrevActivePlayer();
  int FirstActivePlayer();

  int GetNumberOfPlayersThatNeedToAct();
  int GetPot() const;
  int MinimumCall() const;

  int GetNumberOfActivePlayers();
  int GetNumberOfAllInPlayers();
  virtual void CreateChildren() { throw invalid_argument("Not implemented"); };
  virtual int GetValidActionsCount() { throw invalid_argument("Not implemented"); };
  virtual bool IsPlayerInHand(int /*traverser*/) {
    throw invalid_argument("Not implemented");
  };

  virtual string StringId() { throw invalid_argument("Not implemented"); };

  virtual bool IsPlayerTurn(int /*traverser*/) {
    throw invalid_argument("Not implemented");
  };
  int BettingRound();
  virtual shared_ptr<State> DoRandomAction() {
    throw invalid_argument("Not implemented");
  };
  virtual float GetReward(int /*traverser*/) {
    throw invalid_argument("Not implemented");
  };

  virtual ostream &Print(ostream &out) const;
  ostream &PrettyPrint(ostream &out) const;
  void PrettyPrintTree(int depth = 0);
};

ostream &operator<<(ostream &out, const State &state);
} // namespace poker

#endif
