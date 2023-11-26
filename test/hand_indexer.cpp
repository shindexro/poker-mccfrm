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
        vector<int> cardsPerRound{1, 2};
        handIndexer.Construct(cardsPerRound);
    }

    static poker::HandIndexer handIndexer;
};

poker::HandIndexer HandIndexerTest::handIndexer = poker::HandIndexer();

// TODO: try fuzz test for isomorphic hands
TEST_F(HandIndexerTest, IsomorphicHandsHaveSameIndex)
{
    vector<int> cards1{1, 2};
    vector<int> cards2{2, 3};
    int handIndex1 = handIndexer.IndexLastRound(cards1);
    int handIndex2 = handIndexer.IndexLastRound(cards2);

    EXPECT_EQ(handIndex1, handIndex2);
}
