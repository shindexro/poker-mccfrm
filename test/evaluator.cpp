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

poker::Evaluator evaluator = poker::Evaluator();

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
