#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "game/deck.h"

using namespace testing;

TEST(DeckTest, NewDeckInitialSize)
{
    Deck deck = Deck();
    int totalCards = Global::CARDS;
    EXPECT_EQ(deck.NumRemainingCards(), totalCards);
}

TEST(DeckTest, NewDeckWithRemovedCardsInitialSize)
{
    ulong removedCards = 0b01001;
    Deck deck = Deck(removedCards);
    int totalCards = Global::CARDS;
    EXPECT_EQ(deck.NumRemainingCards(), totalCards - 2);
}

TEST(DeckTest, RemovedCardsNotInDeck)
{
    int totalCards = Global::CARDS;
    ulong allCardsBitmap = (1 << totalCards) - 1;
    ulong removedCards = 0b01001;
    ulong remainingCards = allCardsBitmap & ~removedCards;
    Deck deck = Deck(removedCards);

    EXPECT_EQ(deck.Draw(totalCards - 2), remainingCards);
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

TEST(DeckTest, AllCardsStillInDeckAfterShuffle)
{
    Deck deck = Deck();
    int totalCards = Global::CARDS;
    ulong allCardsBitmap = (1 << totalCards) - 1;
    deck.Shuffle();

    EXPECT_EQ(deck.Draw(totalCards), allCardsBitmap);
}

TEST(DeckTest, DrawDeckMatchCardCount)
{
    Deck deck = Deck();
    int totalCards = Global::CARDS;
    deck.Shuffle();

    EXPECT_EQ(__builtin_popcount(deck.Draw(1)), 1);
    EXPECT_EQ(__builtin_popcount(deck.Draw(3)), 3);
    EXPECT_EQ(__builtin_popcount(deck.Draw(6)), 6);
}

TEST(DeckTest, PeekCards)
{
    Deck deck = Deck();

    // deck is not shuffled
    EXPECT_EQ(deck.Peek(0), 0b1);
    EXPECT_EQ(deck.Peek(1), 0b10);
    EXPECT_EQ(deck.Peek(7), 1<<7);
}
