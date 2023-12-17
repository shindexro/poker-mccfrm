#include "gmock/gmock.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>

#include "tables/evaluator.h"
#include "abstraction/community_info.h"
#include "abstraction/terminal_state.h"

using namespace testing;
using namespace poker;


class MockEvaluator : public poker::Evaluator
{
    public:
        int Evaluate(ulong cards) override
        {
            return (int) cards;
        }
};


TEST(TerminalStateTest, NoChildTransition)
{
    auto state = TerminalState();
    state.CreateChildren();

    EXPECT_EQ(state.children.size(), 0);
}

TEST(TerminalStateTest, OnePlayerRemaining)
{
    auto state = TerminalState();
    state.community.cards = vector<ulong>{
        Card("6d").Bitmask(),
        Card("5d").Bitmask(),
        Card("4d").Bitmask(),
        Card("2d").Bitmask(),
        Card("2h").Bitmask(),
    };
    state.players[0].bet = 3000;
    state.players[0].cards = {
        Card("6s").Bitmask(),
        Card("6c").Bitmask(),
    };

    state.players[1].isStillInGame = false;
    state.players[1].bet = 2000;
    state.players[1].cards = {
        Card("5s").Bitmask(),
        Card("5c").Bitmask(),
    };

    for (int i = 2; i < 6; i++)
    {
        state.players[i].isStillInGame = false;
        state.players[i].bet = 0;
    }

    EXPECT_EQ(state.GetReward(0), 2000);
    EXPECT_EQ(state.GetReward(1), -2000);
    EXPECT_EQ(state.GetReward(2), 0);
}

TEST(TerminalStateTest, TwoPlayersShowDownWithWinner)
{
    auto state = TerminalState();
    state.evaluator = make_shared<MockEvaluator>();

    state.players[0].bet = 3000;
    state.players[0].cards = {1, 0};

    state.players[1].bet = 2000;
    state.players[1].cards = {0, 0};

    for (int i = 2; i < 6; i++)
    {
        state.players[i].isStillInGame = false;
        state.players[i].bet = 0;
    }

    EXPECT_EQ(state.GetReward(0), 2000);
    EXPECT_EQ(state.GetReward(1), -2000);
    EXPECT_EQ(state.GetReward(2), 0);
}

TEST(TerminalStateTest, MultiplePlayersClop)
{
    auto state = TerminalState();
    state.evaluator = make_shared<MockEvaluator>();

    for (int i = 0; i < 6; i++)
    {
        state.players[i].isStillInGame = true;
        state.players[i].bet = 1000;
        state.players[i].cards = {0, 0};
    }

    EXPECT_EQ(state.GetReward(0), 0);
    EXPECT_EQ(state.GetReward(1), 0);
    EXPECT_EQ(state.GetReward(2), 0);
}
