#include "abstraction/global.h"

namespace poker
{
    // ratios must be sorted in ascending order
    const map<BettingRound, vector<vector<float>>> Global::raiseRatiosByRoundByPlayerCount{
            {BettingRound::Preflop, {
                                        {}, // 0 players to act
                                        {1.5f, 2.0f}, // 1 players to act
                                        {1.5f, 2.0f}, // 2 players to act
                                        {1.5f, 2.0f}, // 3 players to act
                                        {1.5f, 2.0f}, // 4 players to act
                                        {1.5f, 2.0f}, // 5 players to act
                                        {1.5f, 2.0f}, // 6 players to act
                                    }},
            {BettingRound::Flop, {
                                        {},
                                        {0.27f, 0.5f, 0.75f, 1.0f, 1.4f, 2.0f}, // 1 players to act
                                        {0.27f, 0.5f, 0.75f, 1.0f, 1.4f, 2.0f}, // 2 players to act
                                        {0.27f, 0.5f, 0.75f, 1.0f, 1.4f, 2.0f}, // 3 players to act
                                        {0.27f, 0.5f, 0.75f, 1.0f, 1.4f, 2.0f}, // 4 players to act
                                        {0.27f, 0.5f, 0.75f, 1.0f, 1.4f, 2.0f}, // 5 players to act
                                        {0.27f, 0.5f, 0.75f, 1.0f, 1.4f, 2.0f}, // 6 players to act
                                 }},
            {BettingRound::Turn, {
                                        {},
                                        {0.27f, 0.5f, 0.75f, 1.0f, 1.4f, 2.0f}, // 1 players to act
                                        {0.27f, 0.5f, 0.75f, 1.0f, 1.4f, 2.0f}, // 2 players to act
                                        {0.27f, 0.5f, 0.75f, 1.0f, 1.4f, 2.0f}, // 3 players to act
                                        {0.27f, 0.5f, 0.75f, 1.0f, 1.4f, 2.0f}, // 4 players to act
                                        {0.27f, 0.5f, 0.75f, 1.0f, 1.4f, 2.0f}, // 5 players to act
                                        {0.27f, 0.5f, 0.75f, 1.0f, 1.4f, 2.0f}, // 6 players to act
                                 }},
            {BettingRound::River, {
                                        {},
                                        {0.27f, 0.5f, 0.75f, 1.0f, 1.4f, 2.0f}, // 1 players to act
                                        {0.27f, 0.5f, 0.75f, 1.0f, 1.4f, 2.0f}, // 2 players to act
                                        {0.27f, 0.5f, 0.75f, 1.0f, 1.4f, 2.0f}, // 3 players to act
                                        {0.27f, 0.5f, 0.75f, 1.0f, 1.4f, 2.0f}, // 4 players to act
                                        {0.27f, 0.5f, 0.75f, 1.0f, 1.4f, 2.0f}, // 5 players to act
                                        {0.27f, 0.5f, 0.75f, 1.0f, 1.4f, 2.0f}, // 6 players to act
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

    NodeMap Global::nodeMap = {};

    thread_local Deck Global::deck = Deck(CARDS);
} // namespace poker
