#include "utils/utils.h"

namespace poker
{
    namespace utils
    {
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
            for (auto i = 0UL; i < probabilities.size(); ++i)
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
            for (auto i = 0UL; i < probabilities.size(); ++i)
            {
                sum += probabilities[i];
                if (sum >= rand)
                {
                    return i;
                }
            }
            return probabilities.size() - 1;
        }

        vector<Hand> GetStartingHandChart()
        {
            auto result = vector<Hand>();

            for (auto i = 0; i < Global::RANKS * Global::RANKS; ++i)
            {
                string firstCardRank = "";
                switch (i / Global::RANKS)
                {
                case 0:
                    firstCardRank = "2";
                    break;
                case 1:
                    firstCardRank = "3";
                    break;
                case 2:
                    firstCardRank = "4";
                    break;
                case 3:
                    firstCardRank = "5";
                    break;
                case 4:
                    firstCardRank = "6";
                    break;
                case 5:
                    firstCardRank = "7";
                    break;
                case 6:
                    firstCardRank = "8";
                    break;
                case 7:
                    firstCardRank = "9";
                    break;
                case 8:
                    firstCardRank = "T";
                    break;
                case 9:
                    firstCardRank = "J";
                    break;
                case 10:
                    firstCardRank = "Q";
                    break;
                case 11:
                    firstCardRank = "K";
                    break;
                case 12:
                    firstCardRank = "A";
                    break;
                }
                string secondCardRank = "";
                switch (i % Global::RANKS)
                {
                case 0:
                    secondCardRank = "2";
                    break;
                case 1:
                    secondCardRank = "3";
                    break;
                case 2:
                    secondCardRank = "4";
                    break;
                case 3:
                    secondCardRank = "5";
                    break;
                case 4:
                    secondCardRank = "6";
                    break;
                case 5:
                    secondCardRank = "7";
                    break;
                case 6:
                    secondCardRank = "8";
                    break;
                case 7:
                    secondCardRank = "9";
                    break;
                case 8:
                    secondCardRank = "T";
                    break;
                case 9:
                    secondCardRank = "J";
                    break;
                case 10:
                    secondCardRank = "Q";
                    break;
                case 11:
                    secondCardRank = "K";
                    break;
                case 12:
                    secondCardRank = "A";
                    break;
                }
                string firstCardSuit = "s";
                string secondCardSuit;
                if (i % Global::RANKS > i / Global::RANKS)
                {
                    secondCardSuit = "s";
                }
                else
                {
                    secondCardSuit = "h";
                }
                Hand hand = Hand();
                hand.cards.push_back(Card(firstCardRank + firstCardSuit));
                hand.cards.push_back(Card(secondCardRank + secondCardSuit));
                result.push_back(hand);
            }
            return result;
        }

        bool FileExists(const string &filename)
        {
            return filename.size() && access(filename.c_str(), F_OK) != -1;
        }
    } // namespace utils
} // namespace poker
