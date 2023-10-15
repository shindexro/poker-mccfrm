#ifndef __CLASS_GLOBAL_H__
#define __CLASS_GLOBAL_H__

#include "tables/hand_indexer.h"
#include "tables/evaluator.h"
#include <vector>

using namespace std;

class Global
{
public:
    const int NOF_THREADS = 12;
    static vector<float> raises;
    const int buyIn = 200;
    const int nofPlayers = 200;
    const int C = -100000;
    const int regretFloor = -110000;

    const int BB = 2;
    const int SB = 1;

    // information abstraction parameters, currently this would be a
    // 169 - 200 - 200 - 200 abstraction, where the river is bucketed using OCHS and the turn and flop using EMD
    const int nofRiverBuckets = 1000;
    const int nofTurnBuckets = 1000;
    const int nofFlopBuckets = 1000;
    // 100k or even 1 million shouldn't take too much time compared to the rest of the information abstraction
    const int nofMCSimsPerPreflopHand = 500;
    // for the river, determines the river histogram size (in theory could be up to 169 but will be very slow) default 8
    const int nofOpponentClusters = 16;
    const int flopHistogramSize = 50;
    const int turnHistogramSize = 50;

    // this is used to create the nofOpponentClusters, it can be increased (default 50)
    // with little time penalty because the clustering for 169 hands is very fast
    const int preflopHistogramSize = 100;

    // dont change
    static HandIndexer indexer_2;
    static HandIndexer indexer_2_3;
    static HandIndexer indexer_2_4;
    static HandIndexer indexer_2_5;
    static HandIndexer indexer_2_3_1;
    static HandIndexer indexer_2_3_1_1;
    static HandIndexer indexer_2_5_2;

    static Evaluator handEvaluator;

    // static ConcurrentDictionary<string, Infoset> nodeMap = new ConcurrentDictionary<string, Infoset>(Global.NOF_THREADS, 1000000);
    // static ConcurrentDictionary<string, Infoset> nodeMapBaseline = new ConcurrentDictionary<string, Infoset>(Global.NOF_THREADS, 1000000);

    // static ThreadLocal<Deck> Deck = new ThreadLocal<Deck>(() = > new Deck());
};

#endif
