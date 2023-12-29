#include "abstraction/global.h"

namespace poker
{
    // ratios must be sorted in ascending order
    map<BettingRound, vector<float>> Global::raiseRatiosByRound{
            {BettingRound::Preflop, {0.5f, 1.0f, 2.0f}},
            {BettingRound::Flop, {0.5f, 1.0f, 1.5f, 2.0f}},
            {BettingRound::Turn, {0.5f, 1.0f, 1.5f, 2.0f}},
            {BettingRound::River, {0.5f, 1.0f, 1.5f, 2.0f}},
    };

    HandIndexer Global::indexer_2;
    HandIndexer Global::indexer_2_3;
    HandIndexer Global::indexer_2_4;
    HandIndexer Global::indexer_2_5;
    HandIndexer Global::indexer_2_3_1;
    HandIndexer Global::indexer_2_3_1_1;
    HandIndexer Global::indexer_2_5_2;

    shared_ptr<Evaluator> Global::handEvaluator = make_shared<Evaluator>();

    oneapi::tbb::concurrent_hash_map<string, Infoset> Global::nodeMap = oneapi::tbb::concurrent_hash_map<string, Infoset>();
    oneapi::tbb::concurrent_hash_map<string, Infoset> Global::nodeMapBaseline = oneapi::tbb::concurrent_hash_map<string, Infoset>();

    thread_local Deck Global::deck = Deck(CARDS);
} // namespace poker
