#ifndef __CLASS_COMMUNITY_INFO_H__
#define __CLASS_COMMUNITY_INFO_H__

#include "abstraction/global.h"
#include "enums/betting_round.h"

#include <string>

typedef unsigned long ulong;

using namespace std;

namespace poker {
class CommunityInfo {
public:
  BettingRound bettingRound;
  int playerToMove;
  int lastPlayer;
  int minRaise;
  vector<ulong> cards;

  CommunityInfo();

  ulong GetCardBitmask() const;
};

ostream &operator<<(ostream &out, const CommunityInfo &info);

} // namespace poker

#endif
