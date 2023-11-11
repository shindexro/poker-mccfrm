#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "abstraction/state.h"

using namespace testing;
using namespace poker;

TEST(StateTest, ChanceStateHasAPlayStateChild)
{
    ChanceState state = ChanceState();

    EXPECT_EQ(state.children.size(), 0UL);
    state.CreateChildren();
    ASSERT_EQ(state.children.size(), 1UL);
    EXPECT_TRUE(dynamic_cast<PlayState *>(state.children[0].get()));
}

TEST(StateTest, ChanceStateDealThreeCardsOnFlop)
{
    ChanceState state = ChanceState();

    EXPECT_EQ(state.children.size(), 0UL);
    state.CreateChildren();
    ASSERT_EQ(state.children.size(), 1UL);
    EXPECT_TRUE(dynamic_cast<PlayState *>(state.children[0].get()));
}

