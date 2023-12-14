#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "abstraction/community_info.h"
#include "abstraction/chance_state.h"

using namespace testing;
using namespace poker;


TEST(ChanceStateTest, PreflopTransition)
{
    auto state = ChanceState();
    state.CreateChildren();

    ASSERT_EQ(state.children.size(), 1);
    auto child = *state.children[0];

    EXPECT_EQ(child.community.bettingRound, BettingRound::Preflop);
    EXPECT_EQ(child.community.cards.size(), 0);
    EXPECT_EQ(child.community.minRaise, 100);
    EXPECT_EQ(child.community.lastPlayer, 1);
    EXPECT_EQ(child.community.playerToMove, 2);
    EXPECT_TRUE(child.community.isBettingOpen);

    for (auto &player : child.players)
    {
        EXPECT_EQ(__builtin_popcountll(player.GetCardBitmask()), 2);
    }
}

TEST(ChanceStateTest, FlopTransition)
{
    auto state = ChanceState();
    state.community.bettingRound = BettingRound::Flop;
    state.CreateChildren();

    ASSERT_EQ(state.children.size(), 1);
    auto child = *state.children[0];

    EXPECT_EQ(child.community.bettingRound, BettingRound::Flop);
    EXPECT_EQ(child.community.cards.size(), 3);
    EXPECT_EQ(child.community.minRaise, 100);
    EXPECT_EQ(child.community.lastPlayer, 5);
    EXPECT_EQ(child.community.playerToMove, 0);
    EXPECT_TRUE(child.community.isBettingOpen);
}

TEST(ChanceStateTest, TurnTransition)
{
    auto state = ChanceState();
    state.community.bettingRound = BettingRound::Turn;
    state.community.cards = vector<ulong>{1, 2, 4};
    state.CreateChildren();

    ASSERT_EQ(state.children.size(), 1);
    auto child = *state.children[0];

    EXPECT_EQ(child.community.bettingRound, BettingRound::Turn);
    EXPECT_EQ(child.community.cards.size(), 4);
    EXPECT_EQ(child.community.minRaise, 100);
    EXPECT_EQ(child.community.lastPlayer, 5);
    EXPECT_EQ(child.community.playerToMove, 0);
    EXPECT_TRUE(child.community.isBettingOpen);
}

TEST(ChanceStateTest, RiverTransition)
{
    auto state = ChanceState();
    state.community.bettingRound = BettingRound::River;
    state.community.cards = vector<ulong>{1, 2, 4, 8};
    state.CreateChildren();

    ASSERT_EQ(state.children.size(), 1);
    auto child = *state.children[0];

    EXPECT_EQ(child.community.bettingRound, BettingRound::River);
    EXPECT_EQ(child.community.cards.size(), 5);
    EXPECT_EQ(child.community.minRaise, 100);
    EXPECT_EQ(child.community.lastPlayer, 5);
    EXPECT_EQ(child.community.playerToMove, 0);
    EXPECT_TRUE(child.community.isBettingOpen);
}
