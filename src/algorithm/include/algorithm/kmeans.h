#ifndef __CLASS_KMEANS_H__
#define __CLASS_KMEANS_H__

#include "utils/random.h"
#include "utils/utils.h"
#include "abstraction/global.h"

#include <oneapi/tbb.h>
#include <vector>
#include <unordered_set>
#include <float.h>

using namespace std;

/// <summary>
/// Cluster the elements in the input array into k distinct buckets and return them
/// </summary>
class Kmeans
{
public:
    Kmeans() {}

    /// <summary>
    /// Returns an array where the element at index i contains the cluster entry associated with the entry
    /// </summary>
    /// <param name="data"></param>
    /// <param name="k"></param>
    /// <returns></returns>
    vector<int> ClusterEMD(vector<vector<float>> &data, int k, int nofRuns, vector<int> &_bestCenters);
    vector<int> ClusterL2(vector<vector<float>> &data, int k, int nofRuns, vector<int> &_bestCenters);

private:
    vector<vector<float>> CalculateNewCenters(vector<vector<float>> &data, vector<int> &bestCenters, int k);
    void CalculateClusterDistancesL2(vector<vector<float>> &distances, vector<vector<float>> &clusterCenters);
    void CalculateClusterDistancesEMD(vector<vector<float>> &distances, vector<vector<float>> &clusterCenters);

    /// <summary>
    /// Returns a sample of the data
    /// </summary>
    /// <param name="data"></param>
    /// <param name="nofSamples"></param>
    /// <returns></returns>
    vector<vector<float>> GetUniqueRandomNumbers(vector<vector<float>> &data, int nofSamples);
    vector<vector<float>> FindStartingCentersL2(vector<vector<float>> &data, int k);
    vector<vector<float>> FindStartingCentersEMD(vector<vector<float>> &data, int k);
    static void SquareArray(vector<float> &a);
    void CopyArray(vector<vector<float>> &dataSource, vector<vector<float>> &dataDestination, int indexSource, int indexDestination);

    float GetEarthMoverDistance(vector<vector<float>> &data, vector<vector<float>> &centers, int index1, int index2);
    float GetL2DistanceSquared(vector<vector<float>> &data, vector<vector<float>> &centers, int index1, int index2);
};

#endif
