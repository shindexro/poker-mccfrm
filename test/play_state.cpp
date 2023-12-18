#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "abstraction/chance_state.h"
#include "abstraction/community_info.h"
#include "abstraction/play_state.h"
#include "enums/action.h"

using namespace testing;

MATCHER_P2(PlayerLastActionIs, player, action, "")
{
    return arg->players[player].lastAction == action;
}

MATCHER_P(PlayerToActIs, player, "")
{
    return arg->community.playerToMove == player;
}

MATCHER(IsChanceState, "")
{
    return (bool)dynamic_cast<ChanceState *>(arg.get());
}

MATCHER(IsPlayState, "")
{
    return (bool)dynamic_cast<PlayState *>(arg.get());
}

TEST(PlayStateTest, GotoChanceStateWhenLastPlayerCalls)
{
    auto state = PlayState();
    state.community.lastPlayer = 2;
    state.community.playerToMove = 2;
    state.players[2].stack = 10000;
    state.CreateChildren();

    ASSERT_THAT(state.children,
            Contains(AllOf(
                    PlayerLastActionIs(2, poker::Action::Call),
                    IsChanceState()
            ))
            .Times(1));
}

TEST(PlayStateTest, NextPlayerToAct)
{
    auto state = PlayState();
    state.community.lastPlayer = 1;
    state.community.playerToMove = 5;
    state.players[5].stack = 10000;
    state.CreateChildren();

    ASSERT_THAT(state.children,
            Contains(AllOf(
                    PlayerLastActionIs(5, poker::Action::Call),
                    IsPlayState(),
                    PlayerToActIs(0)
            ))
            .Times(1));
}
