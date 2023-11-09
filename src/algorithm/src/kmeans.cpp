#include "algorithm/kmeans.h"

using namespace std;
using namespace indicators;

vector<int> Kmeans::ClusterEMD(vector<vector<float>> &data, int k, int nofRuns, vector<int> &_bestCenters)
{
    std::cout << "K-means++ (EMD) clustering " << data.size() << " elements into " << k << " clusters with " << nofRuns << " runs..." << std::endl;
    int filenameId = randint(0, 10000000);
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

    double recordDistance = DBL_MAX;

    for (int run = 0; run < nofRuns; ++run)
    {
        auto centers = vector<vector<float>>(k, vector<float>(data[0].size()));

        std::cout << "K-means++ starting clustering..." << std::endl;
        double lastDistance = DBL_MAX;
        bool distanceChanged = true;

        if (!skipInit)
        {
            bestCenters = vector<int>(data.size());
            centers = FindStartingCentersEMD(data, k);
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
            CalculateClusterDistancesEMD(centerCenterDistances, centers);

            // find closest cluster for each element
            auto threadDistance = vector<long>(Global::NOF_THREADS);
            utils::parallelise(data.size(),
                               [&](int threadIdx, int itemIdx)
                               {
                                    double distance = GetEarthMoverDistance(data, centers, itemIdx, bestCenters[itemIdx]);
                                    int bestIndex = bestCenters[itemIdx];
                                    for (int m = 0; m < k; m++) // go through centers
                                    {
                                        if (centerCenterDistances[bestIndex][m] < 2 * distance && bestIndex != m)
                                        {
                                            double tempDistance = GetEarthMoverDistance(data, centers, itemIdx, m);
                                            if (tempDistance < distance)
                                            {
                                                distance = tempDistance;
                                                bestIndex = m;
                                            }
                                        }
                                    }
                                    bestCenters[itemIdx] = bestIndex;
                                    threadDistance[threadIdx] += distance; });
            double totalDistance = accumulate(threadDistance.begin(), threadDistance.end(), 0L);

            centers = CalculateNewCenters(data, bestCenters, k);
            totalDistance = totalDistance / data.size();
            distanceChanged = !(totalDistance == lastDistance);

            double diff = lastDistance - totalDistance;

            std::cout << "Saving intermediate table to file..." << std::endl;

            string filename = "EMDTable_temp_";
            filename += to_string(filenameId);
            filename += ".txt";
            // FileHandler.SaveToFile(recordCenters, filename);

            if (totalDistance < recordDistance)
            {
                recordDistance = totalDistance;
                recordCenters = vector<int>(bestCenters);
            }

            std::cout << "Current average distance: " << totalDistance
                      << " Improvement: " << diff << ", "
                      << 100.0 * (1.0 - totalDistance / lastDistance) << "%" << std::endl;

            lastDistance = totalDistance;
        }
    }
    std::cout << "Best distance found: " << recordDistance << std::endl;
    chrono::steady_clock::time_point end = chrono::steady_clock::now();
    auto elapsed = chrono::duration_cast<std::chrono::seconds>(end - start).count();
    std::cout << "Time taken to generate lookup table: " << elapsed << "[s]" << std::endl;
    // print starting hand chart
    return recordCenters;
}

