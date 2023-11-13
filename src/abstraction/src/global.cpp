#include "abstraction/global.h"

// vector<float> Global::raiseRatios({1.0f, 1.5f, 2.0f});
vector<float> Global::raiseRatios({2.0f});

HandIndexer Global::indexer_2;
HandIndexer Global::indexer_2_3;
HandIndexer Global::indexer_2_4;
HandIndexer Global::indexer_2_5;
HandIndexer Global::indexer_2_3_1;
HandIndexer Global::indexer_2_3_1_1;
HandIndexer Global::indexer_2_5_2;

Evaluator Global::handEvaluator;

oneapi::tbb::concurrent_hash_map<string, Infoset> Global::nodeMap = oneapi::tbb::concurrent_hash_map<string, Infoset>();
oneapi::tbb::concurrent_hash_map<string, Infoset> Global::nodeMapBaseline = oneapi::tbb::concurrent_hash_map<string, Infoset>();

thread_local Deck Global::deck = Deck();
