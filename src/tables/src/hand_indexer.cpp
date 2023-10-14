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

void HandIndexer::CreatePublicFlopHands()
{
    cout << "Creating canonical samples of the 1755 public flop hand combinations..." << endl;

    vector<bool> publicFlopHandsFound(roundSize[0]);
    publicFlopHands = vector<vector<int>>(roundSize[0], vector<int>());
    for (int i = 0; i < roundSize[0]; i++)
    {
        publicFlopHands[i] = vector<int>(cardsPerRound[0]);
    }

    for (int card1 = 0; card1 < 52; card1++)
    {
        for (int card2 = 0; card2 < 52; card2++)
        {
            for (int card3 = 0; card3 < 52; card3++)
            {
                if (card1 != card2 && card2 != card3 && card1 != card3)
                {
                    vector<int> flop = vector<int>({card1, card2, card3});
                    long index = IndexLast(flop);

                    if (!publicFlopHandsFound[index])
                    {
                        publicFlopHandsFound[index] = true;
                        publicFlopHands[index] = vector<int>({card1, card2, card3});
                    }
                }
            }
        }
    }
}

/**
 * Index a hand on every round. This is not more expensive than just indexing the last round.
 *
 * @param cards
 * @param indices an array where the indices for every round will be saved to
 * @return hands index on the last round
 */
long HandIndexer::IndexAll(vector<int> &cards, vector<long> &indices)
{
    if (rounds > 0)
    {
        HandIndexerState state = HandIndexerState();
        for (int i = 0; i < rounds; i++)
        {
            indices[i] = IndexNextRound(state, cards);
        }
        return indices[rounds - 1];
    }
    return 0;
}

/**
 *  Index a hand on the last round.
 *
 * @param cards
 * @return hand's index on the last round
 */
long HandIndexer::IndexLast(vector<int> &cards)
{
    vector<long> indices(rounds);
    return IndexAll(cards, indices);
}

/**
 * Incrementally index the next round.
 *
 * @param state
 * @param cards the cards for the next round only!
 * @return hand's index on the latest round
 */
long HandIndexer::IndexNextRound(HandIndexerState &state, vector<int> &cards)
{
    int round = state.round++;

    vector<int> ranks(SUITS);
    vector<int> shiftedRanks(SUITS);

    for (int i = 0, j = roundStart[round]; i < cardsPerRound[round]; ++i, ++j)
    {
        int rank = cards[j] >> 2;
        int suit = cards[j] & 3;
        int rankBit = 1 << rank;

        ranks[suit] |= rankBit;
        shiftedRanks[suit] |= (rankBit >> __builtin_popcount((unsigned int)((rankBit - 1) & state.usedRanks[suit])));
    }

    for (int i = 0; i < SUITS; i++)
    {
        int usedSize = __builtin_popcount((unsigned int)state.usedRanks[i]);
        int thisSize = __builtin_popcount((unsigned int)ranks[i]);

        state.suitIndex[i] += state.suitMultiplier[i] * rankSetToIndex[shiftedRanks[i]];
        state.suitMultiplier[i] *= nCrRanks[RANKS - usedSize][thisSize];
        state.usedRanks[i] |= ranks[i];
    }

    for (int i = 0, remaining = cardsPerRound[round]; i < SUITS - 1; ++i)
    {
        int thisSize = __builtin_popcount((unsigned int)ranks[i]);
        state.permutationIndex += state.permutationMultiplier * thisSize;
        state.permutationMultiplier *= remaining + 1;
        remaining -= thisSize;
    }

    int configuration = permutationToConfiguration[round][state.permutationIndex];
    int piIndex = permutationToPi[round][state.permutationIndex];
    int equalIndex = configurationToEqual[round][configuration];
    long offset = configurationToOffset[round][configuration];
    vector<int> pi = suitPermutations[piIndex];

    vector<int> suitIndex(SUITS);
    vector<int> suitMultiplier(SUITS);
    for (int i = 0; i < SUITS; ++i)
    {
        suitIndex[i] = state.suitIndex[pi[i]];
        suitMultiplier[i] = state.suitMultiplier[pi[i]];
    }

    long index = offset;
    long multiplier = 1;
    for (int i = 0; i < SUITS;)
    {
        long part, size;
        if (i + 1 < SUITS && equal[equalIndex][i + 1])
        {
            if (i + 2 < SUITS && equal[equalIndex][i + 2])
            {
                if (i + 3 < SUITS && equal[equalIndex][i + 3])
                {
                    Swap(suitIndex, i, i + 1);
                    Swap(suitIndex, i + 2, i + 3);
                    Swap(suitIndex, i, i + 2);
                    Swap(suitIndex, i + 1, i + 3);
                    Swap(suitIndex, i + 1, i + 2);
                    part = suitIndex[i];
                    part += nCrGroups[suitIndex[i + 1] + 1][2];
                    part += nCrGroups[suitIndex[i + 2] + 2][3];
                    part += nCrGroups[suitIndex[i + 3] + 3][4];
                    size = nCrGroups[suitMultiplier[i] + 3][4];
                    i += 4;
                }
                else
                {
                    Swap(suitIndex, i, i + 1);
                    Swap(suitIndex, i, i + 2);
                    Swap(suitIndex, i + 1, i + 2);
                    part = suitIndex[i];
                    part += nCrGroups[suitIndex[i + 1] + 1][2];
                    part += nCrGroups[suitIndex[i + 2] + 2][3];
                    size = nCrGroups[suitMultiplier[i] + 2][3];
                    i += 3;
                }
            }
            else
            {
                Swap(suitIndex, i, i + 1);
                part = suitIndex[i];
                part += nCrGroups[suitIndex[i + 1] + 1][2];
                size = nCrGroups[suitMultiplier[i] + 1][2];
                i += 2;
            }
        }
        else
        {
            part = suitIndex[i];
            size = suitMultiplier[i];
            i + 1;
        }
        index += multiplier * part;
        multiplier *= size;
    }
    return index;
}

