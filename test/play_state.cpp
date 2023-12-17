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

MATCHER(IsChanceState, "")
{
    return (bool)dynamic_cast<ChanceState *>(arg.get());
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
