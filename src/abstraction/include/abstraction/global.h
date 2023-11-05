#ifndef __CLASS_GLOBAL_H__
#define __CLASS_GLOBAL_H__

#include "tables/hand_indexer.h"
#include "tables/evaluator.h"
#include "abstraction/infoset.h"
#include "game/deck.h"

#include <oneapi/tbb/concurrent_hash_map.h>
#include <vector>

using namespace std;

class HandIndexer;
class Evaluator;
class Deck;

class Global
{
public:
    static const int NOF_THREADS = 4;
    static vector<float> raises;
    static const int buyIn = 200;
    static const int nofPlayers = 200;
    static const int C = -100000;
    static const int regretFloor = -110000;

    static const int BB = 2;
    static const int SB = 1;

    static const int RANKS = 5;
    static const int SUITS = 4;
    static const int CARDS = RANKS * SUITS;

    // information abstraction parameters, currently this would be a
    // 169 - 200 - 200 - 200 abstraction, where the river is bucketed using OCHS and the turn and flop using EMD
    static const int nofRiverBuckets = 50;
    static const int nofTurnBuckets = 50;
    static const int nofFlopBuckets = 50;
    // 100k or even 1 million shouldn't take too much time compared to the rest of the information abstraction
    static const int nofMCSimsPerPreflopHand = 5;
    // for the river, determines the river histogram size (in theory could be up to 169 but will be very slow) default 8
    static const int nofOpponentClusters = 16;
    static const int flopHistogramSize = 50;
    static const int turnHistogramSize = 50;

    // this is used to create the nofOpponentClusters, it can be increased (default 50)
    // with little time penalty because the clustering for 169 hands is very fast
    static const int preflopHistogramSize = 100;

    // dont change
    static HandIndexer indexer_2;
    static HandIndexer indexer_2_3;
    static HandIndexer indexer_2_4;
    static HandIndexer indexer_2_5;
    static HandIndexer indexer_2_3_1;
    static HandIndexer indexer_2_3_1_1;
    static HandIndexer indexer_2_5_2;

    static Evaluator handEvaluator;

    static oneapi::tbb::concurrent_hash_map<string, Infoset> nodeMap;
    static oneapi::tbb::concurrent_hash_map<string, Infoset> nodeMapBaseline;

    thread_local static Deck deck;
};

#endif
