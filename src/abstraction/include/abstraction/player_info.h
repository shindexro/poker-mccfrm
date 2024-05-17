#ifndef __CLASS_PLAYER_INFO_H__
#define __CLASS_PLAYER_INFO_H__

#include "enums/action.h"
#include "game/hand.h"

#include <iostream>
#include <tuple>

typedef unsigned long ulong;

using namespace std;

namespace poker {
class PlayerInfo {
public:
  int stack;
  int bet;
  int reward;
  tuple<ulong, ulong> cards;
  Action lastAction;

  PlayerInfo();

  ulong GetCardBitmask() const;

  bool IsAlive() const;

  ostream &PrettyPrint(ostream &out) const;
  friend ostream &operator<<(ostream &out, const PlayerInfo &info);
};
} // namespace poker

#endif
