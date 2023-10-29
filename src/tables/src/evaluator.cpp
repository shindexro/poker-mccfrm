#include "tables/evaluator.h"

void Evaluator::Initialise()
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
            SaveToFile(fileName);
        }
        if (sixCards)
        {
            cout << "Generating new six card lookup table (52C6 = 20,358,520)" << endl;
            GenerateSixCardTable();
            SaveToFile(fileName);
        }
        if (sevenCards)
        {
            cout << "Generating new seven card lookup table (52C7 = 133,784,560)" << endl;
            GenerateSevenCardTable();
            SaveToFile(fileName);
        }

        // Console.WriteLine("Running monte carlo simulation");
        // GenerateMonteCarloMap(100000);
        cout << "Writing table to disk" << endl;
        SaveToFile(fileName);
    }

    chrono::steady_clock::time_point end = chrono::steady_clock::now();

    auto elapsed = chrono::duration_cast<std::chrono::seconds>(end - start).count();
    cout << "Time taken to generate lookup table: " << elapsed << "[s]" << endl;
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
    ofstream file(fileName);
    boost::archive::binary_oarchive archive(file);
    archive << handRankMap;
}

void Evaluator::LoadFromFile(string &fileName)
{
    ifstream file(fileName);
    boost::archive::binary_iarchive archive(file);
    archive >> handRankMap;
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
        ulong bitmap = 0ul;
        for (int i = 0; i < comboSize; i++)
            bitmap |= 1ul << combo[i];
        handBitmaps.push_back(bitmap);
    } while (next_combination(combo.begin(), combo.begin() + comboSize, combo.end()));

    // Calculate hand strength of each hand
    auto handStrengths = unordered_map<ulong, HandStrength>();
    cout << "Calculating hand strength (" << handBitmaps.size() << ")" << endl;

    for (auto bitmap : handBitmaps)
    {
        Hand hand = Hand(bitmap);
        handStrengths[bitmap] = hand.GetStrength();
    }

    // Generate a list of all unique hand strengths
    auto uniqueHandStrengths = vector<HandStrength>();
    cout << "Generating equivalence classes" << endl;

    for (auto [bitmap, strength] : handStrengths)
    {
        auto insert_it = lower_bound(uniqueHandStrengths.begin(), uniqueHandStrengths.end(), strength);
        if (insert_it == uniqueHandStrengths.end() || insert_it->Compare(strength) != 0)
        {
            uniqueHandStrengths.insert(insert_it, strength);
        }
    }

    cout << uniqueHandStrengths.size() << " unique hand strengths" << endl;

    // Create a map of hand bitmaps to hand strength indices
    cout << "Generating new five card lookup table (2,598,960)" << endl;

    for (ulong bitmap : handBitmaps)
    {
        Hand hand = Hand(bitmap);
        HandStrength strength = hand.GetStrength();
        auto equivalence = lower_bound(uniqueHandStrengths.begin(), uniqueHandStrengths.end(), strength);
        if (equivalence == uniqueHandStrengths.end() || equivalence->Compare(strength) != 0)
        {
            throw invalid_argument(hand.ToString() + " hand not found");
        }
        else
        {
            handRankMap[bitmap] = (ulong)(equivalence - uniqueHandStrengths.begin());
        }
    }
}

