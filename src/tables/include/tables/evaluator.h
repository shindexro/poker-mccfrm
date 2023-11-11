#ifndef __CLASS_EVALUATOR_H__
#define __CLASS_EVALUATOR_H__

#include "game/deck.h"
#include "game/hand.h"
#include "game/hand_strength.h"
#include "abstraction/global.h"

#include <string>
#include <iostream>
#include <chrono>
#include <unordered_map>
#include <fstream>
#include <sys/stat.h>
#ifdef __linux__ 
#include <unistd.h>
#endif
#include <algorithm>

#include <indicators/cursor_control.hpp>
#include <indicators/progress_bar.hpp>
#include <indicators/block_progress_bar.hpp>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/list.hpp>


using namespace std;

class Evaluator
{
public:
    void Initialise();

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
    void GenerateHandValueTable(int comboSize);
    void GenerateMonteCarloMap(int iterations);
};

template <typename Iterator>
inline bool next_combination(const Iterator first, Iterator k, const Iterator last);

#endif
