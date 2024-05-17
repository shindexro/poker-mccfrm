#ifndef __CLASS_OCHS_TABLE_H__
#define __CLASS_OCHS_TABLE_H__

#include "abstraction/global.h"
#include "algorithm/kmeans.h"
#include "utils/random.h"
#include "utils/utils.h"

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <chrono>
#include <indicators/block_progress_bar.hpp>
#include <indicators/cursor_control.hpp>
#include <iostream>
#include <oneapi/tbb.h>
#include <string>
#include <vector>

using namespace std;

namespace poker {
class OCHSTable {

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
} // namespace poker

#endif