vector<int> Kmeans::ClusterL2(vector<vector<float>> &data, int k, int nofRuns, vector<int> &_bestCenters)
{
    std::cout << "K-means++ clustering (L2) "
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
    double recordDistance = DBL_MAX;

    for (int run = 0; run < nofRuns; ++run)
    {
        auto centers = vector<vector<float>>(k, vector<float>(data[0].size()));

        std::cout << "K-means++ starting clustering..." << std::endl;
        double lastDistance = DBL_MAX;
        bool distanceChanged = true;

        if (!skipInit)
        {
            bestCenters = vector<int>(data.size());
            centers = FindStartingCentersL2(data, k);
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
            CalculateClusterDistancesL2(centerCenterDistances, centers);

            // find closest cluster for each element
            atomic<long> sharedLoopCounter = 0;
            atomic<double> totalDistance = 0;

            indicators::show_console_cursor(false);
            BlockProgressBar bar{
                option::BarWidth{80},
                option::Start{"["},
                option::End{"]"},
                option::ForegroundColor{Color::white},
                option::FontStyles{std::vector<FontStyle>{FontStyle::bold}},
                option::ShowElapsedTime{true},
                option::ShowRemainingTime{true},
                option::MaxProgress{data.size()}};

            oneapi::tbb::parallel_for(0, Global::NOF_THREADS,
                                      [&](int i)
                                      {
                                          double threadDistance = 0;
                                          long iter = 0;
                                          auto [startItemIdx, endItemIdx] = utils::GetWorkItemsIndices(data.size(), Global::NOF_THREADS, i);

                                          for (int j = startItemIdx; j < endItemIdx; ++j)
                                          { // go through all data
                                              // assume previous cluster was good, this is better for the triangle inequality
                                              double distance = GetL2DistanceSquared(data, centers, j, bestCenters[j]);
                                              int bestIndex = bestCenters[j];

                                              for (int m = 0; m < k; m++) // go through centers
                                              {
                                                  if (centerCenterDistances[bestIndex][m] < 2 * (float)sqrt(distance) && bestIndex != m)
                                                  {
                                                      double tempDistance = GetL2DistanceSquared(data, centers, j, m);
                                                      if (tempDistance < distance)
                                                      {
                                                          distance = tempDistance;
                                                          bestIndex = m;
                                                      }
                                                  }
                                              }
                                              bestCenters[j] = bestIndex;
                                              threadDistance += sqrt(distance);

                                              iter++;
                                              if (iter % 100 == 0)
                                              {
                                                  sharedLoopCounter += 100;
                                                  bar.set_progress(sharedLoopCounter);
                                                  double expectedTotalDistance = atomic_load(&totalDistance);
                                                  while (!totalDistance.compare_exchange_weak(expectedTotalDistance, expectedTotalDistance + threadDistance))
                                                      ;
                                                  threadDistance = 0;
                                              }
                                          }
                                          sharedLoopCounter += iter % 100;
                                          bar.set_progress(sharedLoopCounter);
                                          double expectedTotalDistance = atomic_load(&totalDistance);
                                          while (!totalDistance.compare_exchange_weak(expectedTotalDistance, expectedTotalDistance + threadDistance))
                                              ;
                                      });

            centers = CalculateNewCenters(data, bestCenters, k);
            totalDistance = totalDistance / data.size();
            distanceChanged = !(totalDistance == lastDistance);
            double diff = lastDistance - totalDistance;

            std::cout << "Current average distance: " << totalDistance
                      << " Improvement: " << diff
                      << ", " << 100.0 * (1.0 - totalDistance / lastDistance) << "%"
                      << std::endl;

            // std::cout << "Saving intermediate table to file..." << std::endl;
            // ofstream file("OCHSRiverClusters_temp.txt");
            // boost::archive::binary_oarchive archive(file);
            // archive << recordCenters;

            if (totalDistance < recordDistance)
            {
                recordDistance = totalDistance;
                recordCenters = vector<int>(bestCenters);
            }

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

vector<vector<float>> Kmeans::CalculateNewCenters(vector<vector<float>> &data, vector<int> &bestCenters, int k)
{
    auto centers = vector<vector<float>>(k, vector<float>(data[0].size()));
    auto occurrences = vector<int>(k);
    for (int j = 0; j < data.size(); j++)
    {
        for (int m = 0; m < data[0].size(); ++m)
        {
            centers[bestCenters[j]][m] += data[j][m];
        }
        occurrences[bestCenters[j]]++;
    }
    for (int n = 0; n < k; ++n)
    {
        for (int m = 0; m < data[0].size(); ++m)
        {
            if (occurrences[n] != 0)
                centers[n][m] /= occurrences[n];
            else
                break;
        }
    }
    return centers;
}

void Kmeans::CalculateClusterDistancesL2(vector<vector<float>> &distances, vector<vector<float>> &clusterCenters)
{
    utils::parallelise(clusterCenters.size(),
                       [&](int threadIdx, int itemIdx)
                       {
                           for (int m = 0; m < itemIdx; ++m)
                           {
                               distances[itemIdx][m] = (float)sqrt(GetL2DistanceSquared(clusterCenters, clusterCenters, itemIdx, m));
                           }
                       });
    for (int j = 0; j < clusterCenters.size(); ++j)
    {
        for (int m = 0; m < j; ++m)
        {
            distances[m][j] = distances[j][m];
        }
    }
}

void Kmeans::CalculateClusterDistancesEMD(vector<vector<float>> &distances, vector<vector<float>> &clusterCenters)
{
    utils::parallelise(clusterCenters.size(),
                       [&](int threadIdx, int itemIdx)
                       {
                           for (int m = 0; m < itemIdx; ++m)
                           {
                               distances[itemIdx][m] = GetEarthMoverDistance(clusterCenters, clusterCenters, itemIdx, m);
                           }
                       });
    for (int j = 0; j < clusterCenters.size(); ++j)
    {
        for (int m = 0; m < j; ++m)
        {
            distances[m][j] = distances[j][m];
        }
    }
}

// /// <summary>
// /// Returns a sample of the data
// /// </summary>
// /// <param name="data"></param>
// /// <param name="nofSamples"></param>
// /// <returns></returns>
vector<vector<float>> Kmeans::GetUniqueRandomNumbers(vector<vector<float>> &data, int nofSamples)
{
    auto tempData = vector<vector<float>>(nofSamples, vector<float>(data[0].size()));
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
            CopyArray(data, tempData, rand, destinationIndex);
            destinationIndex++;
        }
    }
    return tempData;
}

vector<vector<float>> Kmeans::FindStartingCentersL2(vector<vector<float>> &data, int k)
{
    std::cout << "K-means++ finding good starting centers..." << std::endl;

    // first get some samples of all data to speed up the algorithm
    int maxSamples = min({k * 100, (int)data.size()});
    auto dataTemp = GetUniqueRandomNumbers(data, maxSamples);

    auto centers = vector<vector<float>>(k, vector<float>(data[0].size()));

    // first cluster center is random
    auto centerIndices = vector<int>();
    int index = -1;

    for (int c = 0; c < k; ++c) // get a new cluster center one by one
    {
        cout << "Finding center for " << c << "-th cluster" << endl;
        auto distancesToBestCenter = vector<double>(dataTemp.size(), DBL_MAX);

        if (c == 0)
        {
            index = randint(0, dataTemp.size());
            centerIndices.push_back(index);
            CopyArray(dataTemp, centers, index, c);

            continue;
        }
        else
        {
            utils::parallelise(dataTemp.size(),
                               [&](int threadIdx, int itemIdx)
                               {
                                    for (int m = 0; m < c; ++m) // go through centers
                                    {
                                        double tempDistance = GetL2DistanceSquared(dataTemp, centers, itemIdx, m);
                                        if (tempDistance < distancesToBestCenter[itemIdx])
                                        {
                                            distancesToBestCenter[itemIdx] = tempDistance;
                                        }
                                } });
            double sum = accumulate(distancesToBestCenter.begin(), distancesToBestCenter.end(), 0.0);
            for (int p = 0; p < distancesToBestCenter.size(); ++p)
            {
                distancesToBestCenter[p] /= sum;
            }
            int centerIndexSample = utils::SampleDistribution(distancesToBestCenter);
            while (find(centerIndices.begin(), centerIndices.end(), centerIndexSample) != centerIndices.end())
            {
                centerIndexSample = utils::SampleDistribution(distancesToBestCenter);
            }
            CopyArray(dataTemp, centers, centerIndexSample, c);
            centerIndices.push_back(centerIndexSample);
        }
    }

    return centers;
}

vector<vector<float>> Kmeans::FindStartingCentersEMD(vector<vector<float>> &data, int k)
{
    // select random centers
    std::cout << "K-means++ finding " << k << " good starting centers..." << std::endl;

    // first get some samples of all data to speed up the algorithm
    int maxSamples = min({k * 20, (int)data.size()});
    auto dataTemp = GetUniqueRandomNumbers(data, maxSamples);

    auto centers = vector<vector<float>>(k, vector<float>(data[0].size()));

    // first cluster center is random
    auto centerIndices = vector<int>();
    int index = -1;

    for (int c = 0; c < k; ++c) // get a new cluster center one by one
    {
        auto distancesToBestCenter = vector<float>(dataTemp.size(), FLT_MAX);

        if (c == 0)
        {
            index = randint(0, dataTemp.size());
            centerIndices.push_back(index);
            CopyArray(dataTemp, centers, index, c);
        }
        else
        {
            utils::parallelise(dataTemp.size(),
                               [&](int threadIdx, int itemIdx)
                               {
                                    for (int m = 0; m < c; ++m) // go through centers
                                    {
                                        float tempDistance = GetEarthMoverDistance(dataTemp, centers, itemIdx, m);
                                        if (tempDistance < distancesToBestCenter[itemIdx])
                                        {
                                            distancesToBestCenter[itemIdx] = tempDistance;
                                        }
                                    } });

            SquareArray(distancesToBestCenter);
            float sum = sum = accumulate(distancesToBestCenter.begin(), distancesToBestCenter.end(), 0.0);
            for (int p = 0; p < distancesToBestCenter.size(); ++p)
            {
                distancesToBestCenter[p] /= sum;
            }
            int centerIndexSample = utils::SampleDistribution(distancesToBestCenter);
            while (find(centerIndices.begin(), centerIndices.end(), centerIndexSample) != centerIndices.end())
            {
                centerIndexSample = utils::SampleDistribution(distancesToBestCenter);
            }
            CopyArray(dataTemp, centers, centerIndexSample, c);
            centerIndices.push_back(centerIndexSample);
        }
    }

    return centers;
}

void Kmeans::SquareArray(vector<float> &a)
{
    for (int i = 0; i < a.size(); i++)
    {
        a[i] *= a[i];
    }
}

void Kmeans::CopyArray(vector<vector<float>> &dataSource, vector<vector<float>> &dataDestination, int indexSource, int indexDestination)
{
    // probably should use buffer.blockcopy (todo)
    for (int i = 0; i < dataSource[0].size(); ++i)
    {
        dataDestination[indexDestination][i] = dataSource[indexSource][i];
    }
}

float Kmeans::GetEarthMoverDistance(vector<vector<float>> &data, vector<vector<float>> &centers, int index1, int index2)
{
    float emd = 0, totalDistance = 0;
    for (int i = 0; i < data[0].size(); i++)
    {
        emd = (data[index1][i] + emd) - centers[index2][i];
        totalDistance += abs(emd);
    }
    return totalDistance;
}

float Kmeans::GetL2DistanceSquared(vector<vector<float>> &data, vector<vector<float>> &centers, int index1, int index2)
{
    double totalDistance = 0;
    for (int i = 0; i < data[0].size() - 4; i += 4)
    {
        for (int j = 0; j < 4; j++)
        {
            float diff = data[index1][i + j] - centers[index2][i + j];
            totalDistance += diff * diff;
        }
    }
    for (int i = data[0].size() - data[0].size() % 4; i < data[0].size(); i++) // if the histogram is not a multiple of 4
    {
        totalDistance += (data[index1][i] - centers[index2][i]) * (double)(data[index1][i] - centers[index2][i]);
    }
    return totalDistance;
}
