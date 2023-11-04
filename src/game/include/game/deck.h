#ifndef __CLASS_DECK_H__
#define __CLASS_DECK_H__

#include "utils/random.h"
#include "abstraction/global.h"

#include <string>

using namespace std;

class Deck
{
public:
    int NumRemainingCards();
    Deck(ulong removedCards = 0);
    void Shuffle(int from = 0);
    ulong Draw_(int count);
    ulong Draw(int pos);

private:
    vector<ulong> cards;
    ulong removedCards;
    int position;
};

#endif
