/// <summary>
/// Opponent Cluster Hand Strength (Last round) of IR-KE-KO
/// http://poker.cs.ualberta.ca/publications/AAMAS13-abstraction.pdf
///
/// cluster starting hands (169) into 8 buckets using earth mover's distance (monte carlo sampling)
/// then, for each river private state calculate winning chance against each of the 8 buckets (opponent hands)
/// so we have 123156254 river combinations (2 + 5 cards), which need to be checked against the opponent cards
/// then we use k means with L2 distance metric to cluster all these "histograms" into k buckets
///
/// </summary>

#include "tables/ochs_table.h"

using namespace std;
using namespace chrono;
using namespace indicators;

vector<int> OCHSTable::preflopIndices; // has 169 elements to map each starting hand to a cluster
vector<int> OCHSTable::riverIndices;   // mapping each canonical river hand (7 cards) to a cluster

vector<vector<float>> OCHSTable::histogramsPreflop;
vector<vector<float>> OCHSTable::histogramsRiver;

const string OCHSTable::filenameOppClusters = "OCHSOpponentClusters.txt";
const string OCHSTable::filenameRiverClusters = "OCHSRiverClusters.txt";
const string OCHSTable::filenameRiverHistograms = "OCHSRiverHistograms.txt";

void OCHSTable::Init()
{
    LoadFromFile();

    if (riverIndices.size())
        return;

    if (!histogramsRiver.size())
    {
        if (!preflopIndices.size())
        {
            CalculateOCHSOpponentClusters();
            ClusterPreflopHands();
            SaveToFile();
        }
        GenerateRiverHistograms();
        SaveToFile();
    }
    ClusterRiver();
    SaveToFile();
}

void OCHSTable::CalculateOCHSOpponentClusters()
{
    std::cout << "Calculating " << Global::nofOpponentClusters << " opponent clusters for OCHS using Monte Carlo Sampling..." << endl;
    chrono::steady_clock::time_point start = chrono::steady_clock::now();

    histogramsPreflop = vector<vector<float>>(169, vector<float>((Global::preflopHistogramSize)));

    for (int i = 0; i < 169; i++)
    {
        auto cards = vector<int>(2);
        Global::indexer_2.Unindex(Global::indexer_2.rounds - 1, i, cards);
        long deadCardMask = (1L << cards[0]) + (1L << cards[1]);
        for (int steps = 0; steps < Global::nofMCSimsPerPreflopHand; steps++)
        {
            int cardFlop1 = randint(0, 52);
            while (((1L << cardFlop1) & deadCardMask) != 0)
                cardFlop1 = randint(0, 52);
            deadCardMask |= (1L << cardFlop1);

            int cardFlop2 = randint(0, 52);
            while (((1L << cardFlop2) & deadCardMask) != 0)
                cardFlop2 = randint(0, 52);
            deadCardMask |= (1L << cardFlop2);

            int cardFlop3 = randint(0, 52);
            while (((1L << cardFlop3) & deadCardMask) != 0)
                cardFlop3 = randint(0, 52);
            deadCardMask |= (1L << cardFlop3);

            int cardTurn = randint(0, 52);
            while (((1L << cardTurn) & deadCardMask) != 0)
                cardTurn = randint(0, 52);
            deadCardMask |= (1L << cardTurn);

            int cardRiver = randint(0, 52);
            while (((1L << cardRiver) & deadCardMask) != 0)
                cardRiver = randint(0, 52);
            deadCardMask |= (1L << cardRiver);

            auto strength = vector<int>(3);
            for (int card1Opponent = 0; card1Opponent < 51; card1Opponent++)
            {
                if (((1L << card1Opponent) & deadCardMask) != 0)
                {
                    continue;
                }
                // deadCardMask |= (1L << card1Opponent); // card2Opponent is anyway > card1Opponent
                for (int card2Opponent = card1Opponent + 1; card2Opponent < 52; card2Opponent++)
                {
                    if (((1L << card2Opponent) & deadCardMask) != 0)
                    {
                        continue;
                    }
                    ulong handSevenCards = (1uL << cards[0]) + (1uL << cards[1]) + (1uL << cardFlop1) + (1uL << cardFlop2) + (1uL << cardFlop3) + (1uL << cardTurn) + (1uL << cardRiver);
                    ulong handOpponentSevenCards = (1uL << cardFlop1) + (1uL << cardFlop2) + (1uL << cardFlop3) + (1uL << cardTurn) + (1uL << cardRiver) + (1uL << card1Opponent) + (1uL << card2Opponent);

                    int valueSevenCards = Global::handEvaluator.Evaluate(handSevenCards);
                    int valueOpponentSevenCards = Global::handEvaluator.Evaluate(handOpponentSevenCards);

                    // strength = histogram with column win, draw, and loss
                    int index = (valueSevenCards > valueOpponentSevenCards ? 0 : valueSevenCards == valueOpponentSevenCards ? 1
                                                                                                                            : 2);

                    strength[index] += 1;
                }
            }
            float equity = (strength[0] + strength[1] / 2.0f) / (strength[0] + strength[1] + strength[2]);
            histogramsPreflop[i][(min({Global::preflopHistogramSize - 1,
                                       (int)(equity * (float)Global::preflopHistogramSize)}))] += 1;
            deadCardMask = (1L << cards[0]) + (1L << cards[1]);
        }
    };

    chrono::steady_clock::time_point end = chrono::steady_clock::now();
    auto elapsed = chrono::duration_cast<std::chrono::seconds>(end - start).count();
    cout << "Time taken to generate lookup table: " << elapsed << "[s]" << endl;

    cout << "Calculated histograms: " << endl;
    auto cardsOutput = vector<int>(2);
    for (int i = 0; i < 169; ++i)
    {
        cardsOutput.clear();
        Global::indexer_2.Unindex(Global::indexer_2.rounds - 1, i, cardsOutput);
        Hand hand = Hand();

        hand.cards.push_back(Card(cardsOutput[0]));
        hand.cards.push_back(Card(cardsOutput[1]));
        hand.PrintColoredCards();
        cout << ": ";
        for (int j = 0; j < Global::preflopHistogramSize; ++j)
        {
            cout << histogramsPreflop[i][j] << " ";
        }
        cout << endl;
    }
}

