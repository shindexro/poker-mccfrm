#ifndef __CLASS_HAND_H__
#define __CLASS_HAND_H__

#include "game/card.h"
#include <string>
#include <vector>
#include <iostream>

using namespace std;

enum HandRanking
{
    HighCard,
    Pair,
    TwoPair,
    ThreeOfAKind,
    Straight,
    Flush,
    FullHouse,
    FourOfAKind,
    StraightFlush,
};

class Hand
{
public:
    vector<Card> cards;

    Hand();
    Hand(ulong bitmap);
    void PrintColoredCards(string &end);
    string ToString();
};

#endif
