#include "tables/evaluator.h"

Evaluator::Evaluator()
{
    string fileName = "HandValueTable.txt";
    double loadFactor = 6.0;

    if (loaded)
        return;
    if (monteCarloMap.size())
        return;

    bool fiveCards = true;
    bool sixCards = true;
    bool sevenCards = true;

    chrono::steady_clock::time_point start = chrono::steady_clock::now();

    // Load hand rank table or create one if no file was given
    if (fileName.size() && access(fileName.c_str(), F_OK) != -1)
    {
        cout << "Loading table from " << fileName << endl;
        LoadFromFile(fileName);
    }
    else
    {
        int minHashMapSize = (fiveCards ? 2598960 : 0) + (sixCards ? 20358520 : 0) + (sevenCards ? 133784560 : 0);
        handRankMap = unordered_map<ulong, ulong>((uint)(minHashMapSize * loadFactor));
        if (fiveCards)
        {
            GenerateFiveCardTable();
        }
        if (sixCards)
        {
            cout << "Generating new six card lookup table (52C6 = 20,358,520)" << endl;
            GenerateSixCardTable();
        }
        if (sevenCards)
        {
            cout << "Generating new seven card lookup table (52C7 = 133,784,560)" << endl;
            GenerateSevenCardTable();
        }

        // Console.WriteLine("Running monte carlo simulation");
        // GenerateMonteCarloMap(100000);
        cout << "Writing table to disk" << endl;
        SaveToFile(fileName);
    }

    chrono::steady_clock::time_point end = chrono::steady_clock::now();

    auto elapsed = chrono::duration_cast<std::chrono::seconds>(end - start).count();
    cout << "Time difference = " << elapsed << "[s]" << endl;
    loaded = true;
}

int Evaluator::Evaluate(ulong bitmap)
{
    // Check if 2-card monte carlo map has an evaluation for this hand
    // if (monteCarloMap.ContainsKey(bitmap)) return (int)monteCarloMap[bitmap];

    // Otherwise return the real evaluation
    return (int)handRankMap[bitmap];
}

void Evaluator::SaveToFile(string &fileName)
{
    using var fileStream = File.Create(fileName);
    BinaryFormatter bf = new BinaryFormatter();
    bf.Serialize(fileStream, handRankMap);
}

void Evaluator::LoadFromFile(string &fileName)
{
    using var fileStream = File.OpenRead(fileName);
    var binForm = new BinaryFormatter();
    handRankMap = (HashMap)binForm.Deserialize(fileStream);
}

void Evaluator::GenerateFiveCardTable()
{
    vector<int> combo(52);
    for (int i = 0; i < 52; i++)
        combo[i] = i;

    int comboSize = 5;
    auto handBitmaps = vector<ulong>();
    do
    {
        ulong bitmap;
        for (int i = 0; i < comboSize; i++)
            bitmap |= 1ul << combo[i];
    } while (next_combination(combo.begin(), combo.begin() + comboSize, combo.end(), less<int>()));

    // Calculate hand strength of each hand
    auto handStrengths = new unordered_map<ulong, HandStrength>();
    cout << "Calculating hand strength (" << handBitmaps.size() << ")" << endl;

    for (ulong bitmap : handBitmaps)
    {
        Hand hand = Hand(bitmap);
        handStrengths[bitmap] = hand.GetStrength();
    }

    // Generate a list of all unique hand strengths
    auto uniqueHandStrengths = vector<HandStrength>();
    cout << "Generating equivalence classes" << endl;

    for (auto [bitmap, strength] : handStrengths)
    {
        Utilities::BinaryInsert(uniqueHandStrengths, strength.Value);
    }

    cout << uniqueHandStrengths.size() << " unique hand strengths" << endl;

    // Create a map of hand bitmaps to hand strength indices
    cout << "Generating new five card lookup table (2,598,960)" << endl;

    for (ulong bitmap : handBitmaps)
    {
        Hand hand = Hand(bitmap);
        HandStrength strength = hand.GetStrength();
        auto equivalence = Utilities::BinarySearch(uniqueHandStrengths, strength);
        if (equivalence == null)
            throw invalid_argument(hand.ToString() + " hand not found");
        else
        {
            handRankMap[bitmap] = (ulong)equivalence;
        }
    }
}

