#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "abstraction/state.h"
#include "abstraction/chance_state.h"
#include "abstraction/play_state.h"
#include "abstraction/terminal_state.h"
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
        preflopChanceState = ChanceState();
        for (auto &player : preflopChanceState.players)
        {
            player.stack = 200;
        }
        chanceStates.push_back(&preflopChanceState);
        /////////1/////////2/////////3/////////4/////////5/////////6
        flopChanceState = ChanceState();
        flopChanceState.community.playerToMove = 1;
        flopChanceState.community.lastPlayer = 1;
        flopChanceState.community.minRaise = 5;
        flopChanceState.community.bettingRound = BettingRound::Flop;
        for (auto &player : flopChanceState.players)
        {
            player.bet = 7;
            player.stack = 193;
            player.cards = {1UL, 1UL};
        }
        flopChanceState.players[0].lastAction = poker::Action::Raise;
        flopChanceState.players[1].lastAction = poker::Action::Call;
        flopChanceState.history = vector<poker::Action>({poker::Action::Raise1, poker::Action::Call});
        chanceStates.push_back(&flopChanceState);
        /////////1/////////2/////////3/////////4/////////5/////////6
        turnChanceState = ChanceState();
        turnChanceState.community.bettingRound = BettingRound::Turn;
        turnChanceState.community.cards = vector<ulong>(3, 1UL);
        for (auto &player : turnChanceState.players)
        {
            player.bet = 5;
            player.stack = 195;
            player.lastAction = poker::Action::Call;
            player.cards = {1UL, 1UL};
            turnChanceState.history.push_back(poker::Action::Call);
            turnChanceState.history.push_back(poker::Action::Call);
        }
        chanceStates.push_back(&turnChanceState);
        /////////1/////////2/////////3/////////4/////////5/////////6
        riverChanceState = ChanceState();
        riverChanceState.community.bettingRound = BettingRound::River;
        riverChanceState.community.cards = vector<ulong>(4, 1UL);
        for (auto &player : riverChanceState.players)
        {
            player.bet = 5;
            player.stack = 195;
            player.lastAction = poker::Action::Call;
            player.cards = {1UL, 1UL};
            riverChanceState.history.push_back(poker::Action::Call);
            riverChanceState.history.push_back(poker::Action::Call);
            riverChanceState.history.push_back(poker::Action::Call);
        }
        chanceStates.push_back(&riverChanceState);
        /////////1/////////2/////////3/////////4/////////5/////////6
        nobodyCanActChanceState = ChanceState();
        nobodyCanActChanceState.community.bettingRound = BettingRound::Flop;
        nobodyCanActChanceState.community.cards = vector<ulong>(0);
        for (auto &player : nobodyCanActChanceState.players)
        {
            player.bet = 200;
            player.stack = 0;
            player.lastAction = poker::Action::Allin;
            player.cards = {1UL, 1UL};
            nobodyCanActChanceState.history.push_back(poker::Action::Allin);
        }
        chanceStates.push_back(&nobodyCanActChanceState);
    }

    void CreateChildren()
    {
        for (auto state : chanceStates)
        {
            state->CreateChildren();
        }
    }

    vector<ChanceState *> chanceStates;
    ChanceState preflopChanceState;
    ChanceState flopChanceState;
    ChanceState turnChanceState;
    ChanceState riverChanceState;
    ChanceState nobodyCanActChanceState;
};

class ShowDownTerminalStateTest : public Test
{
protected:
    void SetUp() override
    {
        community = CommunityInfo();
        community.cards = vector<ulong>({1, 2, 4, 8, 16});
        community.bettingRound = BettingRound::River;

        players = vector<PlayerInfo>(2);
        players[0].cards = {32, 64};
        players[1].cards = {128, 256};
        for (auto &player : players)
        {
            player.bet = 2;
            player.stack = 198;
        }

        history = vector<poker::Action>(8, poker::Action::Call);

        state = TerminalState(community, players, history);
    }

    TerminalState state;
    CommunityInfo community;
    vector<PlayerInfo> players;
    vector<poker::Action> history;
};

class PlayStateTest : public Test
{
protected:
    void SetUp() override
    {
        community = CommunityInfo();
        community.isBettingOpen = true;

        players = vector<PlayerInfo>(2);
        players[0].cards = {32, 64};
        players[1].cards = {128, 256};

        players[0].bet = 1;
        players[1].bet = 2;
        players[0].stack = 199;
        players[1].stack = 198;

        history = vector<poker::Action>();

        preflopSBPlayState = PlayState(community, players, history);
        playStates.push_back(&preflopSBPlayState);
        /////////1/////////2/////////3/////////4/////////5/////////6
        community = CommunityInfo();
        community.isBettingOpen = true;
        community.playerToMove = 1;

        players = vector<PlayerInfo>(2);
        players[0].cards = {32, 64};
        players[1].cards = {128, 256};

        players[0].bet = 2;
        players[1].bet = 2;
        players[0].stack = 198;
        players[1].stack = 198;

        history = vector<poker::Action>(1, poker::Action::Call);

        preflopBBPlayState = PlayState(community, players, history);
        playStates.push_back(&preflopBBPlayState);
        /////////1/////////2/////////3/////////4/////////5/////////6
        community = CommunityInfo();
        community.isBettingOpen = true;
        community.bettingRound = BettingRound::Flop;
        community.cards = vector<ulong>({1, 2, 4});

        players = vector<PlayerInfo>(2);
        players[0].cards = {32, 64};
        players[1].cards = {128, 256};

        players[0].bet = 1;
        players[1].bet = 2;
        players[0].stack = 199;
        players[1].stack = 198;

        history = vector<poker::Action>(2, poker::Action::Call);

        flopPlayState = PlayState(community, players, history);
        playStates.push_back(&flopPlayState);
    }

