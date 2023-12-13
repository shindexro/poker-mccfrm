#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <random>

#include "tables/hand_indexer.h"

using namespace testing;
using namespace poker;

class HandIndexerTest : public Test
{
protected:
    static void SetUpTestSuite()
    {
        handIndexer.Initialise();
        vector<int> cardsPerRound{2};
        handIndexer.Construct(cardsPerRound);
    }

    static poker::HandIndexer handIndexer;
};

poker::HandIndexer HandIndexerTest::handIndexer = poker::HandIndexer();

vector<int> IsomorphicHand(vector<int> &hand)
{
    static auto suitMap = map<Suit, Suit>({
            {Suit::Spades, Suit::Hearts},
            {Suit::Hearts, Suit::Clubs},
            {Suit::Diamonds, Suit::Diamonds},
            {Suit::Clubs, Suit::Spades},
    });
    auto isomorphicHand = vector<int>(hand.size());

    for (auto i = 0ul; i < hand.size(); i++)
    {
        auto card = Card(hand[i]);
        card.suit = suitMap[card.suit];
        isomorphicHand[i] = card.Index();
    }
    return isomorphicHand;
}

TEST_F(HandIndexerTest, IsomorphicHandsHaveSameIndex)
{
    for (int i = 0; i < 20; i++)
    {
        for (int j = i + 1; j < 20; j++)
        {
            vector<int> cards1{i, j};
            auto cards2 = IsomorphicHand(cards1);

            int handIndex1 = handIndexer.IndexLastRound(cards1);
            int handIndex2 = handIndexer.IndexLastRound(cards2);

            ASSERT_EQ(handIndex1, handIndex2) << Card(cards1[0]) << Card(cards1[1]) << Card(cards2[0]) << Card(cards2[1]);
        }
    }
}