void Evaluator::GenerateSixCardTable()
{
    vector<int> combo(52);
    for (int i = 0; i < 52; i++)
        combo[i] = i;

    int comboSize = 6;
    do
    {
        int subsetSize = comboSize - 1;
        auto subset = vector<int>(combo.begin(), combo.begin() + subsetSize);
        auto subsetValues = vector<ulong>();

        do
        {
            ulong subsetBitmap = 0ul;
            for (int card : subset)
            {
                subsetBitmap |= 1ul << card;
                subsetValues.push_back(handRankMap[subsetBitmap]);
            }
        } while (next_combination(subset.begin(), subset.begin() + subsetSize, subset.end(), less<int>()));

        ulong bitmap;
        for (int i = 0; i < comboSize; i++)
            bitmap |= 1ul << combo[i];

        handRankMap[bitmap] = *max_element(subsetValues.begin(), subsetValues.end());

    } while (next_combination(combo.begin(), combo.begin() + comboSize, combo.end(), less<int>()));
}

void Evaluator::GenerateSevenCardTable()
{
    vector<int> combo(52);
    for (int i = 0; i < 52; i++)
        combo[i] = i;

    int comboSize = 7;
    do
    {
        int subsetSize = comboSize - 1;
        auto subset = vector<int>(combo.begin(), combo.begin() + subsetSize);
        auto subsetValues = vector<ulong>();

        do
        {
            ulong subsetBitmap = 0ul;
            for (int card : subset)
            {
                subsetBitmap |= 1ul << card;
                subsetValues.push_back(handRankMap[subsetBitmap]);
            }
        } while (next_combination(subset.begin(), subset.begin() + subsetSize, subset.end(), less<int>()));

        ulong bitmap;
        for (int i = 0; i < comboSize; i++)
            bitmap |= 1ul << combo[i];

        handRankMap[bitmap] = *max_element(subsetValues.begin(), subsetValues.end());

    } while (next_combination(combo.begin(), combo.begin() + comboSize, combo.end(), less<int>()));
}

void Evaluator::GenerateMonteCarloMap(int iterations)
{
    monteCarloMap = unordered_map<ulong, ulong>();

    vector<int> combo(52);
    for (int i = 0; i < 52; i++)
    {
        combo[i] = i;
    }
    int count = 0;
    int comboSize = 2;

    do
    {
        cout << count++ << '\r' << endl;

        ulong bitmap = 0ul;
        for (int i = 0; i < comboSize; i++)
            bitmap |= 1ul << combo[i];

        Hand hand = Hand(bitmap);
        Deck deck = Deck(bitmap);

        ulong evaluationSum = 0;
        for (int i = 0; i < iterations; i++)
        {
            if (deck.NumRemainingCards() < 13)
                deck.Shuffle();
            evaluationSum += handRankMap[bitmap | deck.Draw(3)]; // TODO: this uses an old draw method
        }

        monteCarloMap[bitmap] = evaluationSum / (ulong)iterations;
    } while (next_combination(combo.begin(), combo.begin() + comboSize, combo.end(), less<int>()));

    for (auto [k, v] : monteCarloMap)
    {
        Hand hand = Hand(k);
        string end = "\t";
        hand.PrintColoredCards(end);
        cout << v << endl;
        handRankMap[k] = v;
    }
}

template <class RandIt, class Compare>
bool next_combination(RandIt first, RandIt mid, RandIt last, Compare comp)
{
    std::sort(mid, last, std::tr1::bind(comp, std::tr1::placeholders::_2, std::tr1::placeholders::_1));
    return std::next_permutation(first, last, comp);
}
