#ifndef __CLASS_CARD_H__
#define __CLASS_CARD_H__

#include <string>
#include <stdexcept>
#include <iostream>
#include <cmath>

#include "enums/rank.h"
#include "enums/suit.h"

const int rankPrimes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41};
const int suitPrimes[] = {43, 47, 53, 59};

using namespace std;

namespace poker
{
    class Card
    {
    public:
        Rank rank;
        Suit suit;

        Card(const char *s);
        Card(const string &s);
        Card(int index);
        Card(ulong bitmap);

        bool Equals(const Card &other) const;
        int PrimeRank() const;
        int PrimeSuit() const;
        int HashCode(Card &c);
        int Index();
        ulong Bitmask();

        bool operator==(const Card &other) const;
        bool operator<(const Card &rhs) const;
        friend ostream &operator<<(ostream &out, const Card &card);

        static int GetIndexFromBitmask(ulong bitmask);

        string ToString() const;
        string PrettyString(const string_view &end = "") const;
    };
} // namespace poker
#endif
