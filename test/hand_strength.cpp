#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <random>

#include "game/hand_strength.h"

using namespace testing;
using namespace poker;

TEST(HandStrengthTest, HandStrengthComparison)
{
    vector<string> royalFlush{"As", "Ks", "Qs", "Js", "Ts"};
    vector<string> straightFlush{"9s", "Ks", "Qs", "Js", "Ts"};
    vector<string> aceHighFlush{"As", "9s", "2s", "5s", "Ts"};
    vector<string> queenHighFlush{"Qh", "9h", "2h", "5h", "Th"};
    vector<string> straight{"9s", "Ks", "Qs", "Jc", "Ts"};
    vector<string> fullHouse{"9c", "9s", "2h", "2s", "9h"};
    vector<string> ace5FourOfAKind{"As", "Ah", "Ad", "Ac", "5d"};
    vector<string> ace7FourOfAKind{"7s", "Ah", "Ad", "As", "Ac"};
    vector<string> threeOfAKindAceKicker{"7s", "7h", "7d", "2s", "Ac"};
    vector<string> threeOfAKind4Kicker{"7s", "7h", "7c", "3s", "4c"};
    vector<string> twoPair{"7s", "7h", "3c", "3s", "4c"};
    vector<string> pairSeven{"7s", "7h", "2c", "3s", "4c"};
    vector<string> pairQueen{"7s", "5h", "2c", "Qs", "Qc"};
    vector<string> highCardKingHigh{"7s", "2h", "Kc", "3s", "4c"};
    vector<string> highCardNineHigh{"8s", "5h", "9c", "3s", "2c"};

    vector<Hand> hands{
        Hand(highCardNineHigh),
        Hand(highCardKingHigh),
        Hand(pairSeven),
        Hand(pairQueen),
        Hand(twoPair),
        Hand(threeOfAKind4Kicker),
        Hand(threeOfAKindAceKicker),
        Hand(straight),
        Hand(queenHighFlush),
        Hand(aceHighFlush),
        Hand(fullHouse),
        Hand(ace5FourOfAKind),
        Hand(ace7FourOfAKind),
        Hand(straightFlush),
        Hand(royalFlush),
    };

    vector<Hand> handsCopy(hands);

    std::random_device rd;
    std::mt19937 g(rd());
    shuffle(handsCopy.begin(), handsCopy.end(), g);

    sort(handsCopy.begin(), handsCopy.end(), [](Hand &a, Hand &b)
         { return a.GetStrength() < b.GetStrength(); });

    EXPECT_THAT(handsCopy, ElementsAreArray(hands));
}
