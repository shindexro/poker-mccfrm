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

TEST(HandTest, HandStrengthIsOnePair)
{
    auto cardString = vector<string>({"As", "Qh", "2d", "7d", "7c"});
    auto kickers = vector<Rank>({Rank::Seven,
                                 Rank::Ace,
                                 Rank::Queen,
                                 Rank::Two});
    Hand hand(cardString);
    auto strength = hand.GetStrength();

    EXPECT_EQ(strength.handRanking, HandRanking::Pair);
    EXPECT_THAT(strength.kickers, ElementsAreArray(kickers));
}

TEST(HandTest, HandStrengthIsTwoPair)
{
    auto cardString = vector<string>({"As", "Qh", "Qd", "7d", "7c"});
    auto kickers = vector<Rank>({Rank::Queen,
                                 Rank::Seven,
                                 Rank::Ace});
    Hand hand(cardString);
    auto strength = hand.GetStrength();

    EXPECT_EQ(strength.handRanking, HandRanking::TwoPair);
    EXPECT_THAT(strength.kickers, ElementsAreArray(kickers));
}

TEST(HandTest, HandStrengthIsThreeOfAKind)
{
    auto cardString = vector<string>({"7s", "3h", "Ad", "7d", "7c"});
    auto kickers = vector<Rank>({Rank::Seven,
                                 Rank::Ace,
                                 Rank::Three});
    Hand hand(cardString);
    auto strength = hand.GetStrength();

    EXPECT_EQ(strength.handRanking, HandRanking::ThreeOfAKind);
    EXPECT_THAT(strength.kickers, ElementsAreArray(kickers));
}

TEST(HandTest, HandStrengthIsStraight)
{
    auto cardString = vector<string>({"Js", "Th", "7d", "8d", "9c"});
    auto kickers = vector<Rank>({Rank::Jack,
                                 Rank::Ten,
                                 Rank::Nine,
                                 Rank::Eight,
                                 Rank::Seven});
    Hand hand(cardString);
    auto strength = hand.GetStrength();

    EXPECT_EQ(strength.handRanking, HandRanking::Straight);
    EXPECT_THAT(strength.kickers, ElementsAreArray(kickers));
}

TEST(HandTest, HandStrengthIsFlush)
{
    auto cardString = vector<string>({"Jd", "Td", "7d", "3d", "Kd"});
    auto kickers = vector<Rank>({Rank::King,
                                 Rank::Jack,
                                 Rank::Ten,
                                 Rank::Seven,
                                 Rank::Three});
    Hand hand(cardString);
    auto strength = hand.GetStrength();

    EXPECT_EQ(strength.handRanking, HandRanking::Flush);
    EXPECT_THAT(strength.kickers, ElementsAreArray(kickers));
}

TEST(HandTest, HandStrengthIsFullHouse)
{
    auto cardString = vector<string>({"Jd", "Ks", "Jh", "Js", "Kd"});
    auto kickers = vector<Rank>({Rank::Jack,
                                 Rank::King});
    Hand hand(cardString);
    auto strength = hand.GetStrength();

    EXPECT_EQ(strength.handRanking, HandRanking::FullHouse);
    EXPECT_THAT(strength.kickers, ElementsAreArray(kickers));
}

TEST(HandTest, HandStrengthIsFourOfAKind)
{
    auto cardString = vector<string>({"Ad", "Js", "Jh", "Jc", "Jd"});
    auto kickers = vector<Rank>({Rank::Jack,
                                 Rank::Ace});
    Hand hand(cardString);
    auto strength = hand.GetStrength();

    EXPECT_EQ(strength.handRanking, HandRanking::FourOfAKind);
    EXPECT_THAT(strength.kickers, ElementsAreArray(kickers));
}

TEST(HandTest, HandStrengthIsStraightFlush)
{
    auto cardString = vector<string>({"Jh", "Th", "7h", "8h", "9h"});
    auto kickers = vector<Rank>({Rank::Jack,
                                 Rank::Ten,
                                 Rank::Nine,
                                 Rank::Eight,
                                 Rank::Seven});
    Hand hand(cardString);
    auto strength = hand.GetStrength();

    EXPECT_EQ(strength.handRanking, HandRanking::StraightFlush);
    EXPECT_THAT(strength.kickers, ElementsAreArray(kickers));
}

TEST(HandTest, NoHandStrengthWhenLessThanFiveCards)
{
    auto cardString = vector<string>({"Jh", "Th", "7h", "8h"});
    Hand hand(cardString);

    EXPECT_THROW(hand.GetStrength(), invalid_argument);
}

TEST(HandTest, NoHandStrengthWhenMoreThanFiveCards)
{
    auto cardString = vector<string>({"Jh", "Th", "7h", "8h", "9h", "Ac"});
    Hand hand(cardString);

    EXPECT_THROW(hand.GetStrength(), invalid_argument);
}
