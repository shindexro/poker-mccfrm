#ifndef __CLASS_OCHS_TABLE_H__
#define __CLASS_OCHS_TABLE_H__

#include "abstraction/global.h"
#include "utils/random.h"

#include <string>
#include <iostream>
#include <vector>
#include <chrono>

using namespace std;

class OCHSTable
{

public:
    static vector<int> preflopIndices;
    static vector<int> riverIndices;

    static vector<vector<float>> histogramsPreflop;
    static vector<vector<float>> histogramsRiver;

    static const string filenameOppClusters;
    static const string filenameRiverClusters;
    static const string filenameRiverHistograms;

    static void Init();

private:
    static void CalculateOCHSOpponentClusters();
    static void ClusterPreflopHands();
    static void ClusterRiver();
    static void GenerateRiverHistograms();
    static void SaveToFile();
    static void LoadFromFile();
};

#endif