/**
 * Recover the canonical hand from a particular index.
 *
 * @param round
 * @param index
 * @param cards
 * @return true if successful
 */
bool HandIndexer::Unindex(int round, long index, vector<int> &cards)
{
    if (round >= rounds || index >= roundSize[round])
        return false;

    int low = 0;
    int high = configurations[round];
    int configurationIdx = 0;

    while ((unsigned int)low < (unsigned int)high)
    {
        int mid = ((low + high) / 2);
        if (configurationToOffset[round][mid] <= index)
        {
            configurationIdx = mid;
            low = mid + 1;
        }
        else
        {
            high = mid;
        }
    }
    index -= configurationToOffset[round][configurationIdx];

    vector<long> suitIndex(SUITS);
    for (int i = 0; i < SUITS;)
    {
        int j = i + 1;
        while (j < SUITS && configuration[round][configurationIdx][j] == configuration[round][configurationIdx][i])
        {
            j++;
        }

        int suitSize = configurationToSuitSize[round][configurationIdx][i];
        long groupSize = nCrGroups[suitSize + j - i - 1][j - 1];
        long groupIndex = (long)((unsigned long)index % (unsigned long)groupSize);

        index = (long)((unsigned long)index / (unsigned long)groupSize);
        for (; i < j - 1; ++i)
        {
            suitIndex[i] = (int)floor(exp(log(groupIndex) / (j - i) - 1 + log(j - i)) - j - i);
            low = (int)floor(exp(log(groupIndex) / (j - i) - 1 + log(j - i)) - j - i);
            high = (int)ceil(exp(log(groupIndex) / (j - i) + log(j - i)) - j + i + 1);
            if ((uint)high > (uint)suitSize)
            {
                high = suitSize;
            }
            if ((uint)high <= (uint)low)
            {
                low = 0;
            }
            while ((uint)low < (uint)high)
            {
                int mid = (int)((uint)(low + high) / 2);
                if (nCrGroups[mid + j - i - 1][j - i] <= groupIndex)
                {
                    suitIndex[i] = mid;
                    low = mid + 1;
                }
                else
                {
                    high = mid;
                }
            }
            groupIndex -= nCrGroups[(suitIndex[i] + j - i - 1)][j - i];
        }

        suitIndex[i] = groupIndex;
        ++i;
    }

    vector<int> location(roundStart);

    for (int i = 0; i < SUITS; ++i)
    {
        int used = 0, m = 0;
        for (int j = 0; j < rounds; ++j)
        {
            int n = configuration[round][configurationIdx][i] >> (ROUND_SHIFT * (rounds - j - 1)) & ROUND_MASK;
            int roundSize = nCrRanks[RANKS - m][n];
            m += n;
            int roundIdx = (int)((ulong)suitIndex[i] % (ulong)roundSize);
            suitIndex[i] = (long)((ulong)suitIndex[i] / (ulong)roundSize);
            int shiftedCards = indexToRankSet[n][roundIdx], rankSet = 0;
            for (int k = 0; k < n; ++k)
            {
                int shiftedCard = shiftedCards & -shiftedCards;
                shiftedCards ^= shiftedCard;
                int card = nthUnset[used][__builtin_ctz(shiftedCard)];
                rankSet |= (1 << card);
                cards[location[j]++] = card << 2 | i;
            }
            used |= rankSet;
        }
    }
    return true;
}

