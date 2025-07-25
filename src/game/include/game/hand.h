#ifndef __CLASS_HAND_H__
#define __CLASS_HAND_H__

#include "game/card.h"
#include "game/hand_strength.h"
#include <algorithm>
#include <iostream>
#include <map>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

typedef unsigned long ulong;

using namespace std;

namespace poker {
class HandStrength;
class Hand {
public:
  vector<Card> cards;

  Hand();
  Hand(ulong bitmap);
  Hand(vector<string> &cards);

  HandStrength GetStrength();
  ulong Bitmap();
  void PrintColoredCards(const string &end = string(""));
  string ToString();

  bool operator==(const Hand &other) const;
};
} // namespace poker

#endif
