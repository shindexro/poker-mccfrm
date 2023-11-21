#ifndef __CLASS_DECK_H__
#define __CLASS_DECK_H__

#include "utils/random.h"
#include "abstraction/global.h"
#include "enums/betting_round.h"

#include <string>

using namespace std;
namespace poker
{
    class Deck
    {
    public:
        int NumRemainingCards();
        Deck(ulong removedCards = 0);
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