void HandIndexer::Swap(vector<int> &suitIndex, int u, int v)
{
    if (suitIndex[u] > suitIndex[v])
    {
        int tmp = suitIndex[u];
        suitIndex[u] = suitIndex[v];
        suitIndex[v] = tmp;
    }
}

void HandIndexer::EnumerateConfigurations(bool tabulate)
{
    vector<int> used(SUITS);
    vector<int> configuration(SUITS);

    EnumerateConfigurationsR(0, cardsPerRound[0], 0, ((1 << SUITS) - 2), used, configuration,
                             tabulate);
}

void HandIndexer::EnumerateConfigurationsR(int round, int remaining, int suit, int equal, vector<int> &used, vector<int> &configuration, bool tabulate)
{
    if (suit == SUITS)
    {
        if (tabulate)
            TabulateConfigurations(round, configuration);
        else
            ++configurations[round];

        if (round + 1 < rounds)
        {
            EnumerateConfigurationsR(round + 1, cardsPerRound[round + 1], 0, equal, used,
                                     configuration, tabulate);
        }
    }
    else
    {
        int min = 0;
        if (suit == SUITS - 1)
        {
            min = remaining;
        }

        int max = RANKS - used[suit];
        if (remaining < max)
        {
            max = remaining;
        }

        int previous = RANKS + 1;
        bool wasEqual = (equal & 1 << suit) != 0;
        if (wasEqual)
        {
            previous = configuration[suit - 1] >> (ROUND_SHIFT * (rounds - round - 1)) & ROUND_MASK;
            if (previous < max)
            {
                max = previous;
            }
        }

        int oldConfiguration = configuration[suit], oldUsed = used[suit];
        for (int i = min; i <= max; ++i)
        {
            int newConfiguration = oldConfiguration | i << (ROUND_SHIFT * (rounds - round - 1));
            int newEqual = ((equal & ~(1 << suit)) | (wasEqual & (i == previous) ? 1 : 0) << suit);

            used[suit] = oldUsed + i;
            configuration[suit] = newConfiguration;
            EnumerateConfigurationsR(round, remaining - i, suit + 1, newEqual, used, configuration,
                                     tabulate);
            configuration[suit] = oldConfiguration;
            used[suit] = oldUsed;
        }
    }
}

void HandIndexer::TabulateConfigurations(int round, vector<int> &configuration)
{
    int id = configurations[round]++;

    for (; id > 0; --id)
    {
        for (int i = 0; i < SUITS; ++i)
        {
            if (configuration[i] < this->configuration[round][id - 1][i])
            {
                break;
            }
            else if (configuration[i] > this->configuration[round][id - 1][i])
            {
                goto OUT;
            }
        }
        for (int i = 0; i < SUITS; ++i)
        {
            this->configuration[round][id][i] = this->configuration[round][id - 1][i];
            configurationToSuitSize[round][id][i] = configurationToSuitSize[round][id - 1][i];
        }
        configurationToOffset[round][id] = configurationToOffset[round][id - 1];
        configurationToEqual[round][id] = configurationToEqual[round][id - 1];
    }

OUT:

    configurationToOffset[round][id] = 1;
    for (int i = 0; i < SUITS; i++)
    {
        this->configuration[round][id][i] = configuration[i];
    }

    int equal = 0;
    for (int i = 0; i < SUITS;)
    {
        int size = 1;
        int j = 0;
        for (int remaining = RANKS; j <= round; ++j)
        {
            int ranks = configuration[i] >> (ROUND_SHIFT * (rounds - j - 1)) & ROUND_MASK;
            size *= nCrRanks[remaining][ranks];
            remaining -= ranks;
        }

        j = i + 1;
        while (j < SUITS && configuration[j] == configuration[i])
        {
            ++j;
        }

        for (int k = i; k < j; ++k)
        {
            configurationToSuitSize[round][id][k] = size;
        }

        configurationToOffset[round][id] *= nCrGroups[size + j - i - 1][j - i];

        for (int k = i + 1; k < j; ++k)
        {
            equal |= 1 << k;
        }

        i = j;
    }

    configurationToEqual[round][id] = equal >> 1;
}

