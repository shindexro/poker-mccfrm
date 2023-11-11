#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "abstraction/state.h"
#include "abstraction/chance_state.h"
#include "abstraction/play_state.h"
#include "abstraction/terminal_state.h"
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

///////////////////////////////////////////////////////////////////////////////////////////
// ChanceState tests

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

    EXPECT_EQ(state.community.cards.size(), 0);
    EXPECT_EQ(nextState->community.cards.size(), 0);

    for (auto &player : nextState->players)
    {
        int cardCount = __builtin_popcountll(player.GetCardBitmask());
        EXPECT_EQ(cardCount, 2);
    }
}

TEST_F(StateTest, ChanceStateDealThreeCardsOnFlop)
{
    ChanceState state = ChanceState(flopCommunity, players, history);
    state.CreateChildren();
    auto nextState = state.children[0];

    EXPECT_EQ(nextState->community.cards.size(), state.community.cards.size() + 3);
}

TEST_F(StateTest, ChanceStateDealOneCardOnTurn)
{
    ChanceState state = ChanceState(turnCommunity, players, history);
    state.CreateChildren();
    auto nextState = state.children[0];

    EXPECT_EQ(nextState->community.cards.size(), state.community.cards.size() + 1);
}

TEST_F(StateTest, ChanceStateDealOneCardOnRiver)
{
    ChanceState state = ChanceState(riverCommunity, players, history);
    state.CreateChildren();
    auto nextState = state.children[0];

    EXPECT_EQ(nextState->community.cards.size(), state.community.cards.size() + 1);
}

TEST_F(StateTest, ChanceStateChildIsOneBettingRoundAfter)
{
    ChanceState state = ChanceState();
    state.CreateChildren();
    auto nextState = state.children[0];

    EXPECT_EQ(nextState->community.bettingRound, state.community.bettingRound + 1);
}

TEST_F(StateTest, ChanceStateChildHasBBMinRaise)
{
    ChanceState state = ChanceState();
    state.CreateChildren();
    auto nextState = state.children[0];
    auto BB = Global::BB;

    EXPECT_EQ(nextState->community.minRaise, BB);
}

TEST_F(StateTest, ChanceStateChildIsBettingRound)
{
    ChanceState state = ChanceState();
    state.CreateChildren();
    auto nextState = state.children[0];

    EXPECT_EQ(nextState->community.isBettingOpen, true);
}

TEST_F(StateTest, ChanceStateChildUnchangedInfo)
{
    ChanceState state = ChanceState();
    state.CreateChildren();
    auto nextState = state.children[0];

    EXPECT_EQ(nextState->community.lastPlayer, state.community.lastPlayer);
    EXPECT_EQ(nextState->community.playerToMove, state.community.playerToMove);
    EXPECT_THAT(nextState->history, ElementsAreArray(state.history));
}

TEST_F(StateTest, ChanceStateSkipPlayeStateIfNoPlayersCanAct)
{
    for (auto &player : players)
        player.lastAction = poker::Action::ALLIN;
    auto flopState = ChanceState(flopCommunity, players, history);
    flopState.CreateChildren();

    ASSERT_EQ(flopState.GetNumberOfPlayersThatNeedToAct(), 0);
    ASSERT_EQ(flopState.children.size(), 1);
    ASSERT_TRUE(dynamic_cast<ChanceState *>(flopState.children[0].get()));

    auto turnState = flopState.children[0];
    turnState->CreateChildren();

    ASSERT_EQ(turnState->GetNumberOfPlayersThatNeedToAct(), 0);
    ASSERT_EQ(turnState->children.size(), 1);
    ASSERT_TRUE(dynamic_cast<ChanceState *>(turnState->children[0].get()));

    auto riverState = turnState->children[0];
    riverState->CreateChildren();

    ASSERT_EQ(riverState->GetNumberOfPlayersThatNeedToAct(), 0);
    ASSERT_EQ(riverState->children.size(), 1);
    ASSERT_TRUE(dynamic_cast<TerminalState *>(riverState->children[0].get()));
}

///////////////////////////////////////////////////////////////////////////////////////////
// TerminalState tests

TEST_F(StateTest, TerminalStateSingleWinnerRewards)
{
    players[0].bet = 7;
    players[1].bet = 5;
    players[0].isStillInGame = true;
    players[1].isStillInGame = false;
    auto state = TerminalState(flopCommunity, players, history);

    EXPECT_EQ(state.GetReward(0), -7 + 7 + 5);
    EXPECT_EQ(state.GetReward(1), -5);
}
