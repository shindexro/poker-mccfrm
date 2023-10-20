#ifndef __CLASS_HAND_H__
#define __CLASS_HAND_H__

#include "game/card.h"
#include "game/hand_strength.h"
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <map>
#include <stdexcept>

using namespace std;

class HandStrength;

class Hand
{
public:
    vector<Card> cards;

    Hand();
    Hand(ulong bitmap);

    HandStrength GetStrength();
    void PrintColoredCards(const string &end = string(""));
    string ToString();
};

#endif