void Evaluator::GenerateSixCardTable()
{
    vector<int> combo(52);
    for (int i = 0; i < 52; i++)
        combo[i] = i;

    int comboSize = 6;

    using namespace indicators;
    indicators::show_console_cursor(false);
    ProgressBar bar{
        option::BarWidth{50},
        option::Start{"["},
        option::Fill{"="},
        option::Lead{">"},
        option::Remainder{" "},
        option::End{"]"},
        option::PostfixText{"Generating 6 card table"},
        option::ForegroundColor{Color::green},
        option::ShowElapsedTime{true},
        option::ShowRemainingTime{true},
        option::FontStyles{std::vector<FontStyle>{FontStyle::bold}}};

    long bar_percentile_count = 20358520 / 100;

    do
    {
        int subsetSize = comboSize - 1;
        auto subset = vector<int>(combo.begin(), combo.begin() + comboSize);
        auto subsetValues = vector<ulong>();

        do
        {
            ulong subsetBitmap = 0ul;
            for (int card : subset)
            {
                subsetBitmap |= 1ul << card;
                subsetValues.push_back(handRankMap[subsetBitmap]);
            }
        } while (next_combination(subset.begin(), subset.begin() + subsetSize, subset.end()));

        ulong bitmap;
        for (int i = 0; i < comboSize; i++)
            bitmap |= 1ul << combo[i];

        handRankMap[bitmap] = *max_element(subsetValues.begin(), subsetValues.end());

        bar_percentile_count--;
        if (bar_percentile_count == 0)
        {
            bar_percentile_count = 20358520 / 100;
            bar.tick();
        }
    } while (next_combination(combo.begin(), combo.begin() + comboSize, combo.end()));

    indicators::show_console_cursor(true);
}

void Evaluator::GenerateSevenCardTable()
{
    vector<int> combo(52);
    for (int i = 0; i < 52; i++)
        combo[i] = i;

    int comboSize = 7;

    using namespace indicators;

    indicators::show_console_cursor(false);
    ProgressBar bar{
        option::BarWidth{50},
        option::Start{"["},
        option::Fill{"="},
        option::Lead{">"},
        option::Remainder{" "},
        option::End{"]"},
        option::PostfixText{"Generating 7 card table"},
        option::ForegroundColor{Color::green},
        option::ShowElapsedTime{true},
        option::ShowRemainingTime{true},
        option::FontStyles{std::vector<FontStyle>{FontStyle::bold}}};

    long combination_count = 0;
    long bar_percentile_count = 133784560 / 100;
    do
    {
        int subsetSize = comboSize - 1;
        auto subset = vector<int>(combo.begin(), combo.begin() + comboSize);
        auto subsetValues = vector<ulong>();

        do
        {
            ulong subsetBitmap = 0ul;
            for (int card : subset)
            {
                subsetBitmap |= 1ul << card;
                subsetValues.push_back(handRankMap[subsetBitmap]);
            }
        } while (next_combination(subset.begin(), subset.begin() + subsetSize, subset.end()));

        ulong bitmap;
        for (int i = 0; i < comboSize; i++)
            bitmap |= 1ul << combo[i];

        handRankMap[bitmap] = *max_element(subsetValues.begin(), subsetValues.end());

        bar_percentile_count--;
        if (bar_percentile_count == 0)
        {
            bar_percentile_count = 133784560 / 100;
            bar.tick();
        }
    } while (next_combination(combo.begin(), combo.begin() + comboSize, combo.end()));
    indicators::show_console_cursor(true);
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
    } while (next_combination(combo.begin(), combo.begin() + comboSize, combo.end()));

    for (auto [k, v] : monteCarloMap)
    {
        Hand hand = Hand(k);
        string end = "\t";
        hand.PrintColoredCards(end);
        cout << v << endl;
        handRankMap[k] = v;
    }
}

template <typename Iterator>
inline bool next_combination(const Iterator first, Iterator k, const Iterator last)
{
    /* Credits: Thomas Draper */
    // http://stackoverflow.com/a/5097100/8747
    if ((first == last) || (first == k) || (last == k))
        return false;
    Iterator itr1 = first;
    Iterator itr2 = last;
    ++itr1;
    if (last == itr1)
        return false;
    itr1 = last;
    --itr1;
    itr1 = k;
    --itr2;
    while (first != itr1)
    {
        if (*--itr1 < *itr2)
        {
            Iterator j = k;
            while (!(*itr1 < *j))
                ++j;
            std::iter_swap(itr1, j);
            ++itr1;
            ++j;
            itr2 = k;
            std::rotate(itr1, j, last);
            while (last != j)
            {
                ++j;
                ++itr2;
            }
            std::rotate(k, itr2, last);
            return true;
        }
    }
    std::rotate(first, k, last);
    return false;
}