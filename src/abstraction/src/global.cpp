#include "abstraction/global.h"

namespace poker
{
    // ratios must be sorted in ascending order
    const map<BettingRound, vector<vector<float>>> Global::raiseRatiosByRoundByPlayerCount{
            {BettingRound::Preflop, {
                                        {}, // 0 players to act
                                        {2.0f}, // 1 players to act
                                        {2.0f}, // 2 players to act
                                        {2.0f}, // 3 players to act
                                        {2.0f}, // 4 players to act
                                        {2.0f}, // 5 players to act
                                        {2.0f}, // 6 players to act
                                    }},
            {BettingRound::Flop, {
                                        {},
                                        {0.3f, 0.75f, 1.4f}, // 1 players to act
                                        {0.3f, 0.75f, 1.4f}, // 2 players to act
                                        {0.3f, 0.75f, 1.4f}, // 3 players to act
                                        {0.3f, 0.6f},        // 4 players to act
                                        {0.3f, 0.6f},        // 5 players to act
                                        {0.3f, 0.6f},        // 6 players to act
                                    }},
            {BettingRound::Turn, {
                                        {},
                                        {0.3f, 0.75f, 1.4f}, // 1 players to act
                                        {0.3f, 0.75f, 1.4f}, // 2 players to act
                                        {0.3f, 0.75f, 1.4f}, // 3 players to act
                                        {0.3f, 0.6f},        // 4 players to act
                                        {0.3f, 0.6f},        // 5 players to act
                                        {0.3f, 0.6f},        // 6 players to act
                                    }},
            {BettingRound::River, {
                                        {},
                                        {0.2f, 0.6f, 1.25f}, // 1 players to act
                                        {0.2f, 0.6f, 1.25f}, // 2 players to act
                                        {0.2f, 0.6f, 1.25f}, // 3 players to act
                                        {0.3f, 0.6f},        // 4 players to act
                                        {0.3f, 0.6f},        // 5 players to act
                                        {0.3f, 0.6f},        // 6 players to act
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

    phmap::parallel_flat_hash_map<string, Infoset,
        phmap::priv::hash_default_hash<string>,
        phmap::priv::hash_default_eq<string>,
        std::allocator<std::pair<const string, Infoset>>, 6, phmap::AbslMutex> Global::nodeMap = {};

    thread_local Deck Global::deck = Deck(CARDS);
} // namespace poker
