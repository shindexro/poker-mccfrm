#include "game/hand.h"

Hand::Hand()
{
}

Hand::Hand(ulong bitmap)
{
    char ranks[] = "23456789TJQKA";
    char suits[] = "shdc";

    for (int r = 0; r < 14; r++)
    {
        for (int s = 0; s < 4; s++)
        {
            int shift = r * 4 + s;
            if (((1ul << shift) & bitmap) != 0)
            {
                string card{ranks[r], suits[s]};
                cards.push_back(Card(card));
            }
        }
    }
}

Hand::Hand(vector<string> &cardStrings)
{
    for (auto card : cardStrings)
    {
        cards.push_back(Card(card));
    }
}

HandStrength Hand::GetStrength()
{
    if (cards.size() != 5)
    {
        throw invalid_argument("Failed to determine hand strength because card count is not 5.");
    }

    // sort cards by (rank, suit) from high to low
    sort(cards.begin(), cards.end(), [](Card &a, Card &b)
         { return a.PrimeRank() * 100 + a.PrimeSuit() > b.PrimeRank() * 100 + b.PrimeSuit(); });

    int rankProduct = accumulate(cards.begin(), cards.end(), 1, [](int acc, Card &c)
                                 { return acc * c.PrimeRank(); });
    int suitProduct = accumulate(cards.begin(), cards.end(), 1, [](int acc, Card &c)
                                 { return acc * c.PrimeSuit(); });

    bool straight =
        rankProduct == 8610         // 5-high straight
        || rankProduct == 2310      // 6-high straight
        || rankProduct == 15015     // 7-high straight
        || rankProduct == 85085     // 8-high straight
        || rankProduct == 323323    // 9-high straight
        || rankProduct == 1062347   // T-high straight
        || rankProduct == 2800733   // J-high straight
        || rankProduct == 6678671   // Q-high straight
        || rankProduct == 14535931  // K-high straight
        || rankProduct == 31367009; // A-high straight

    bool flush =
        suitProduct == 147008443     // Spades
        || suitProduct == 229345007  // Hearts
        || suitProduct == 418195493  // Diamonds
        || suitProduct == 714924299; // Clubs

    map<int, vector<Card>> rankGroups = map<int, vector<Card>>();
    for (auto card : cards)
    {
        rankGroups[(int)card.rank].push_back(card);
    }

    int fourOfAKind = -1;
    int threeOfAKind = -1;
    int onePair = -1;
    int twoPair = -1;

    for (auto [rank, rankCards] : rankGroups)
    {
        int count = rankCards.size();
        if (count == 4)
            fourOfAKind = rank;
        else if (count == 3)
            threeOfAKind = rank;
        else if (count == 2)
        {
            twoPair = onePair;
            onePair = rank;
        }
    }

    HandStrength strength = HandStrength();

    if (straight && flush)
    {
        strength.handRanking = HandRanking::StraightFlush;
        for (auto card : cards)
            strength.kickers.push_back(card.rank);
    }
    else if (fourOfAKind >= 0)
    {
        strength.handRanking = HandRanking::FourOfAKind;
        strength.kickers.push_back((Rank)fourOfAKind);
        for (auto card : cards)
        {
            if ((int)card.rank == fourOfAKind)
                continue;
            strength.kickers.push_back(card.rank);
        }
    }
    else if (threeOfAKind >= 0 && onePair >= 0)
    {
        strength.handRanking = HandRanking::FullHouse;
        strength.kickers.push_back((Rank)threeOfAKind);
        strength.kickers.push_back((Rank)onePair);
    }
    else if (flush)
    {
        strength.handRanking = HandRanking::Flush;
        for (auto card : cards)
            strength.kickers.push_back(card.rank);
    }
    else if (straight)
    {
        strength.handRanking = HandRanking::Straight;
        for (auto card : cards)
            strength.kickers.push_back(card.rank);
    }
    else if (threeOfAKind >= 0)
    {
        strength.handRanking = HandRanking::ThreeOfAKind;
        strength.kickers.push_back((Rank)threeOfAKind);
        for (auto card : cards)
        {
            if ((int)card.rank == threeOfAKind)
                continue;
            strength.kickers.push_back(card.rank);
        }
    }
    else if (twoPair >= 0)
    {
        strength.handRanking = HandRanking::TwoPair;
        strength.kickers.push_back((Rank)max({twoPair, onePair}));
        strength.kickers.push_back((Rank)min({twoPair, onePair}));
        for (auto card : cards)
        {
            if ((int)card.rank == twoPair || (int)card.rank == onePair)
                continue;
            strength.kickers.push_back(card.rank);
        }
    }
    else if (onePair >= 0)
    {
        strength.handRanking = HandRanking::Pair;
        strength.kickers.push_back((Rank)onePair);
        for (auto card : cards)
        {
            if (card.rank == (Rank)twoPair || card.rank == (Rank)onePair)
                continue;
            strength.kickers.push_back(card.rank);
        }
    }
    else
    {
        strength.handRanking = HandRanking::HighCard;
        for (auto card : cards)
            strength.kickers.push_back(card.rank);
    }

    return strength;
}

void Hand::PrintColoredCards(const string &end)
{
    for (int i = 0; i < cards.size(); i++)
    {
        Card card = cards[i];
        cout << card.ToString();
    }
    cout << end;
}

string Hand::ToString()
{
    string s;
    for (Card card : cards)
    {
        s += card.ToString();
    }
    return s;
}
