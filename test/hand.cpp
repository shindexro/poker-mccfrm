#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "game/hand.h"

using namespace testing;

TEST(HandTest, CreateEmptyHand)
{
    Hand hand = Hand();
    EXPECT_THAT(hand.cards, IsEmpty());
}

TEST(HandTest, CreateHandWithCardBitmap)
{
    ulong bitmap = 0ul;
    auto cards = vector<Card>({Card("As"), Card("Qh"), Card("2d")});
    for (auto card : cards)
    {
        bitmap |= card.Bitmask();
    }

    Hand hand(bitmap);
    EXPECT_THAT(hand.cards, UnorderedElementsAreArray(cards));
}

TEST(HandTest, CreateHandWithCardStrings)
{
    auto cards = vector<Card>({Card("As"), Card("Qh"), Card("2d")});
    auto cardString = vector<string>({"As", "Qh", "2d"});

    Hand hand(cardString);
    EXPECT_THAT(hand.cards, UnorderedElementsAreArray(cards));
}

TEST(HandTest, HandStrengthIsHighCard)
{
    auto cardString = vector<string>({"As", "Qh", "2d", "5d", "7c"});
    auto kickers = vector<Rank>({Rank::Ace,
                                 Rank::Queen,
                                 Rank::Seven,
                                 Rank::Five,
                                 Rank::Two});

    Hand hand(cardString);
    auto strength = hand.GetStrength();
    EXPECT_EQ(strength.handRanking, HandRanking::HighCard);
    EXPECT_THAT(strength.kickers, ElementsAreArray(kickers));
}
