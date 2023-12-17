#include "tables/emd_table.h"

using namespace std;
using namespace chrono;
using namespace indicators;

namespace poker
{
    // mapping each canonical flop hand (2+3 cards) to a cluster
    vector<int> EMDTable::flopIndices;
    // mapping each canonical turn hand (2+4 cards) to a cluster
    vector<int> EMDTable::turnIndices;

    vector<vector<float>> EMDTable::histogramsFlop;
    vector<vector<float>> EMDTable::histogramsTurn;

    const string EMDTable::filenameEMDTurnTable = "EMDTurnTable.txt";
    const string EMDTable::filenameEMDFlopTable = "EMDFlopTable.txt";
    const string EMDTable::filenameEMDFlopHistogram = "EMDFlopHistogram.txt";
    const string EMDTable::filenameEMDTurnHistogram = "EMDTurnHistogram.txt";

    void EMDTable::Init()
    {
        LoadFromFile();

        if (turnIndices.size() == 0)
        {
            if (histogramsTurn.size() == 0)
            {
                GenerateTurnHistograms();
                SaveToFile();
            }
            ClusterTurn();
            SaveToFile();
            GenerateFlopHistograms();
            SaveToFile();
            ClusterFlop();
            SaveToFile();
        }
        else if (flopIndices.size() == 0)
        {
            if (histogramsFlop.size() == 0)
            {
                GenerateFlopHistograms();
                SaveToFile();
            }
            ClusterFlop();
            SaveToFile();
        }
    }
    void EMDTable::SaveToFile()
    {
        if (flopIndices.size())
        {
            utils::SaveToFile(flopIndices, filenameEMDFlopTable);
        }
        if (turnIndices.size())
        {
            utils::SaveToFile(turnIndices, filenameEMDTurnTable);
        }
        if (histogramsFlop.size())
        {
            utils::SaveToFile(histogramsFlop, filenameEMDFlopHistogram);
        }
        if (histogramsTurn.size())
        {
            utils::SaveToFile(histogramsTurn, filenameEMDTurnHistogram);
        }
    }

    void EMDTable::LoadFromFile()
    {
        if (utils::FileExists(filenameEMDTurnTable))
        {
            utils::LoadFromFile(turnIndices, filenameEMDTurnTable);
        }
        if (utils::FileExists(filenameEMDFlopTable))
        {
            utils::LoadFromFile(flopIndices, filenameEMDFlopTable);
        }
        if (flopIndices.size() == 0 && utils::FileExists(filenameEMDFlopHistogram))
        {
            utils::LoadFromFile(histogramsFlop, filenameEMDFlopHistogram);
        }
        if (turnIndices.size() == 0 && utils::FileExists(filenameEMDTurnHistogram))
        {
            utils::LoadFromFile(histogramsTurn, filenameEMDTurnHistogram);
        }
    }

