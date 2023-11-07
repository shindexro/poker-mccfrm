#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "game/deck.h"

TEST(DeckTest, NewDeckInitialSize)
{
    Deck deck = Deck();
    int totalCards = Global::CARDS;
    EXPECT_EQ(deck.NumRemainingCards(), totalCards);
}
