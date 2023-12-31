#include "algorithm/kmeans.h"

using namespace std;
using namespace indicators;

namespace poker
{

    // distanceFunc should be unsquared, positive
    vector<int> Kmeans::Cluster(
            function<float(vector<vector<float>>&, vector<vector<float>>&, int, int)> distanceFunc,
            vector<vector<float>> &data, int k, int nofRuns, vector<int> &_bestCenters)
    {
        std::cout << "K-means++ clustering "
                  << data.size() << " elements into "
                  << k << " clusters with "
                  << nofRuns << " runs...";

        chrono::steady_clock::time_point start = chrono::steady_clock::now();

        auto bestCenters = vector<int>(data.size());
        auto recordCenters = vector<int>(data.size()); // we return indices only, the centers are discarded

        // load previous indices if passed
        bool skipInit = false;
        if (_bestCenters.size())
        {
            skipInit = true;
            bestCenters = vector<int>(_bestCenters);
            recordCenters = vector<int>(_bestCenters);
        }
        float recordDistance = FLT_MAX;

        for (auto run = 0; run < nofRuns; ++run)
        {
            std::cout << "K-means++ starting clustering "
                << run << "/" << nofRuns << " runs..." << std::endl;
            
            auto centers = vector<vector<float>>(k, vector<float>(data[0].size()));
            float lastDistance = FLT_MAX;
            bool distanceChanged = true;

            if (!skipInit)
            {
                bestCenters = vector<int>(data.size());
                centers = FindStartingCenters(distanceFunc, data, k);
            }
            else
            {
                // find new cluster centers // todo: it isnt theoretically sound to take the mean when using EMD distance metric
                centers = CalculateNewCenters(data, bestCenters, k);
                skipInit = false;
            }

            auto centerCenterDistances = vector<vector<float>>(k, vector<float>(k));

            while (distanceChanged)
            {
                // calculate cluster-cluster distances to use triangle inequality
                CalculateClusterDistances(distanceFunc, centerCenterDistances, centers);

                // find closest cluster for each element
                auto threadDistance = vector<long>(Global::NOF_THREADS);
                auto threadFunc = [&](int threadIdx, int itemIdx)
                {
                    // assume previous cluster was good, this is better for the triangle inequality
                    float distance = distanceFunc(data, centers, itemIdx, bestCenters[itemIdx]);
                    int bestIndex = bestCenters[itemIdx];

                    for (auto m = 0; m < k; m++) // go through centers
                    {
                        if (centerCenterDistances[bestIndex][m] < 2 * distance && bestIndex != m)
                        {
                            float tempDistance = distanceFunc(data, centers, itemIdx, m);
                            if (tempDistance < distance)
                            {
                                distance = tempDistance;
                                bestIndex = m;
                            }
                        }
                    }
                    bestCenters[itemIdx] = bestIndex;
                    threadDistance[threadIdx] += distance;
                };
                utils::parallelise(data.size(), threadFunc);

                float totalDistance = accumulate(threadDistance.begin(), threadDistance.end(), 0.0L);
                totalDistance = totalDistance / data.size();

                centers = CalculateNewCenters(data, bestCenters, k);
                float diff = lastDistance - totalDistance;
                std::cout << "Current average distance: " << totalDistance
                          << " Improvement: " << diff
                          << ", " << 100.0 * (1.0 - totalDistance / lastDistance) << "%"
                          << std::endl;

                if (totalDistance < recordDistance)
                {
                    recordDistance = totalDistance;
                    recordCenters = vector<int>(bestCenters);
                }

                distanceChanged = abs(diff) / lastDistance < stopClusterImprovementThreshold;
                lastDistance = totalDistance;
            }
        }
        std::cout << "Best distance found: " << recordDistance << std::endl;
        chrono::steady_clock::time_point end = chrono::steady_clock::now();
        auto elapsed = chrono::duration_cast<std::chrono::seconds>(end - start).count();
        cout << "Time taken to generate lookup table: " << elapsed << "[s]" << std::endl;
        // print starting hand chart
        return recordCenters;
    }

    vector<int> Kmeans::ClusterEMD(vector<vector<float>> &data, int k, int nofRuns, vector<int> &_bestCenters)
    {
        return Cluster(GetEarthMoverDistance, data, k, nofRuns, _bestCenters);
    }

    vector<int> Kmeans::ClusterL2(vector<vector<float>> &data, int k, int nofRuns, vector<int> &_bestCenters)
    {
        return Cluster(GetL2Distance, data, k, nofRuns, _bestCenters);
    }

    vector<vector<float>> Kmeans::CalculateNewCenters(vector<vector<float>> &data, vector<int> &bestCenters, int k)
    {
        auto centers = vector<vector<float>>(k, vector<float>(data[0].size()));
        auto occurrences = vector<int>(k);
        for (auto j = 0UL; j < data.size(); j++)
        {
            for (auto m = 0UL; m < data[0].size(); ++m)
            {
                centers[bestCenters[j]][m] += data[j][m];
            }
            occurrences[bestCenters[j]]++;
        }
        for (auto n = 0; n < k; ++n)
        {
            for (auto m = 0UL; m < data[0].size(); ++m)
            {
                if (occurrences[n] != 0)
                    centers[n][m] /= occurrences[n];
                else
                    break;
            }
        }
        return centers;
    }