void OCHSTable::ClusterPreflopHands()
{
    // k-means clustering
    Kmeans kmeans = Kmeans();
    auto emptyVector = vector<int>();
    preflopIndices = kmeans.ClusterEMD(histogramsPreflop, Global::nofOpponentClusters, 100, emptyVector);

    cout << "Created the following cluster for starting hands: " << endl;
    vector<Hand> startingHands = GetStartingHandChart();

    for (int i = 0; i < 169; ++i)
    {
        auto toIndex = vector<int>({startingHands[i].cards[0].Index(),
                                    startingHands[i].cards[1].Index()});
        long index = Global::indexer_2.IndexLast(toIndex);

        cout << preflopIndices[index] << "  ";

        if (i % 13 == 12)
            cout << endl;
    }

    cout << endl;
}

void OCHSTable::ClusterRiver()
{
    // k-means clustering
    chrono::steady_clock::time_point start = chrono::steady_clock::now();

    Kmeans kmeans = Kmeans();
    // TODO: vector<int> indices = FileHandler.LoadFromFileIndex("OCHSRiverClusters_temp.txt");
    vector<int> indices;
    ifstream file("OCHSRiverClusters_temp.txt");
    boost::archive::binary_iarchive archive(file);
    archive >> indices;

    riverIndices = kmeans.ClusterL2(histogramsRiver, Global::nofRiverBuckets, 1, indices);

    cout << "Created the following clusters for the River: " << endl;

    int nofExamplesToPrint = 10;
    for (int i = 0; i < Global::indexer_2_5.roundSize[1]; ++i)
    {
        if (riverIndices[i] == 0 && nofExamplesToPrint > 0)
        {
            auto cards = vector<int>(7);
            Global::indexer_2_5.Unindex(Global::indexer_2_5.rounds - 1, i, cards);

            Hand hand = Hand();
            hand.cards.push_back(Card(cards[0]));
            hand.cards.push_back(Card(cards[1]));
            hand.cards.push_back(Card(cards[2]));
            hand.cards.push_back(Card(cards[3]));
            hand.cards.push_back(Card(cards[4]));
            hand.cards.push_back(Card(cards[5]));
            hand.cards.push_back(Card(cards[6]));
            hand.PrintColoredCards();
            cout << endl;
            nofExamplesToPrint--;
        }
    }
    chrono::steady_clock::time_point end = chrono::steady_clock::now();
    auto elapsed = chrono::duration_cast<std::chrono::seconds>(end - start).count();
    cout << "Time taken to generate lookup table: " << elapsed << "[s]" << endl;
}

