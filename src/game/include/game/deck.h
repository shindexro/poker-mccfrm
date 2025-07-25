#ifndef __CLASS_DECK_H__
#define __CLASS_DECK_H__

#include "enums/betting_round.h"
#include "utils/random.h"

#include <algorithm>
#include <stdexcept>
#include <string>

typedef unsigned long ulong;

using namespace std;
namespace poker {
class Deck {
public:
  int NumRemainingCards();
  Deck(int size, ulong removedCards = 0);
  void Shuffle();
  ulong Draw(int count);
  ulong Peek(int idx);

private:
  ulong removedCards;
  int numRemovedCards;
  size_t position;
  vector<ulong> cards;
};
} // namespace poker
#endif
