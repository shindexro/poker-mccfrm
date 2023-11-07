#include <gtest/gtest.h>

#include "game/card.h"

TEST(CardTest, CreateCardWithCString)
{
    Card card("5s");
    EXPECT_EQ(card.rank, Rank::Five);
    EXPECT_EQ(card.suit, Suit::Spades);
}

TEST(CardTest, CreateCardWithCPPString)
{
    string cardString = "6h";
    Card card(cardString);
    EXPECT_EQ(card.rank, Rank::Six);
    EXPECT_EQ(card.suit, Suit::Hearts);
}

TEST(CardTest, CreateCardWithIndex)
{
    Card card(0);
    EXPECT_EQ(card.rank, Rank::Two);
    EXPECT_EQ(card.suit, Suit::Spades);
}

TEST(CardTest, CardEquality)
{
    Card card1("Ad");
    Card card2("Ad");
    EXPECT_EQ(card1, card2);
}
