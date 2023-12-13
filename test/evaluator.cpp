#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "tables/evaluator.h"

using namespace testing;
using namespace poker;

class EvaluatorTest : public Test
{
protected:
    static void SetUpTestSuite()
    {
        evaluator.Initialise();
    }

    static poker::Evaluator evaluator;
};

poker::Evaluator EvaluatorTest::evaluator = poker::Evaluator();

TEST_F(EvaluatorTest, SameHandsHaveSameEvaluation)
{
    auto eval1 = evaluator.Evaluate(0b1111100);
    auto eval2 = evaluator.Evaluate(0b1111100);

    EXPECT_EQ(eval1, eval2);
}

TEST_F(EvaluatorTest, HandsWithSameStrengthHaveSameEvaluation)
{
    auto eval1 = evaluator.Evaluate(214098);
    auto eval2 = evaluator.Evaluate(247874);

    EXPECT_EQ(eval1, eval2);
}

TEST_F(EvaluatorTest, SevenCardsFullHouse)
{
    auto cards1 = vector<string>({"2d", "4h", "5h", "3c", "4s", "4s", "5c"});
    auto cards2 = vector<string>({"2d", "4h", "5h", "3c", "4s", "5c", "4s"});

    auto hand1 = Hand(cards1);
    auto hand2 = Hand(cards2);

    auto eval1 = evaluator.Evaluate(hand1.Bitmap());
    auto eval2 = evaluator.Evaluate(hand2.Bitmap());

    EXPECT_EQ(eval1, eval2);
}
