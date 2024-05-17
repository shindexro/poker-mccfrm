#ifndef __CLASS_EMD_TABLE_H__
#define __CLASS_EMD_TABLE_H__

#include "abstraction/global.h"
#include "algorithm/kmeans.h"
#include "game/hand.h"
#include "tables/ochs_table.h"
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
class EMDTable {

public:
  static vector<int>
      flopIndices; // mapping each canonical flop hand (2+3 cards) to a cluster
  static vector<int>
      turnIndices; // mapping each canonical turn hand (2+4 cards) to a cluster

  static vector<vector<float>> histogramsFlop;
  static vector<vector<float>> histogramsTurn;

  static const string filenameEMDTurnTable;
  static const string filenameEMDFlopTable;
  static const string filenameEMDFlopHistogram;
  static const string filenameEMDTurnHistogram;

  static void Init();
  static void SaveToFile();
  static void LoadFromFile();

private:
  static void GenerateTurnHistograms();
  static void GenerateFlopHistograms();
  static void ClusterTurn();
  static void ClusterFlop();
};
} // namespace poker
#endif