    void Kmeans::CalculateClusterDistances(
            function<float(vector<vector<float>>&, vector<vector<float>>&, int, int)> distanceFunc,
            vector<vector<float>> &distances, vector<vector<float>> &clusterCenters)
    {
        auto threadFunc = [&](int /*threadIdx*/, int itemIdx)
        {
            for (auto m = 0; m < itemIdx; ++m)
            {
                distances[itemIdx][m] = distanceFunc(clusterCenters, clusterCenters, itemIdx, m);
                distances[m][itemIdx] = distances[itemIdx][m];
            }
        };
        utils::parallelise(clusterCenters.size(), threadFunc);
    }

    vector<vector<float>> Kmeans::FindStartingCenters(
            function<float(vector<vector<float>>&, vector<vector<float>>&, int, int)> distanceFunc,
            vector<vector<float>> &data, int k)
    {
        std::cout << "K-means++ finding good starting centers..." << std::endl;

        // first get some samples of all data to speed up the algorithm
        static const int centerCandidateMultiplier = 100;
        int maxSamples = min({k * centerCandidateMultiplier, (int)data.size()});
        auto centerCandidates = GetRandomSubset(data, maxSamples);

        auto centers = vector<vector<float>>(k, vector<float>(data[0].size()));

        // first cluster center is randomly chosen
        auto centerIndices = vector<int>();
        int index = randint(0, centerCandidates.size());
        CopyArray(centerCandidates, centers, index, 0);
        centerIndices.push_back(index);

        for (auto c = 1; c < k; ++c)
        {
            cout << "Finding center for " << c << "-th cluster" << endl;
            auto distancesToBestCenter = vector<float>(centerCandidates.size(), FLT_MAX);

            auto findDistanceToBestCenter = [&](int /*threadIdx*/, int itemIdx)
            {
                for (auto m = 0; m < c; ++m)
                {
                    float tempDistance = distanceFunc(centerCandidates, centers, itemIdx, m);
                    tempDistance = tempDistance * tempDistance;
                    if (tempDistance < distancesToBestCenter[itemIdx])
                    {
                        distancesToBestCenter[itemIdx] = tempDistance;
                    }
                }
            };
            utils::parallelise(centerCandidates.size(), findDistanceToBestCenter);

            // kmean++, choose next center based on weighted probability on squared distance
            utils::normalise(distancesToBestCenter);

            int nextCenterIndex = utils::SampleDistribution(distancesToBestCenter);
            while (find(centerIndices.begin(), centerIndices.end(), nextCenterIndex) != centerIndices.end())
            {
                nextCenterIndex = utils::SampleDistribution(distancesToBestCenter);
            }
            CopyArray(centerCandidates, centers, nextCenterIndex, c);
            centerIndices.push_back(nextCenterIndex);
        }

        return centers;
    }
    
    // return a subnet of length nofSamples of data, without repeating elements
    vector<vector<float>> Kmeans::GetRandomSubset(vector<vector<float>> &data, int nofSamples)
    {
        auto subset = vector<vector<float>>(nofSamples, vector<float>(data[0].size()));
        unordered_set<int> numbers;

        int numbersLeft = nofSamples;
        int destinationIndex = 0;
        while (numbersLeft > 0)
        {
            int rand = randint(0, data.size());
            if (!numbers.count(rand))
            {
                numbers.insert(rand);
                numbersLeft--;
                CopyArray(data, subset, rand, destinationIndex);
                destinationIndex++;
            }
        }
        return subset;
    }

    void Kmeans::SquareArray(vector<float> &a)
    {
        for (auto i = 0UL; i < a.size(); i++)
        {
            a[i] *= a[i];
        }
    }

    void Kmeans::CopyArray(vector<vector<float>> &dataSource, vector<vector<float>> &dataDestination, int indexSource, int indexDestination)
    {
        for (auto i = 0UL; i < dataSource[0].size(); ++i)
        {
            dataDestination[indexDestination][i] = dataSource[indexSource][i];
        }
    }

    float Kmeans::GetEarthMoverDistance(vector<vector<float>> &data, vector<vector<float>> &centers, int index1, int index2)
    {
        float emd = 0, totalDistance = 0;
        for (auto i = 0UL; i < data[0].size(); i++)
        {
            // todo: how is this emd???
            emd = (data[index1][i] + emd) - centers[index2][i];
            totalDistance += abs(emd);
        }
        return totalDistance;
    }

    float Kmeans::GetL2Distance(vector<vector<float>> &data, vector<vector<float>> &centers, int index1, int index2)
    {
        float totalDistance = 0;
        for (auto i = 0UL; i < data[0].size(); i++)
        {
            float diff = data[index1][i] - centers[index2][i];
            totalDistance += diff * diff;
        }
        return sqrt(totalDistance);
    }
} // namespace poker
