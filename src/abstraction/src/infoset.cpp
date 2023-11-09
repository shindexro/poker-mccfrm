#include "abstraction/infoset.h"

Infoset::Infoset() : regret(),
                     actionCounter()
{
}

Infoset::Infoset(int actions) : regret(actions),
                                actionCounter(actions)
{
}

vector<float> Infoset::CalculateStrategy()
{
    float sum = 0;
    auto moveProbs = vector<float>(regret.size());
    for (auto a = 0UL; a < regret.size(); ++a)
    {
        sum += max(0.0f, regret[a]);
    }
    for (auto a = 0UL; a < regret.size(); ++a)
    {
        if (sum > 0.00001)
        {
            moveProbs[a] = max(0.0f, regret[a]) / sum;
        }
        else
        {
            moveProbs[a] = 1.0f / regret.size();
        }
    }
    return moveProbs;
}

vector<float> Infoset::GetFinalStrategy()
{
    float sum = 0;
    auto moveProbs = vector<float>(regret.size());
    for (auto a = 0UL; a < regret.size(); ++a)
    {
        sum += actionCounter[a];
    }
    for (auto a = 0UL; a < regret.size(); ++a)
    {
        if (sum > 0.00001)
        {
            moveProbs[a] = actionCounter[a] / sum;
        }
        else
        {
            moveProbs[a] = 1.0f / regret.size();
        }
    }
    return moveProbs;
}