void HandIndexer::EnumeratePermutations(bool tabulate)
{
    vector<int> used(SUITS);
    vector<int> count(SUITS);

    EnumeratePermutationsR(0, cardsPerRound[0], 0, used, count, tabulate);
}

void HandIndexer::EnumeratePermutationsR(int round, int remaining, int suit, vector<int> &used, vector<int> &count, bool tabulate)
{
    if (suit == SUITS)
    {
        if (tabulate)
        {
            TabulatePermutations(round, count);
        }
        else
        {
            CountPermutations(round, count);
        }

        if (round + 1 < rounds)
        {
            EnumeratePermutationsR(round + 1, cardsPerRound[round + 1], 0, used, count, tabulate);
        }
    }
    else
    {
        int min = 0;
        if (suit == SUITS - 1)
        {
            min = remaining;
        }

        int max = RANKS - used[suit];
        if (remaining < max)
        {
            max = remaining;
        }

        int oldCount = count[suit], oldUsed = used[suit];
        for (int i = min; i <= max; ++i)
        {
            int newCount = oldCount | i << (ROUND_SHIFT * (rounds - round - 1));

            used[suit] = oldUsed + i;
            count[suit] = newCount;
            EnumeratePermutationsR(round, remaining - i, suit + 1, used, count, tabulate);
            count[suit] = oldCount;
            used[suit] = oldUsed;
        }
    }
}

void HandIndexer::CountPermutations(int round, vector<int> &count)
{
    int idx = 0, mult = 1;
    for (int i = 0; i <= round; ++i)
    {
        for (int j = 0, remaining = cardsPerRound[i]; j < SUITS - 1; ++j)
        {
            int size = count[j] >> ((rounds - i - 1) * ROUND_SHIFT) & ROUND_MASK;
            idx += mult * size;
            mult *= remaining + 1;
            remaining -= size;
        }
    }

    if (permutations[round] < idx + 1)
    {
        permutations[round] = idx + 1;
    }
}

void HandIndexer::TabulatePermutations(int round, vector<int> &count)
{
    int idx = 0, mult = 1;
    for (int i = 0; i <= round; ++i)
    {
        for (int j = 0, remaining = cardsPerRound[i]; j < SUITS - 1; ++j)
        {
            int size = count[j] >> ((rounds - i - 1) * ROUND_SHIFT) & ROUND_MASK;
            idx += mult * size;
            mult *= remaining + 1;
            remaining -= size;
        }
    }

    vector<int> pi(SUITS);
    for (int i = 0; i < SUITS; ++i)
    {
        pi[i] = i;
    }

    for (int i = 1; i < SUITS; ++i)
    {
        int j = i, pi_i = pi[i];
        for (; j > 0; --j)
        {
            if (count[pi_i] > count[pi[j - 1]])
            {
                pi[j] = pi[j - 1];
            }
            else
            {
                break;
            }
        }
        pi[j] = pi_i;
    }

    int pi_idx = 0, pi_mult = 1, pi_used = 0;
    for (int i = 0; i < SUITS; ++i)
    {
        int this_bit = (1 << pi[i]);
        int smaller = __builtin_popcount((uint)((this_bit - 1) & pi_used));
        pi_idx += (pi[i] - smaller) * pi_mult;
        pi_mult *= SUITS - i;
        pi_used |= this_bit;
    }

    permutationToPi[round][idx] = pi_idx;

    int low = 0, high = configurations[round];
    while (low < high)
    {
        int mid = (low + high) / 2;

        int compare = 0;
        for (int i = 0; i < SUITS; ++i)
        {
            int that = count[pi[i]];
            int other = configuration[round][mid][i];
            if (other > that)
            {
                compare = -1;
                break;
            }
            else if (other < that)
            {
                compare = 1;
                break;
            }
        }

        if (compare == -1)
        {
            high = mid;
        }
        else if (compare == 0)
        {
            low = high = mid;
        }
        else
        {
            low = mid + 1;
        }
    }

    permutationToConfiguration[round][idx] = low;
}
