#ifndef __CLASS_EVALUATOR_H__
#define __CLASS_EVALUATOR_H__

#include "game/deck.h"
#include "game/hand.h"
#include "game/hand_strength.h"
#include <string>
#include <iostream>
#include <chrono>
#include <unordered_map>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include <algorithm>

using namespace std;

class Evaluator
{
public:
    Evaluator();

    int Evaluate(ulong bitmap);
    void SaveToFile(string &filename);

private:
    bool loaded;
    unordered_map<ulong, ulong> handRankMap;
    unordered_map<ulong, ulong> monteCarloMap;

    void LoadFromFile(string &filename);
    void GenerateFiveCardTable();
    void GenerateSixCardTable();
    void GenerateSevenCardTable();
    void GenerateMonteCarloMap(int iterations);
};

template <class RandIt, class Compare>
bool next_combination(RandIt first, RandIt mid, RandIt last, Compare comp);

#endif
