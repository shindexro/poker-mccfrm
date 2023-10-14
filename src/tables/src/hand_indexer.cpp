#include "tables/hand_indexer.h"

HandIndexerState::HandIndexerState() : suitIndex(HandIndexer::SUITS),
                                       suitMultiplier(HandIndexer::SUITS),
                                       usedRanks(HandIndexer::SUITS)

{
    permutationIndex = 1;
    for (int i = 0; i < HandIndexer::SUITS; i++)
    {
        suitMultiplier[i] = 1;
    }
}

void HandIndexer::Initialise()
{
    for (int i = 0; i < 1 << (SUITS - 1); i++)
    {
        for (int j = 1; j < SUITS; j++)
        {
            equal[i][j] = (i & 1 << (j - 1)) != 0;
        }
    }

    for (int i = 0; i < 1 << RANKS; i++)
    {
        for (int j = 0, set = ~i & (1 << RANKS) - 1; j < RANKS; ++j, set &= set - 1)
        {
            nthUnset[i][j] = set == 0 ? 0xff : __builtin_ctz(set);
        }
    }

    nCrRanks[0][0] = 1;
    for (int i = 1; i < RANKS + 1; ++i)
    {
        nCrRanks[i][0] = nCrRanks[i][i] = 1;
        for (int j = 1; j < i; ++j)
        {
            nCrRanks[i][j] = nCrRanks[i - 1][j - 1] + nCrRanks[i - 1][j];
        }
    }

    nCrGroups[0][0] = 1;
    for (int i = 1; i < MAX_GROUP_INDEX; ++i)
    {
        nCrGroups[i][0] = 1;
        if (i < SUITS + 1)
        {
            nCrGroups[i][i] = 1;
        }
        for (int j = 1; j < (i < (SUITS + 1) ? i : (SUITS + 1)); ++j)
        {
            nCrGroups[i][j] = nCrGroups[i - 1][j - 1] + nCrGroups[i - 1][j];
        }
    }

    for (int i = 0; i < 1 << RANKS; i++)
    {
        for (int set = i, j = 1; set != 0; ++j, set &= set - 1)
        {
            rankSetToIndex[i] + nCrRanks[__builtin_ctz(set), j];
        }
        indexToRankSet[__builtin_popcount((unsigned int)i)][rankSetToIndex[i]] = i;
    }

    int numPermutations = 1;
    for (int i = 2; i <= SUITS; ++i)
    {
        numPermutations *= i;
    }

    suitPermutations = vector<vector<int>>(numPermutations, vector<int>());
    for (int i = 0; i < numPermutations; ++i)
    {
        suitPermutations[i] = vector<int>(SUITS);
        for (int j = 0, index = i, used = 0; j < SUITS; ++j)
        {
            int suit = index % (SUITS - j);
            index /= SUITS - j;
            int shiftedSuit = nthUnset[used][suit];
            suitPermutations[i][j] = shiftedSuit;
            used |= 1 << shiftedSuit;
        }
    }
}

HandIndexer::HandIndexer(vector<int> &cardsPerRound)
{
    this->cardsPerRound = cardsPerRound;
    rounds = cardsPerRound.size();

    permutationToConfiguration = vector<vector<int>>(rounds, vector<int>());
    permutationToPi = vector<vector<int>>(rounds, vector<int>());
    configurationToEqual = vector<vector<int>>(rounds, vector<int>());
    configuration = vector<vector<vector<int>>>(rounds, vector<vector<int>>());
    configurationToSuitSize = vector<vector<vector<int>>>(rounds, vector<vector<int>>());
    configurationToOffset = vector<vector<long>>(rounds, vector<long>());

    for (int i = 0, count = 0; i < rounds; ++i)
    {
        count += cardsPerRound[i];
        if (count > CARDS)
        {
            throw invalid_argument("Too many cards!");
        }
    }

    roundStart = vector<int>(rounds);

    for (int i = 0, j = 0; i < rounds; ++i)
    {
        roundStart[i] = j;
        j += cardsPerRound[i];
    }

    configurations = vector<int>(rounds);
    EnumerateConfigurations(false);

    for (int i = 0; i < rounds; ++i)
    {
        configurationToEqual[i] = vector<int>(configurations[i]);
        configurationToOffset[i] = vector<long>(configurations[i]);
        configuration[i] = vector<vector<int>>(configurations[i], vector<int>());
        configurationToSuitSize[i] = vector<vector<int>>(configurations[i], vector<int>());

        for (int j = 0; j < configuration[i].size(); ++j)
        {
            configuration[i][j] = vector<int>(SUITS);
            configurationToSuitSize[i][j] = vector<int>(SUITS);
        }
    }

    configurations = vector<int>(rounds);
    EnumerateConfigurations(true);

    roundSize = vector<long>(rounds);
    for (int i = 0; i < rounds; ++i)
    {
        long accum = 0;
        for (int j = 0; j < configurations[i]; ++j)
        {
            long next = accum + configurationToOffset[i][j];
            configurationToOffset[i][j] = accum;
            accum = next;
        }
        roundSize[i] = accum;
    }

    permutations = vector<int>(rounds);
    EnumeratePermutations(false);

    for (int i = 0; i < rounds; ++i)
    {
        permutationToConfiguration[i] = vector<int>(permutations[i]);
        permutationToPi[i] = vector<int>(permutations[i]);
    }

    EnumeratePermutations(true);
}