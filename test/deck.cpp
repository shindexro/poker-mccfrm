#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "game/deck.h"

TEST(DeckTest, NewDeckInitialSize)
{
    Deck deck = Deck();
    int totalCards = Global::CARDS;
    EXPECT_EQ(deck.NumRemainingCards(), totalCards);
}

TEST(DeckTest, FewerCardsInDeckAfterEachDraw)
{
    Deck deck = Deck();
    int totalCards = Global::CARDS;

    deck.Draw(1);
    EXPECT_EQ(deck.NumRemainingCards(), totalCards - 1);
    deck.Draw(5);
    EXPECT_EQ(deck.NumRemainingCards(), totalCards - 6);
    deck.Draw(3);
    EXPECT_EQ(deck.NumRemainingCards(), totalCards - 9);
}

TEST(DeckTest, CannotDrawMoreCardsThanDeck)
{
    Deck deck = Deck();
    int totalCards = Global::CARDS;

    EXPECT_THROW(deck.Draw(totalCards + 1), invalid_argument);
}

TEST(DeckTest, CannotDrawMoreCardsThanAvailableCardsInDeck)
{
    ulong removedCards = 0b001;
    Deck deck = Deck(removedCards);
    int totalCards = Global::CARDS;

    EXPECT_THROW(deck.Draw(totalCards), invalid_argument);
}

