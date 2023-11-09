#include "tables/ehs_table.h"

string EHSTable::EHSTable5Cards = "EHSTable5Cards.txt";
string EHSTable::EHSTable6Cards = "EHSTable6Cards.txt";

EHSTable::EHSTable() : tableFlop{},
                       tableTurn{}
{
    // In essence, go through the 2+3 HandIndexer set and combine with opponent, turn, river
    // 1286792 * 1081*990 = 1 377 111 930 480 combinations

    LoadFromFile();

    if (EHSFlop == NULL)
    {
        Generate5CardsTable();
        SaveToFile();
        Generate6CardsTable();
        SaveToFile();
    }
    else if (EHSTurn == NULL)
    {
        Generate6CardsTable();
        SaveToFile();
    }
    CalculateFlopHistograms();
    ClusterFlop();
}

void EHSTable::ClusterFlop()
{
}

void EHSTable::Generate5CardsTable()
{
    cout << "Calculating effective hand strength table for 2 + 3 (1286792)" << endl;

    int cards[5] = {};
    long deadCardMask = 0;

    for (auto i = 0; i < NUM_ISOMORPHIC_FLOP; i++)
    {
        memset(tableFlop, 0, sizeof(tableFlop));
    }
}
