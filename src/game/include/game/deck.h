#ifndef __CLASS_DECK_H__
#define __CLASS_DECK_H__

using namespace std;

class Deck
{
public:
    int NumRemainingCards();
    Deck(ulong removedCards);
    void Shuffle(int from);
    ulong Draw_(int count);
    ulong Draw(int pos);

private:
    ulong cards[52];
    ulong removedCards;
    int position;
};

#endif
