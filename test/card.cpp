#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <random>

#include "game/card.h"

using namespace testing;
using namespace poker;

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

TEST(CardTest, CardInequality)
{
    Card card1("Ad");
    Card card2("Ah");
    EXPECT_NE(card1, card2);
}

TEST(CardTest, CardOrderByRank)
{
    auto cards = std::vector<Card>({
        Card("2s"),
        Card("3s"),
        Card("4s"),
        Card("5s"),
        Card("6s"),
        Card("7s"),
        Card("8s"),
        Card("9s"),
        Card("Ts"),
        Card("Js"),
        Card("Qs"),
        Card("Ks"),
        Card("As"),
    });
    vector<Card> cardsCopy(cards);

    std::random_device rd;
    std::mt19937 g(rd());
    shuffle(cardsCopy.begin(), cardsCopy.end(), g);

    sort(cardsCopy.begin(), cardsCopy.end());

    EXPECT_THAT(cardsCopy, ElementsAreArray(cards));
}

TEST(CardTest, CardOrder)
{
    EXPECT_LT(Card("2c"), Card("3d"));
    EXPECT_GT(Card("3d"), Card("2c"));
}
