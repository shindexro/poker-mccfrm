#ifndef __CLASS_EHS_TABLE_H__
#define __CLASS_EHS_TABLE_H__

#include <iostream>
#include <string.h>
#include <string>

#define NUM_ISOMORPHIC_FLOP 1286792
#define NUM_ISOMORPHIC_TURN 13960050

using namespace std;

namespace poker {
class EHSTable {
public:
  int tableFlop[3][3];
  int tableTurn[3][3];
  static float EHSFlop[NUM_ISOMORPHIC_FLOP];
  static float EHSTurn[NUM_ISOMORPHIC_TURN];
  static float histogramsFlop[NUM_ISOMORPHIC_FLOP];

  EHSTable();

private:
  static string EHSTable5Cards;
  static string EHSTable6Cards;

  void ClusterFlop();
  void Generate5CardsTable();
  void Generate6CardsTable();
  void CalculateFlopHistograms();
  static void SaveToFile();
  static void LoadFromFile();
};
} // namespace poker

#endif
