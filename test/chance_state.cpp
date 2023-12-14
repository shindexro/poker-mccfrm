#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "abstraction/state.h"
#include "abstraction/chance_state.h"
#include "enums/action.h"

using namespace testing;
using namespace poker;

///////////////////////////////////////////////////////////////////////////////////////////
// Test fixtures

class ChanceStateTest : public Test
{
protected:
    void SetUp() override
    {
        preflopState = ChanceState();
        SetUpFlopState();
        SetUpTurnState();
        SetUpRiverState();
        SetUpNobodyCanActState();

        states.push_back(&preflopState);
        states.push_back(&flopState);
        states.push_back(&turnState);
        states.push_back(&riverState);
        states.push_back(&nobodyCanActState);

        CreateChildren();
    }

    void SetUpFlopState()
    {
        flopState = ChanceState();
        flopState.community.bettingRound = BettingRound::Flop;
    }
    void SetUpTurnState()
    {
        turnState = ChanceState();
        turnState.community.bettingRound = BettingRound::Turn;
        turnState.community.cards = vector<ulong>(3, 1);
    }
    void SetUpRiverState()
    {
        riverState = ChanceState();
        riverState.community.bettingRound = BettingRound::River;
        riverState.community.cards = vector<ulong>(4, 1);
    }
    void SetUpNobodyCanActState()
    {
        nobodyCanActState = ChanceState();
        flopState.community.bettingRound = BettingRound::Flop;
        for (auto &player : nobodyCanActState.players)
        {
            player.lastAction = poker::Action::Allin;
        }
    }


    void CreateChildren()
    {
        for (auto state : states)
        {
            state->CreateChildren();
        }
    }

    vector<ChanceState *> states;
    ChanceState preflopState;
    ChanceState flopState;
    ChanceState turnState;
    ChanceState riverState;
    ChanceState nobodyCanActState;
};


TEST_F(ChanceStateTest, HasSingleChild)
{
    for (auto &state : states)
        EXPECT_EQ(state->children.size(), 1);
}

TEST_F(ChanceStateTest, DealCommunityCards)
{
    EXPECT_EQ(preflopState.children[0]->community.cards.size(), 0);
    EXPECT_EQ(flopState.children[0]->community.cards.size(), 3);
    EXPECT_EQ(turnState.children[0]->community.cards.size(), 4);
    EXPECT_EQ(riverState.children[0]->community.cards.size(), 5);
}

TEST_F(ChanceStateTest, NextPlayerToMove)
{
    for (auto &state : states)
    {
        if (state == &preflopState)
        {
            EXPECT_EQ(state->children[0]->community.playerToMove, 2);
            continue;
        }
        EXPECT_EQ(state->children[0]->community.playerToMove, 0);
    }
}

TEST_F(ChanceStateTest, ChildBettingRound)
{
    for (auto &state : states)
    {
        if (dynamic_cast<PlayState *>(state))
            EXPECT_EQ(state->children[0]->community.bettingRound, state->community.bettingRound);
        else
            EXPECT_EQ(state->children[0]->community.bettingRound, ++(state->community.bettingRound));
    }
}

TEST_F(ChanceStateTest, GameRootState)
{
    auto state = ChanceState();
    state.CreateChildren();

    ASSERT_EQ(state.children.size(), 1);
    auto child = *state.children[0];

    EXPECT_EQ(child.community.bettingRound, BettingRound::Preflop);
    EXPECT_EQ(child.community.cards.size(), 0);
    EXPECT_EQ(child.community.minRaise, 100);
    EXPECT_EQ(child.community.lastPlayer, 1);
    EXPECT_EQ(child.community.playerToMove, 0);
    EXPECT_TRUE(child.community.isBettingOpen);

    for (auto &player : child.players)
    {
        EXPECT_EQ(__builtin_popcountll(player.GetCardBitmask()), 2);
    }
}
