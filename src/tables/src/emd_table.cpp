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

    const string EMDTable::filenameEMDTurnTable = "EMDTurnTable.bin";
    const string EMDTable::filenameEMDFlopTable = "EMDFlopTable.bin";
    const string EMDTable::filenameEMDFlopHistogram = "EMDFlopHistogram.bin";
    const string EMDTable::filenameEMDTurnHistogram = "EMDTurnHistogram.bin";

    void EMDTable::Init()
    {
        LoadFromFile();
        SaveToFile();

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
        if (flopIndices.size() && !utils::FileExists(filenameEMDFlopTable))
        {
            utils::SaveToFile(flopIndices, filenameEMDFlopTable);
        }
        if (turnIndices.size() && !utils::FileExists(filenameEMDTurnTable))
        {
            utils::SaveToFile(turnIndices, filenameEMDTurnTable);
        }
        if (histogramsFlop.size() && !utils::FileExists(filenameEMDFlopHistogram))
        {
            utils::SaveToFile(histogramsFlop, filenameEMDFlopHistogram);
        }
        if (histogramsTurn.size() && !utils::FileExists(filenameEMDTurnHistogram))
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
                  << " turn hands of length " << Global::nofRiverBuckets << " each..." << endl;
        chrono::steady_clock::time_point start = chrono::steady_clock::now();

        histogramsTurn = vector<vector<float>>(Global::indexer_2_4.roundSize[1], vector<float>(Global::nofRiverBuckets));

        auto threadFunc = [&](int /*threadIdx*/, int itemIdx)
        {
            auto cards = vector<int>(6);

            Global::indexer_2_4.Unindex(Global::indexer_2_4.rounds - 1, itemIdx, cards);

            ulong shared = (1uL << cards[2]) + (1uL << cards[3]) + (1uL << cards[4]) + (1uL << cards[5]);
            ulong handTurn = (1uL << cards[0]) + (1uL << cards[1]) + shared;

            for (auto cardRiver = 0; cardRiver < Global::CARDS; cardRiver++)
            {
                if ((1uL << cardRiver) & handTurn)
                    continue;

                cards.push_back(cardRiver);
                auto riverHandCanonicalIndex = Global::indexer_2_5.IndexLastRound(cards);
                auto riverClusterIndex = OCHSTable::riverIndices[riverHandCanonicalIndex];
                histogramsTurn[itemIdx][riverClusterIndex]++;

                cards.pop_back();
            }
        };

        utils::parallelise(histogramsTurn.size(), threadFunc);

        chrono::steady_clock::time_point end = chrono::steady_clock::now();
        auto elapsed = chrono::duration_cast<std::chrono::seconds>(end - start).count();
        std::cout << "Time taken to generate turn histograms: " << elapsed << "[s]" << endl;
    }

    void EMDTable::GenerateFlopHistograms()
    {
        std::cout << "Generating histograms for " << Global::indexer_2_3.roundSize[1]
                  << " flop hands of length " << Global::nofTurnBuckets << " each..." << endl;
        chrono::steady_clock::time_point start = chrono::steady_clock::now();

        histogramsFlop = vector<vector<float>>(Global::indexer_2_3.roundSize[1], vector<float>(Global::nofTurnBuckets));

        auto threadFunc = [&](int /*threadIdx*/, int itemIdx)
        {
            auto cards = vector<int>(5);

            Global::indexer_2_3.Unindex(Global::indexer_2_3.rounds - 1, itemIdx, cards);

            ulong shared = (1uL << cards[2]) + (1uL << cards[3]) + (1uL << cards[4]);
            ulong handFlop = (1uL << cards[0]) + (1uL << cards[1]) + shared;

            for (auto cardTurn = 0; cardTurn < Global::CARDS; cardTurn++)
            {
                if ((1uL << cardTurn) & handFlop)
                    continue;

                cards.push_back(cardTurn);
                auto turnHandCanonicalIndex = Global::indexer_2_4.IndexLastRound(cards);
                auto turnClusterIndex = EMDTable::turnIndices[turnHandCanonicalIndex];
                histogramsFlop[itemIdx][turnClusterIndex]++;

                cards.pop_back();
            }
        };
        utils::parallelise(histogramsFlop.size(), threadFunc);

        chrono::steady_clock::time_point end = chrono::steady_clock::now();
        auto elapsed = chrono::duration_cast<std::chrono::seconds>(end - start).count();
        cout << "Time taken to generate flop histograms: " << elapsed << "[s]" << endl;
    }

    void EMDTable::ClusterTurn()
    {
        chrono::steady_clock::time_point start = chrono::steady_clock::now();
        Kmeans kmeans = Kmeans();
        auto indices = vector<int>();
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
        chrono::steady_clock::time_point start = chrono::steady_clock::now();
        Kmeans kmeans = Kmeans();
        auto indices = vector<int>();
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
