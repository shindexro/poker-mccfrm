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
    int rounds;                // no. of rounds
    vector<int> cardsPerRound; // no. of cards dealt at i-th round
    vector<int> configurations;
    vector<int> permutations;
    vector<long> roundSize; // no. of non-isomorphic hands at i-th round

    HandIndexer();
    void Construct(vector<int> &cardsPerRound);
    static void Initialise();

    long IndexAllRounds(vector<int> &cards, vector<long> &indices);
    long IndexLastRound(vector<int> &cards);
    long IndexNextRound(HandIndexerState &state, vector<int> &cards);
    bool Unindex(int round, long index, vector<int> &cards);

private:
    const static int MAX_GROUP_INDEX = 0x1000000;
    const static int ROUND_SHIFT = 4;
    const static int ROUND_MASK = 0xf;

    static vector<vector<int>> nthUnset;
    static vector<vector<bool>> equal;
    static vector<vector<int>> nCrRanks; // cache of nCr calculation
    static vector<int> rankSetToIndex;
    static vector<vector<int>> indexToRankSet;
    static vector<vector<int>> suitPermutations;
    static vector<vector<long>> nCrGroups; // cache of nCr calculation

    vector<int> roundStart;
    vector<vector<int>> permutationToConfiguration;
    vector<vector<int>> permutationToPi;
    vector<vector<int>> configurationToEqual;
    vector<vector<vector<int>>> configuration;
    vector<vector<vector<int>>> configurationToSuitSize;
    vector<vector<long>> configurationToOffset;
    vector<vector<int>> publicFlopHands; // map idx (from IndexLast) to canonical flop cards

    static void CacheNCRCalculation();

    void Swap(vector<int> &suitIndex, int u, int v);

    void CreatePublicFlopHands();
    void EnumerateConfigurations(bool tabulate);
    void EnumerateConfigurationsR(int round, int remaining, int suit, int equal, vector<int> &used, vector<int> &configuration, bool tabulate);
    void TabulateConfigurations(int round, vector<int> &configuration);

    void EnumeratePermutations(bool tabulate);
    void EnumeratePermutationsR(int round, int remaining, int suit, vector<int> &used, vector<int> &count, bool tabulate);
    void TabulatePermutations(int round, vector<int> &count);
    void CountPermutations(int round, vector<int> &count);
};

#endif
