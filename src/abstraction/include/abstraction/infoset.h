#ifndef __CLASS_INFOSET_H__
#define __CLASS_INFOSET_H__

#include <vector>
#include <string>

using namespace std;

class Infoset
{
public:
    vector<float> regret;
    vector<float> actionCounter;

    Infoset();
    Infoset(int actions);

    vector<float> CalculateStrategy();
    vector<float> GetFinalStrategy();
};

#endif
