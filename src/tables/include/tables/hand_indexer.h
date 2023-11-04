#ifndef __CLASS_HAND_INDEXER_H__
#define __CLASS_HAND_INDEXER_H__

#include "abstraction/global.h"

#include <vector>
#include <stdexcept>
#include <iostream>
#include <cmath>

using namespace std;

class HandIndexerState
{

public:
    vector<int> suitIndex;
    vector<int> suitMultiplier;
    int round;
    int permutationIndex;
    int permutationMultiplier;
    vector<int> usedRanks;

    HandIndexerState();
};

class HandIndexer
{
public:
    int rounds;
    vector<int> cardsPerRound;
    vector<int> configurations;
    vector<int> permutations;
    vector<long> roundSize;

    HandIndexer();
    void Construct(vector<int> &cardsPerRound);
    static void Initialise();

    void CreatePublicFlopHands();
    long IndexAll(vector<int> &cards, vector<long> &indices);
    long IndexLast(vector<int> &cards);
    long IndexNextRound(HandIndexerState &state, vector<int> &cards);
    bool Unindex(int round, long index, vector<int> &cards);

private:
    const static int MAX_GROUP_INDEX = 0x1000000;
    const static int ROUND_SHIFT = 4;
    const static int ROUND_MASK = 0xf;

    static int nthUnset[1 << RANKS][RANKS];
    static bool equal[1 << (SUITS - 1)][SUITS];
    static int nCrRanks[RANKS + 1][RANKS + 1];
    static int rankSetToIndex[1 << RANKS];
    static int indexToRankSet[RANKS + 1][1 << RANKS];
    static vector<vector<int>> suitPermutations;
    static long nCrGroups[MAX_GROUP_INDEX][SUITS + 1];

    // static vector<vector<int>> nthUnset;
    // static vector<vector<bool>> equal;
    // static vector<vector<int>> nCrRanks;
    // static vector<int> rankSetToIndex;
    // static vector<vector<int>> indexToRankSet;
    // static vector<vector<int>> suitPermutations;
    // static vector<vector<long>> nCrGroups;

    vector<int> roundStart;
    vector<vector<int>> permutationToConfiguration;
    vector<vector<int>> permutationToPi;
    vector<vector<int>> configurationToEqual;
    vector<vector<vector<int>>> configuration;
    vector<vector<vector<int>>> configurationToSuitSize;
    vector<vector<long>> configurationToOffset;
    vector<vector<int>> publicFlopHands;

    void Swap(vector<int> &suitIndex, int u, int v);

    void EnumerateConfigurations(bool tabulate);
    void EnumerateConfigurationsR(int round, int remaining, int suit, int equal, vector<int> &used, vector<int> &configuration, bool tabulate);
    void TabulateConfigurations(int round, vector<int> &configuration);

    void EnumeratePermutations(bool tabulate);
    void EnumeratePermutationsR(int round, int remaining, int suit, vector<int> &used, vector<int> &count, bool tabulate);
    void TabulatePermutations(int round, vector<int> &count);
    void CountPermutations(int round, vector<int> &count);
};

#endif
