#ifndef __CLASS_CARD_H__
#define __CLASS_CARD_H__

#include <string>
#include <stdexcept>
#include "enums/rank.h"
#include "enums/suit.h"

const int rankPrimes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41};
const int suitPrimes[] = {43, 47, 53, 59};

using namespace std;

class Card
{
public:
    Rank rank;
    Suit suit;

    Card(const char *s);
    Card(int index);
    bool Equals(const Card &other);
    int PrimeRank();
    int PrimeSuit();
    int HashCode(Card &c);
    int Index();
    string ToString();
};

#endif
