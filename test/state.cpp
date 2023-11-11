#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "abstraction/state.h"
#include "abstraction/chance_state.h"
#include "abstraction/play_state.h"
#include "enums/action.h"

using namespace testing;
using namespace poker;

class StateTest : public Test
{
protected:
    void SetUp() override
    {
        preflopCommunity = CommunityInfo();
        flopCommunity = CommunityInfo();
        turnCommunity = CommunityInfo();
        riverCommunity = CommunityInfo();

        preflopCommunity.bettingRound = BettingRound::Preflop;
        flopCommunity.bettingRound = BettingRound::Flop;
        turnCommunity.bettingRound = BettingRound::Turn;
        riverCommunity.bettingRound = BettingRound::River;

        players = vector<PlayerInfo>(Global::nofPlayers);
        history = vector<poker::Action>();
    }

    CommunityInfo preflopCommunity;
    CommunityInfo flopCommunity;
    CommunityInfo turnCommunity;
    CommunityInfo riverCommunity;
    vector<PlayerInfo> players;
    vector<poker::Action> history;
};

TEST_F(StateTest, ChanceStateHasAPlayStateChild)
{
    ChanceState state = ChanceState();

    EXPECT_EQ(state.children.size(), 0UL);
    state.CreateChildren();
    ASSERT_EQ(state.children.size(), 1UL);
    EXPECT_TRUE(dynamic_cast<PlayState *>(state.children[0].get()));
}

TEST_F(StateTest, ChanceStateDealPlayerCardsPreflop)
{
    ChanceState state = ChanceState(preflopCommunity, players, history);
    state.CreateChildren();
    auto nextState = state.children[0];

    ASSERT_EQ(state.community.cards.size(), 0);
    ASSERT_EQ(nextState->community.cards.size(), 0);

    for (auto &player : nextState->players)
    {
        int cardCount = __builtin_popcountll(player.GetCardBitmask());
        ASSERT_EQ(cardCount, 2);
    }
}

TEST_F(StateTest, ChanceStateDealThreeCardsOnFlop)
{
    ChanceState state = ChanceState(flopCommunity, players, history);
    state.CreateChildren();
    auto nextState = state.children[0];

    ASSERT_EQ(nextState->community.cards.size(), state.community.cards.size() + 3);
}

TEST_F(StateTest, ChanceStateDealOneCardOnTurn)
{
    ChanceState state = ChanceState(turnCommunity, players, history);
    state.CreateChildren();
    auto nextState = state.children[0];

    ASSERT_EQ(nextState->community.cards.size(), state.community.cards.size() + 1);
}

TEST_F(StateTest, ChanceStateDealOneCardOnRiver)
{
    ChanceState state = ChanceState(riverCommunity, players, history);
    state.CreateChildren();
    auto nextState = state.children[0];

    ASSERT_EQ(nextState->community.cards.size(), state.community.cards.size() + 1);
}

TEST_F(StateTest, ChanceStateChildIsOneBettingRoundAfter)
{
    ChanceState state = ChanceState();
    state.CreateChildren();
    auto nextState = state.children[0];

    ASSERT_EQ(nextState->community.bettingRound, state.community.bettingRound + 1);
}

TEST_F(StateTest, ChanceStateChildHasBBMinRaise)
{
    ChanceState state = ChanceState();
    state.CreateChildren();
    auto nextState = state.children[0];
    auto BB = Global:BB;

    ASSERT_EQ(nextState->community.minRaise, BB);
}

TEST_F(StateTest, ChanceStateChildIsBettingRound)
{
    ChanceState state = ChanceState();
    state.CreateChildren();
    auto nextState = state.children[0];

    ASSERT_EQ(nextState->community.bettingRound, true);
}

TEST_F(StateTest, ChanceStateChildPlayerToPlayUnchanged)
{
    ChanceState state = ChanceState();
    state.CreateChildren();
    auto nextState = state.children[0];

    ASSERT_EQ(nextState->community.lastPlayer, state.community.lastPlayer);
    ASSERT_EQ(nextState->community.playerToMove, state.community.playerToMove);
}