void OCHSTable::GenerateRiverHistograms()
{
    cout << "Generating histograms for " << Global::indexer_2_5.roundSize[1] << " river hands of length "
         << Global::nofOpponentClusters << " each..." << endl;
    chrono::steady_clock::time_point start = chrono::steady_clock::now();

    histogramsRiver = vector<vector<float>>(Global::indexer_2_5.roundSize[1], vector<float>(Global::nofOpponentClusters));

    atomic<long> sharedLoopCounter = 0;

    indicators::show_console_cursor(false);
    BlockProgressBar bar{
        option::BarWidth{80},
        option::Start{"["},
        option::End{"]"},
        option::ForegroundColor{Color::white},
        option::FontStyles{std::vector<FontStyle>{FontStyle::bold}},
        option::ShowElapsedTime{true},
        option::ShowRemainingTime{true},
        option::MaxProgress{Global::indexer_2_5.roundSize[1]}};

    oneapi::tbb::parallel_for(0, Global::NOF_THREADS,
                              [&](int t)
                              {
                                  long iter = 0;
                                  for (int i = get<0>(GetWorkItemsIndices((int)Global::indexer_2_5.roundSize[1], Global::NOF_THREADS, t));
                                       i < get<1>(GetWorkItemsIndices((int)Global::indexer_2_5.roundSize[1], Global::NOF_THREADS, t)); ++i)
                                  {
                                      auto cards = std::vector<int>(7);
                                      Global::indexer_2_5.Unindex(Global::indexer_2_5.rounds - 1, i, cards);
                                      long deadCardMask = (1L << cards[0]) + (1L << cards[1]) + (1L << cards[2]) + (1L << cards[3]) + (1L << cards[4]) + (1L << cards[5]) + (1L << cards[6]);

                                      for (int card1Opponent = 0; card1Opponent < 51; card1Opponent++)
                                      {
                                          if (((1L << card1Opponent) & deadCardMask) != 0)
                                          {
                                              continue;
                                          }
                                          for (int card2Opponent = card1Opponent + 1; card2Opponent < 52; card2Opponent++)
                                          {
                                              if (((1L << card2Opponent) & deadCardMask) != 0)
                                              {
                                                  continue;
                                              }

                                              ulong handSevenCards = (1uL << cards[0]) + (1uL << cards[1]) + (1uL << cards[2]) + (1uL << cards[3]) + (1uL << cards[4]) +
                                                                     +(1uL << cards[5]) + (1uL << cards[6]);
                                              ulong handOpponentSevenCards = (1uL << card1Opponent) + (1uL << card2Opponent) + (1uL << cards[2]) + (1uL << cards[3]) + (1uL << cards[4]) + (1uL << cards[5]) + (1uL << cards[6]);

                                              int valueSevenCards = Global::handEvaluator.Evaluate(handSevenCards);
                                              int valueOpponentSevenCards = Global::handEvaluator.Evaluate(handOpponentSevenCards);

                                              auto preflop = std::vector<int>({card1Opponent, card2Opponent});
                                              long indexPreflop = Global::indexer_2.IndexLast(preflop);
                                              histogramsRiver[i][preflopIndices[indexPreflop]] += valueSevenCards > valueOpponentSevenCards ? 1 : (valueSevenCards == valueOpponentSevenCards) ? 0.5f
                                                                                                                                                                                               : 0.0f;
                                          }
                                      }

                                      iter++;

                                      if (iter == 10000)
                                      {
                                          sharedLoopCounter += iter;
                                          bar.set_progress(sharedLoopCounter);
                                          iter = 0;
                                      }
                                  }
                              });
    indicators::show_console_cursor(true);
    chrono::steady_clock::time_point end = chrono::steady_clock::now();
    auto elapsed = chrono::duration_cast<std::chrono::seconds>(end - start).count();
    cout << "Time taken to generate lookup table: " << elapsed << "[s]" << endl;
}

void OCHSTable::SaveToFile()
{
    if (preflopIndices.size())
    {
        cout << "Saving table to file " << filenameOppClusters << endl;
        ofstream file(filenameOppClusters);
        boost::archive::binary_oarchive archive(file);
        archive << preflopIndices;
    }
    if (histogramsRiver.size())
    {
        cout << "Saving river histograms from file " << filenameRiverHistograms << endl;
        ofstream file(filenameRiverHistograms);
        boost::archive::binary_oarchive archive(file);
        archive << histogramsRiver;
    }
    if (riverIndices.size())
    {
        cout << "Saving river cluster index to file " << filenameRiverClusters << endl;
        ofstream file(filenameRiverClusters);
        boost::archive::binary_oarchive archive(file);
        archive << riverIndices;
    }
}

void OCHSTable::LoadFromFile()
{
    if (filenameRiverClusters.size() && access(filenameRiverClusters.c_str(), F_OK) != -1)
    {
        ifstream file(filenameRiverClusters);
        boost::archive::binary_iarchive archive(file);
        archive >> riverIndices;
    }
    else
    {
        if (filenameRiverHistograms.size() && access(filenameRiverHistograms.c_str(), F_OK) != -1)
        {
            cout << "Loading river histograms from file " << filenameRiverHistograms << endl;
            ifstream file(filenameRiverHistograms);
            boost::archive::binary_iarchive archive(file);
            archive >> histogramsRiver;
        }
        if (filenameOppClusters.size() && access(filenameOppClusters.c_str(), F_OK) != -1)
        {
            cout << "Loading flop opponent clusters from file " << filenameOppClusters << endl;
            ifstream file(filenameOppClusters);
            boost::archive::binary_iarchive archive(file);
            archive >> preflopIndices;
        }
    }
}