    void CreateChildren()
    {
        for (auto state : playStates)
        {
            state->CreateChildren();
        }
    }

    vector<PlayState *> playStates;
    PlayState preflopSBPlayState;
    PlayState preflopBBPlayState;
    PlayState flopPlayState;
    CommunityInfo community;
    vector<PlayerInfo> players;
    vector<poker::Action> history;
};

///////////////////////////////////////////////////////////////////////////////////////////
// ChanceState tests

TEST_F(ChanceStateTest, HasPlayStateChildWhenSomeoneCanAct)
{
    for (auto state : chanceStates)
        EXPECT_EQ(state->children.size(), 0);

    CreateChildren();
    for (auto state : chanceStates)
    {
        if (state->GetNumberOfPlayersThatNeedToAct() == 0)
            continue;

        ASSERT_EQ(state->children.size(), 1);
        EXPECT_TRUE(dynamic_cast<PlayState *>(state->children[0].get()));
    }
}

TEST_F(ChanceStateTest, DirectlyGoToNextChanceWhenNobodyCanAct)
{
    CreateChildren();

    ASSERT_EQ(nobodyCanActChanceState.children.size(), 1);
    EXPECT_TRUE(dynamic_cast<ChanceState *>(nobodyCanActChanceState.children[0].get()));
}

TEST_F(ChanceStateTest, PlayersThatNeedToAct)
{
    CreateChildren();

    EXPECT_EQ(preflopChanceState.GetNumberOfPlayersThatNeedToAct(), 2);
    EXPECT_EQ(flopChanceState.GetNumberOfPlayersThatNeedToAct(), 2);
    EXPECT_EQ(turnChanceState.GetNumberOfPlayersThatNeedToAct(), 2);
    EXPECT_EQ(riverChanceState.GetNumberOfPlayersThatNeedToAct(), 2);
    EXPECT_EQ(nobodyCanActChanceState.GetNumberOfPlayersThatNeedToAct(), 0);
}

TEST_F(ChanceStateTest, DealPlayerCards)
{
    CreateChildren();

    EXPECT_EQ(preflopChanceState.community.cards.size(), 0);
    EXPECT_EQ(preflopChanceState.children[0]->community.cards.size(), 0);

    for (auto &player : preflopChanceState.children[0]->players)
    {
        int cardCount = __builtin_popcountll(player.GetCardBitmask());
        EXPECT_EQ(cardCount, 2);
    }
}

TEST_F(ChanceStateTest, DealCommunityCards)
{
    CreateChildren();

    EXPECT_EQ(flopChanceState.children[0]->community.cards.size(), 3);
    EXPECT_EQ(turnChanceState.children[0]->community.cards.size(), 4);
    EXPECT_EQ(riverChanceState.children[0]->community.cards.size(), 5);
}

TEST_F(ChanceStateTest, PlayStateChildInSameBettingRound)
{
    CreateChildren();

    for (auto state : chanceStates)
    {
        if (!dynamic_cast<PlayState *>(state->children[0].get()))
            continue;

        EXPECT_EQ(state->children[0]->community.bettingRound, state->community.bettingRound);
    }
}

TEST_F(ChanceStateTest, ChanceStateChildInNextBettingRound)
{
    CreateChildren();

    for (auto state : chanceStates)
    {
        if (!dynamic_cast<ChanceState *>(state->children[0].get()))
            continue;

        BettingRound expectedBettingRound = state->community.bettingRound;
        ++expectedBettingRound;
        EXPECT_EQ(state->children[0]->community.bettingRound, expectedBettingRound);
    }
}

TEST_F(ChanceStateTest, PlayStateChildConfigurations)
{
    CreateChildren();
    int BB = Global::BB;
    int totalPlayers = Global::nofPlayers;

    for (auto state : chanceStates)
    {
        if (!dynamic_cast<PlayState *>(state->children[0].get()))
            continue;

        EXPECT_TRUE(state->children[0]->community.isBettingOpen);
        EXPECT_EQ(state->children[0]->community.minRaise, BB);
        EXPECT_EQ(state->children[0]->community.playerToMove, 0);
        EXPECT_EQ(state->children[0]->community.lastPlayer, totalPlayers - 1);
        EXPECT_THAT(state->children[0]->history, ElementsAreArray(state->history));
    }
}

