#ifndef __CLASS_KMEANS_H__
#define __CLASS_KMEANS_H__

#include "utils/random.h"
#include "utils/utils.h"
#include "abstraction/global.h"

#include <oneapi/tbb.h>
#include <vector>
#include <unordered_set>
#include <float.h>
#include <boost/serialization/vector.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <indicators/block_progress_bar.hpp>
#include <indicators/cursor_control.hpp>

using namespace std;

namespace poker
{
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
        vector<int> Cluster(
            function<float(vector<vector<float>>&, vector<vector<float>>&, int, int)> distanceFunc,
            vector<vector<float>> &data, int k, int nofRuns, vector<int> &_bestCenters);
        vector<int> ClusterEMD(vector<vector<float>> &data, int k, int nofRuns, vector<int> &_bestCenters);
        vector<int> ClusterL2(vector<vector<float>> &data, int k, int nofRuns, vector<int> &_bestCenters);

    private:
        inline static const float stopClusterImprovementThreshold = 1e-5;

        vector<vector<float>> CalculateNewCenters(vector<vector<float>> &data, vector<int> &bestCenters, int k);

        void CalculateClusterDistances(
            function<float(vector<vector<float>>&, vector<vector<float>>&, int, int)> distanceFunc,
            vector<vector<float>> &distances, vector<vector<float>> &clusterCenters);

        vector<vector<float>> FindStartingCenters(
            function<float(vector<vector<float>>&, vector<vector<float>>&, int, int)> distanceFunc,
            vector<vector<float>> &data, int k);

        static vector<vector<float>> GetRandomSubset(vector<vector<float>> &data, int nofSamples);
        static void SquareArray(vector<float> &a);
        static void CopyArray(vector<vector<float>> &dataSource, vector<vector<float>> &dataDestination, int indexSource, int indexDestination);

        static float GetEarthMoverDistance(vector<vector<float>> &data, vector<vector<float>> &centers, int index1, int index2);
        static float GetL2Distance(vector<vector<float>> &data, vector<vector<float>> &centers, int index1, int index2);
    };
} // namespace poker
#endif