    void EMDTable::GenerateTurnHistograms()
    {
        std::cout << "Generating histograms for " << Global::indexer_2_4.roundSize[1]
                  << " turn hands of length " << Global::turnHistogramSize << " each..." << endl;
        chrono::steady_clock::time_point start = chrono::steady_clock::now();

        histogramsTurn = vector<vector<float>>(Global::indexer_2_4.roundSize[1], vector<float>(Global::turnHistogramSize));

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
                                      auto [startItemIdx, endItemIdx] = utils::GetWorkItemsIndices((int)Global::indexer_2_4.roundSize[1], Global::NOF_THREADS, t);
                                      for (auto i = startItemIdx; i < endItemIdx; ++i)
                                      {
                                          auto cardsTurn = vector<int>(6);
                                          auto countTable = vector<vector<int>>(3, vector<int>(3));

                                          Global::indexer_2_4.Unindex(Global::indexer_2_4.rounds - 1, i, cardsTurn);
                                          long deadCardMask = (1L << cardsTurn[0]) + (1L << cardsTurn[1]) + (1L << cardsTurn[2]) + (1L << cardsTurn[3]) + (1L << cardsTurn[4]) + (1L << cardsTurn[5]);

                                          ulong shared = (1uL << cardsTurn[2]) + (1uL << cardsTurn[3]) + (1uL << cardsTurn[4]) + (1uL << cardsTurn[5]);
                                          ulong handTurn = (1uL << cardsTurn[0]) + (1uL << cardsTurn[1]) + shared;
                                          int valueTurn = Global::handEvaluator->Evaluate(handTurn);

                                          for (auto cardRiver = 0; cardRiver < Global::CARDS; cardRiver++)
                                          {
                                              countTable = vector<vector<int>>(3, vector<int>(3));
                                              if (((1L << cardRiver) & deadCardMask) != 0)
                                              {
                                                  continue;
                                              }
                                              deadCardMask |= (1L << cardRiver);

                                              ulong handRiver = (1uL << cardsTurn[0]) + (1uL << cardsTurn[1]) + shared + (1uL << cardRiver);
                                              int valueRiver = Global::handEvaluator->Evaluate(handRiver);

                                              for (auto card1Opponent = 0; card1Opponent < Global::CARDS - 1; card1Opponent++)
                                              {
                                                  if (((1L << card1Opponent) & deadCardMask) != 0)
                                                  {
                                                      continue;
                                                  }
                                                  for (auto card2Opponent = card1Opponent + 1; card2Opponent < Global::CARDS; card2Opponent++)
                                                  {
                                                      if (((1L << card2Opponent) & deadCardMask) != 0)
                                                      {
                                                          continue;
                                                      }

                                                      ulong handOppRiver = (1uL << card1Opponent) + (1uL << card2Opponent) + shared + (1uL << cardRiver);
                                                      ulong handOppTurn = (1uL << card1Opponent) + (1uL << card2Opponent) + shared;

                                                      int valueOppTurn = Global::handEvaluator->Evaluate(handOppTurn);
                                                      int valueOppRiver = Global::handEvaluator->Evaluate(handOppRiver);

                                                      // index 0 = win, 1 = draw, 2 = loss
                                                      int indexTurn = valueTurn > valueOppTurn ? 0 : valueTurn == valueOppTurn ? 1
                                                                                                                               : 2;
                                                      int indexRiver = valueRiver > valueOppRiver ? 0 : valueRiver == valueOppRiver ? 1
                                                                                                                                    : 2;

                                                      countTable[indexTurn][indexRiver] += 1;
                                                  }
                                              }

                                              // save the equity in histogram
                                              // float behindRiver = countTable[0, 2] + countTable[1, 2] + countTable[2, 2];
                                              // float tiedRiver = countTable[0, 1] + countTable[1, 1] + countTable[2, 1];
                                              // float aheadRiver = countTable[0, 0] + countTable[1, 0] + countTable[2, 0];

                                              float behindTurn = countTable[2][0] + countTable[2][1] + countTable[2][2];
                                              float tiedTurn = countTable[1][0] + countTable[1][1] + countTable[1][2];
                                              float aheadTurn = countTable[0][0] + countTable[0][1] + countTable[0][2];

                                              float handstrengthTurn = (aheadTurn + tiedTurn / 2.0f) / (aheadTurn + tiedTurn + behindTurn);
                                              float PpotTurn = (behindTurn + tiedTurn == 0) ? 0 : (countTable[2][0] + countTable[2][1] / 2.0f + countTable[1][0] / 2.0f) / (behindTurn + tiedTurn);
                                              float NPotTurn = (aheadTurn + tiedTurn == 0) ? 0 : (countTable[0][2] + countTable[1][2] / 2.0f + countTable[0][1] / 2.0f) / (aheadTurn + tiedTurn);

                                              histogramsTurn[i][min({Global::turnHistogramSize - 1,
                                                                     (int)(Global::turnHistogramSize * (handstrengthTurn * (1 - NPotTurn) + (1 - handstrengthTurn) * PpotTurn))})] += 1;

                                              deadCardMask &= ~(1L << cardRiver);
                                          }

                                          sharedLoopCounter++;
                                          bar.set_progress(sharedLoopCounter);
                                      }
                                  });

        chrono::steady_clock::time_point end = chrono::steady_clock::now();
        auto elapsed = chrono::duration_cast<std::chrono::seconds>(end - start).count();
        std::cout << "Time taken to generate turn histograms: " << elapsed << "[s]" << endl;
    }

    void EMDTable::GenerateFlopHistograms()
    {
        std::cout << "Generating histograms for " << Global::indexer_2_3.roundSize[1]
                  << " flop hands of length " << Global::flopHistogramSize << " each..." << endl;
        chrono::steady_clock::time_point start = chrono::steady_clock::now();

        histogramsFlop = vector<vector<float>>(Global::indexer_2_3.roundSize[1], vector<float>(Global::flopHistogramSize));

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
                                      auto [startItemIdx, endItemIdx] = utils::GetWorkItemsIndices((int)Global::indexer_2_3.roundSize[1], Global::NOF_THREADS, t);
                                      for (auto i = startItemIdx; i < endItemIdx; ++i)
                                      {
                                          auto cardsFlop = vector<int>(5);
                                          auto countTable = vector<vector<int>>(3, vector<int>(3));

                                          Global::indexer_2_3.Unindex(Global::indexer_2_3.rounds - 1, i, cardsFlop);
                                          long deadCardMask = (1L << cardsFlop[0]) + (1L << cardsFlop[1]) + (1L << cardsFlop[2]) + (1L << cardsFlop[3]) + (1L << cardsFlop[4]);

                                          ulong handFlop = (1uL << cardsFlop[0]) + (1uL << cardsFlop[1]) + (1uL << cardsFlop[2]) +
                                                           (1uL << cardsFlop[3]) + (1uL << cardsFlop[4]);
                                          int valueFlop = Global::handEvaluator->Evaluate(handFlop);

                                          for (auto cardTurn = 0; cardTurn < Global::CARDS - 1; cardTurn++)
                                          {
                                              if (((1L << cardTurn) & deadCardMask) != 0)
                                              {
                                                  continue;
                                              }
                                              deadCardMask |= (1L << cardTurn);
                                              for (auto cardRiver = cardTurn + 1; cardRiver < Global::CARDS; cardRiver++)
                                              {
                                                  countTable = vector<vector<int>>(3, vector<int>(3));

                                                  if (((1L << cardRiver) & deadCardMask) != 0)
                                                  {
                                                      continue;
                                                  }
                                                  deadCardMask |= (1L << cardRiver);

                                                  ulong handRiver = (1uL << cardsFlop[0]) + (1uL << cardsFlop[1]) + (1uL << cardsFlop[2]) + (1uL << cardsFlop[3]) +
                                                                    (1uL << cardsFlop[4]) + (1uL << cardTurn) + (1uL << cardRiver);
                                                  int valueRiver = Global::handEvaluator->Evaluate(handRiver);

                                                  for (auto card1Opponent = 0; card1Opponent < Global::CARDS - 1; card1Opponent++)
                                                  {
                                                      if (((1L << card1Opponent) & deadCardMask) != 0)
                                                      {
                                                          continue;
                                                      }
                                                      for (auto card2Opponent = card1Opponent + 1; card2Opponent < Global::CARDS; card2Opponent++)
                                                      {
                                                          if (((1L << card2Opponent) & deadCardMask) != 0)
                                                          {
                                                              continue;
                                                          }

                                                          ulong handOppRiver = (1uL << card1Opponent) + (1uL << card2Opponent) + (1uL << cardsFlop[2]) + (1uL << cardsFlop[3]) +
                                                                               (1uL << cardsFlop[4]) + (1uL << cardTurn) + (1uL << cardRiver);
                                                          ulong handOppFlop = (1uL << card1Opponent) + (1uL << card2Opponent) + (1uL << cardsFlop[2]) +
                                                                              (1uL << cardsFlop[3]) + (1uL << cardsFlop[4]);

                                                          int valueOppFlop = Global::handEvaluator->Evaluate(handOppFlop);
                                                          int valueOppRiver = Global::handEvaluator->Evaluate(handOppRiver);

                                                          // index 0 = win, 1 = draw, 2 = loss
                                                          int indexFlop = valueFlop > valueOppFlop ? 0 : valueFlop == valueOppFlop ? 1
                                                                                                                                   : 2;
                                                          int indexRiver = valueRiver > valueOppRiver ? 0 : valueRiver == valueOppRiver ? 1
                                                                                                                                        : 2;

                                                          countTable[indexFlop][indexRiver] += 1;
                                                      }
                                                  }
                                                  // save the equity in histogram
                                                  float behindFlop = countTable[2][0] + countTable[2][1] + countTable[2][2];
                                                  float tiedFlop = countTable[1][0] + countTable[1][1] + countTable[1][2];
                                                  float aheadFlop = countTable[0][0] + countTable[0][1] + countTable[0][2];

                                                  float handstrengthFlop = (aheadFlop + tiedFlop / 2.0f) / (aheadFlop + tiedFlop + behindFlop);
                                                  float PpotFlop = (behindFlop + tiedFlop == 0) ? 0 : (countTable[2][0] + countTable[2][1] / 2.0f + countTable[1][0] / 2.0f) / (behindFlop + tiedFlop);
                                                  float NPotFlop = (aheadFlop + tiedFlop == 0) ? 0 : (countTable[0][2] + countTable[1][2] / 2.0f + countTable[0][1] / 2.0f) / (aheadFlop + tiedFlop);

                                                  histogramsFlop[i][min({Global::flopHistogramSize - 1,
                                                                         (int)(Global::flopHistogramSize * (handstrengthFlop * (1 - NPotFlop) + (1 - handstrengthFlop) * PpotFlop))})] += 1;
                                                  // Console.WriteLine("{0}, {1}, {2}, {3}", handstrengthFlop, NPotFlop, PpotFlop, (handstrengthFlop * (1 - NPotFlop) + (1 - handstrengthFlop) * PpotFlop));
                                                  deadCardMask &= ~(1L << cardRiver);
                                              }

                                              deadCardMask &= ~(1L << cardTurn);
                                          }
                                          sharedLoopCounter++;
                                          bar.set_progress(sharedLoopCounter);
                                      }
                                  });
        chrono::steady_clock::time_point end = chrono::steady_clock::now();
        auto elapsed = chrono::duration_cast<std::chrono::seconds>(end - start).count();
        cout << "Time taken to generate flop histograms: " << elapsed << "[s]" << endl;
    }

    void EMDTable::ClusterTurn()
    {
        // k-means clustering
        chrono::steady_clock::time_point start = chrono::steady_clock::now();
        Kmeans kmeans = Kmeans();
        auto indices = vector<int>();
        // int[] indices = FileHandler.LoadFromFileIndex("EMDTableTurn_temp.txt");
        turnIndices = kmeans.ClusterEMD(histogramsTurn, Global::nofTurnBuckets, 1, indices);

        chrono::steady_clock::time_point end = chrono::steady_clock::now();
        auto elapsed = chrono::duration_cast<std::chrono::seconds>(end - start).count();
        cout << "Time taken to cluster turn: " << elapsed << "[s]" << endl;

        std::cout << "Created the following clusters for the Turn (extract of one cluster): " << endl;
        int nofEntriesToDisplay = 20;
        for (auto i = 0L; i < Global::indexer_2_4.roundSize[1] && nofEntriesToDisplay > 0; ++i)
        {
            if (turnIndices[i] == 0)
            {
                auto cards = vector<int>(6);
                Global::indexer_2_4.Unindex(Global::indexer_2_4.rounds - 1, i, cards);

                Hand hand = Hand();
                hand.cards = vector<Card>({Card(cards[0]), Card(cards[1]), Card(cards[2]), Card(cards[3]),
                                           Card(cards[4]), Card(cards[5])});
                hand.PrintColoredCards("\n");
                nofEntriesToDisplay--;
            }
        }
        if (nofEntriesToDisplay == 0)
            cout << "..." << endl;
    }

    void EMDTable::ClusterFlop()
    {
        // k-means clustering
        chrono::steady_clock::time_point start = chrono::steady_clock::now();
        Kmeans kmeans = Kmeans();
        auto indices = vector<int>();
        // int[] indices = FileHandler.LoadFromFileIndex("EMDTableFlop_temp.txt");
        flopIndices = kmeans.ClusterEMD(histogramsFlop, Global::nofFlopBuckets, 1, indices);

        chrono::steady_clock::time_point end = chrono::steady_clock::now();
        auto elapsed = chrono::duration_cast<std::chrono::seconds>(end - start).count();
        cout << "Time taken to cluster flop: " << elapsed << "[s]" << endl;

        cout << "Created the following clusters for the Flop (extract of one cluster): " << endl;
        int nofEntriesToDisplay = 20;
        for (auto i = 0L; i < Global::indexer_2_3.roundSize[1] && nofEntriesToDisplay > 0; ++i)
        {
            if (turnIndices[i] == 0)
            {
                auto cards = vector<int>(5);
                Global::indexer_2_3.Unindex(Global::indexer_2_3.rounds - 1, i, cards);

                Hand hand = Hand();
                hand.cards = vector<Card>({Card(cards[0]), Card(cards[1]), Card(cards[2]), Card(cards[3]),
                                           Card(cards[4])});
                hand.PrintColoredCards("\n");
                nofEntriesToDisplay--;
            }
        }
        if (nofEntriesToDisplay == 0)
            std::cout << "..." << endl;
    }
} // namespace poker