TEST_F(ChanceStateTest, SkipPlayeStateIfNoPlayersCanAct)
{
    for (auto state : chanceStates)
        for (auto &player : state->players)
            player.lastAction = poker::Action::Allin;

    CreateChildren();

    for (auto state : chanceStates)
    {
        EXPECT_EQ(state->GetNumberOfPlayersThatNeedToAct(), 0);
        EXPECT_EQ(state->children.size(), 1);
    }
    EXPECT_TRUE(dynamic_cast<ChanceState *>(preflopChanceState.children[0].get()));
    EXPECT_TRUE(dynamic_cast<ChanceState *>(flopChanceState.children[0].get()));
    EXPECT_TRUE(dynamic_cast<ChanceState *>(turnChanceState.children[0].get()));
    EXPECT_TRUE(dynamic_cast<TerminalState *>(riverChanceState.children[0].get()));
}

///////////////////////////////////////////////////////////////////////////////////////////
// TerminalState tests

TEST_F(ShowDownTerminalStateTest, SingleWinnerRewards)
{
    state.players[0].bet = 7;
    state.players[1].bet = 5;

    EXPECT_EQ(state.GetReward(0), -7 + 7 + 5);
    EXPECT_EQ(state.GetReward(1), -5);
}

TEST_F(ShowDownTerminalStateTest, MultipleWinnersRewards)
{
    state.players[0].bet = 5;
    state.players[1].bet = 5;
    // players have same hand strength
    state.players[0].cards = state.players[1].cards;

    EXPECT_EQ(state.GetReward(0), 0);
    EXPECT_EQ(state.GetReward(1), 0);
}

///////////////////////////////////////////////////////////////////////////////////////////
// PlayState tests

TEST_F(PlayStateTest, HasChildren)
{
    CreateChildren();
    for (auto state : playStates)
    {
        EXPECT_GT(state->children.size(), 0);
    }
}

TEST_F(PlayStateTest, ChanceStateChildGoToNextRound)
{
    CreateChildren();
    for (auto state : playStates)
    {
        BettingRound expectedBettingRound = state->community.bettingRound;
        ++expectedBettingRound;

        for (auto child : state->children)
        {
            if (!dynamic_cast<ChanceState *>(child.get()))
                continue;

            EXPECT_EQ(child->community.bettingRound, expectedBettingRound);
        }
    }
}

// One ChanceState child by calling (including calling by effective all-in)
TEST_F(PlayStateTest, AtMostOneChanceStateChild)
{
    CreateChildren();
    for (auto state : playStates)
    {
        int chanceChildCount = 0;
        for (auto child : state->children)
        {
            if (!dynamic_cast<ChanceState *>(child.get()))
                continue;

            chanceChildCount++;
        }
        EXPECT_LE(chanceChildCount, 1);
    }
}

TEST_F(PlayStateTest, PlayStateChildrenInSameRound)
{
    CreateChildren();
    for (auto state : playStates)
    {
        for (auto child : state->children)
        {
            if (!dynamic_cast<PlayState *>(child.get()))
                continue;

            EXPECT_EQ(child->community.bettingRound, state->community.bettingRound);
        }
    }
}

TEST_F(PlayStateTest, PlayStateChildrenTypeAmount)
{
    CreateChildren();
    for (auto &state : playStates)
    {
        int calledChildCount = 0;
        int allinChildCount = 0;

        for (auto child : state->children)
        {
            if (!dynamic_cast<PlayState *>(child.get()))
                continue;

            if (child->history.back() == poker::Action::Call)
            {
                calledChildCount++;
            }
            else if (child->history.back() == poker::Action::Allin)
            {
                allinChildCount++;
            }
        }
        EXPECT_LE(calledChildCount, 1);
        EXPECT_LE(allinChildCount, 1);

        if (state == &preflopSBPlayState)
        {
            EXPECT_EQ(calledChildCount, 1);
            EXPECT_EQ(allinChildCount, 1);
        }
    }
}

TEST_F(PlayStateTest, PlayStateChildrenHasPlayerToMove)
{
    CreateChildren();
    for (auto &state : playStates)
    {
        for (auto child : state->children)
        {
            if (!dynamic_cast<PlayState *>(child.get()))
                continue;

            EXPECT_NE(child->community.playerToMove, -1);
            EXPECT_NE(child->community.playerToMove, state->community.playerToMove);
        }
    }
}

TEST_F(PlayStateTest, TotalChipsEqualBuyIns)
{
    CreateChildren();
    for (auto &state : playStates)
    {
        for (auto child : state->children)
        {
            int chips = child->GetPot();
            for (auto &player : child->players)
            {
                chips += player.stack;
            }
            EXPECT_EQ(chips, 400);
        }
    }
}
