#ifndef __CLASS_DECK_H__
#define __CLASS_DECK_H__

#include "utils/random.h"
#include "abstraction/global.h"
#include "enums/betting_round.h"

#include <string>

using namespace std;

class Deck
{
public:
    int NumRemainingCards();
    Deck(ulong removedCards = 0);
    void Shuffle();
    ulong Draw(int count);
    ulong Peek(int idx);

private:
    vector<ulong> cards;
    ulong removedCards;
    int numRemovedCards;
    int position;
};

#endif
