#ifndef __RANDON_H__
#define __RANDON_H__

#include "utils/random.h"
#include "game/hand.h"
#include "game/card.h"
#include "abstraction/global.h"

#include <tuple>
#include <vector>

using namespace std;

tuple<int, int> GetWorkItemsIndices(int dataCount, int threadCount, int threadIndex);
int SampleDistribution(vector<float> &probabilities);
int SampleDistribution(vector<double> &probabilities);

vector<Hand> GetStartingHandChart();

#endif