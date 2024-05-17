#ifndef __CLASS_GLOBAL_H__
#define __CLASS_GLOBAL_H__

#include "tables/hand_indexer.h"
#include "tables/evaluator.h"
#include "abstraction/infoset.h"
#include "game/deck.h"
#include "enums/betting_round.h"

#include "parallel_hashmap/phmap.h"
#include <mutex>

#include <oneapi/tbb/concurrent_hash_map.h>
#include <vector>
#include <thread>

using namespace std;

class HandIndexer;
class Evaluator;


namespace poker
{
    typedef phmap::parallel_flat_hash_map<string, Infoset,
            phmap::priv::hash_default_hash<string>,
            phmap::priv::hash_default_eq<string>,
            std::allocator<std::pair<const string, Infoset>>, 12, phmap::AbslMutex> NodeMap;

    class Global
    {
    public:
        inline static const int NOF_THREADS = std::thread::hardware_concurrency();;
        static const map<BettingRound, vector<vector<float>>> raiseRatiosByRoundByPlayerCount;
        inline static const int buyIn = 10000;
        inline static const int nofPlayers = 6;
        inline static const int regretPrunedThreshold = -300000000;
        inline static const int regretFloor = -310000000;

        inline static const int BB = 100;
        inline static const int SB = 50;
        inline static const float rakePercent = 0.02;

        inline static const int RANKS = 13;
        inline static const int SUITS = 4;
        inline static const int CARDS = RANKS * SUITS;

        // information abstraction parameters, currently this would be a
        // 169 - 200 - 200 - 200 abstraction, where the river is bucketed using OCHS and the turn and flop using EMD
        inline static const int nofRiverBuckets = 200;
        inline static const int nofTurnBuckets = 200;
        inline static const int nofFlopBuckets = 200;
        // 100k or even 1 million shouldn't take too much time compared to the rest of the information abstraction
        inline static const int nofMCSimsPerPreflopHand = 1000000;
        // for the river, determines the river histogram size (in theory could be up to 169 but will be very slow) default 8
        inline static const int nofOpponentClusters = 16;
        // inline static const int flopHistogramSize = 50;
        // inline static const int turnHistogramSize = 50;

        // this is used to create the nofOpponentClusters, it can be increased (default 50)
        // with little time penalty because the clustering for 169 hands is very fast
        inline static const int preflopHistogramSize = 50;

        // dont change
        static HandIndexer indexer_2;
        static HandIndexer indexer_2_3;
        static HandIndexer indexer_2_4;
        static HandIndexer indexer_2_5;
        static HandIndexer indexer_2_3_1;
        static HandIndexer indexer_2_3_1_1;
        static HandIndexer indexer_2_5_2;

        static shared_ptr<Evaluator> handEvaluator;

        static NodeMap nodeMap;

        static thread_local Deck deck;
    };
} // namespace poker
#endif
