#include "abstraction/global.h"

vector<float> Global::raises({1.0f, 1.5f, 2.0f});

HandIndexer indexer_2;
HandIndexer indexer_2_3;
HandIndexer indexer_2_4;
HandIndexer indexer_2_5;
HandIndexer indexer_2_3_1;
HandIndexer indexer_2_3_1_1;
HandIndexer indexer_2_5_2;
Evaluator handEvaluator;

ConcurrentDictionary<string, Infoset> nodeMap = new ConcurrentDictionary<string, Infoset>(Global.NOF_THREADS, 1000000);
ConcurrentDictionary<string, Infoset> nodeMapBaseline = new ConcurrentDictionary<string, Infoset>(Global.NOF_THREADS, 1000000);

ThreadLocal<Deck> Deck = new ThreadLocal<Deck>(() = > new Deck());
