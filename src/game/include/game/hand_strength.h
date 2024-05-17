#ifndef __CLASS_HAND_STRENGTH_H__
#define __CLASS_HAND_STRENGTH_H__

#include "enums/hand_ranking.h"
#include "enums/rank.h"
#include "game/hand.h"
#include <string>
#include <vector>

using namespace std;

namespace poker {
class HandStrength {
public:
  HandRanking handRanking;
  vector<Rank> kickers;

  HandStrength();

  int Compare(const HandStrength &other) const;
  bool operator<(const HandStrength &rhs) const;
  bool operator==(const HandStrength &rhs) const;

  friend ostream &operator<<(ostream &out, const HandStrength &strength);
};
} // namespace poker

#endif
