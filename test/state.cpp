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

TEST_F(StateTest, ChanceStateHasNoCommunityCardsInitially)
{
    ChanceState state = ChanceState(flopCommunity, players, history);

    ASSERT_EQ(state.community.cards.size(), 0);
}

TEST_F(StateTest, ChanceStateDealThreeCardsOnFlop)
{
    ChanceState state = ChanceState();
    state.CreateChildren();
    auto nextState = state.children[0];

    ASSERT_EQ(nextState->community.cards.size(), state.community.cards.size() + 3);
}

TEST_F(StateTest, ChanceStateDealOneCardOnTurn)
{
    ChanceState state = ChanceState();
    state.CreateChildren();
    auto nextState = state.children[0];

    ASSERT_EQ(nextState->community.cards.size(), state.community.cards.size() + 1);
}

TEST_F(StateTest, ChanceStateDealOneCardOnRiver)
{
    ChanceState state = ChanceState();
    state.CreateChildren();
    auto nextState = state.children[0];

    ASSERT_EQ(nextState->community.cards.size(), state.community.cards.size() + 1);
}