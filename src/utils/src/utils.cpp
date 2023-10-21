#include "utils/utils.h"

tuple<int, int> GetWorkItemsIndices(int dataCount, int threadCount, int threadIndex)
{
    int minItems = dataCount / threadCount;
    int extraItems = dataCount % threadCount;

    if (threadIndex < extraItems)
    {
        return {(minItems + 1) * threadIndex, (minItems + 1) * (threadIndex + 1)};
    }
    return {(minItems * threadIndex) + extraItems, (minItems * threadIndex) + extraItems + minItems};
}

int SampleDistribution(vector<float> &probabilities)
{
    double rand = randDouble();
    double sum = 0.0;
    for (int i = 0; i < probabilities.size(); ++i)
    {
        sum += probabilities[i];
        if (sum >= rand)
        {
            return i;
        }
    }
    return probabilities.size() - 1;
}

int SampleDistribution(vector<double> &probabilities)
{
    double rand = randDouble();
    double sum = 0.0;
    for (int i = 0; i < probabilities.size(); ++i)
    {
        sum += probabilities[i];
        if (sum >= rand)
        {
            return i;
        }
    }
    return probabilities.size() - 1;
}
