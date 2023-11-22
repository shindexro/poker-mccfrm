#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "game/deck.h"

using namespace testing;
using namespace poker;

TEST(DeckTest, NewDeckInitialSize)
{
    Deck deck = Deck(20);
    EXPECT_EQ(deck.NumRemainingCards(), 20);
}

TEST(DeckTest, NewDeckWithRemovedCardsInitialSize)
{
    ulong removedCards = 0b01001;
    Deck deck = Deck(20, removedCards);
    EXPECT_EQ(deck.NumRemainingCards(), 20 - 2);
}

TEST(DeckTest, RemovedCardsNotInDeck)
{
    int totalCards = 20;
    ulong allCardsBitmap = (1ul << totalCards) - 1;
    ulong removedCards = 0b01001;
    ulong remainingCards = allCardsBitmap & ~removedCards;
    Deck deck = Deck(removedCards);

    EXPECT_EQ(deck.Draw(totalCards - 2), remainingCards);
}

TEST(DeckTest, FewerCardsInDeckAfterEachDraw)
{
    int totalCards = Global::CARDS;
    Deck deck = Deck(totalCards);

    deck.Draw(1);
    EXPECT_EQ(deck.NumRemainingCards(), totalCards - 1);
    deck.Draw(5);
    EXPECT_EQ(deck.NumRemainingCards(), totalCards - 6);
    deck.Draw(3);
    EXPECT_EQ(deck.NumRemainingCards(), totalCards - 9);
}

TEST(DeckTest, CannotDrawMoreCardsThanDeck)
{
    Deck deck = Deck(20);
    EXPECT_THROW(deck.Draw(21), invalid_argument);
}

TEST(DeckTest, CannotDrawMoreCardsThanAvailableCardsInDeck)
{
    ulong removedCards = 0b001;
    Deck deck = Deck(20, removedCards);
    EXPECT_THROW(deck.Draw(20), invalid_argument);
}

TEST(DeckTest, AllCardsStillInDeckAfterShuffle)
{
    Deck deck = Deck(20);
    ulong allCardsBitmap = (1ul << 20) - 1;
    deck.Shuffle();

    EXPECT_EQ(deck.Draw(20), allCardsBitmap);
}

TEST(DeckTest, DrawDeckMatchCardCount)
{
    Deck deck = Deck(20);
    deck.Shuffle();

    EXPECT_EQ(__builtin_popcountll(deck.Draw(1)), 1);
    EXPECT_EQ(__builtin_popcountll(deck.Draw(3)), 3);
    EXPECT_EQ(__builtin_popcountll(deck.Draw(6)), 6);
}

TEST(DeckTest, PeekCards)
{
    Deck deck = Deck(20);

    // deck is not shuffled
    EXPECT_EQ(deck.Peek(0), 0b1);
    EXPECT_EQ(deck.Peek(1), 0b10);
    EXPECT_EQ(deck.Peek(7), 1 << 7);
}
