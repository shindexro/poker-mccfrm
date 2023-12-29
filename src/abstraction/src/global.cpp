#include "abstraction/global.h"

namespace poker
{
    // ratios must be sorted in ascending order
    map<BettingRound, vector<vector<float>>> Global::raiseRatiosByRoundByPlayerCount{
            {BettingRound::Preflop, {
                                        {},
                                        {},
                                        {1.0f, 1.5f, 2.0f}, // 2 players to act
                                        {1.0f, 1.5f, 2.0f}, // 3 players to act
                                        {1.0f, 1.5f, 2.0f}, // 4 players to act
                                        {1.0f, 1.5f, 2.0f}, // 5 players to act
                                        {1.0f, 1.5f, 2.0f}, // 6 players to act
                                    }},
            {BettingRound::Flop, {
                                        {},
                                        {},
                                        {0.33f, 0.67f, 1.0f, 1.5f, 2.0f}, // 2 players to act
                                        {0.33f, 0.67f, 1.0f, 1.5f, 2.0f}, // 3 players to act
                                        {0.5f}, // 4 players to act
                                        {0.5f}, // 5 players to act
                                        {0.5f}, // 6 players to act
                                    }},
            {BettingRound::Turn, {
                                        {},
                                        {},
                                        {0.33f, 0.67f, 1.0f, 1.5f, 2.0f}, // 2 players to act
                                        {0.33f, 0.67f, 1.0f, 1.5f, 2.0f}, // 3 players to act
                                        {0.5f}, // 4 players to act
                                        {0.5f}, // 5 players to act
                                        {0.5f}, // 6 players to act
                                    }},
            {BettingRound::River, {
                                        {},
                                        {},
                                        {0.33f, 0.67f, 1.0f, 1.5f, 2.0f}, // 2 players to act
                                        {0.33f, 0.67f, 1.0f, 1.5f, 2.0f}, // 3 players to act
                                        {0.5f}, // 4 players to act
                                        {0.5f}, // 5 players to act
                                        {0.5f}, // 6 players to act
                                    }},
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
