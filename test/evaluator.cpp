#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "tables/evaluator.h"

using namespace testing;
using namespace poker;

class EvaluatorTest : public Test
{
protected:
    void SetUp() override
    {
        if (initialised)
            return;

        evaluator = poker::Evaluator();
        evaluator.Initialise();
        initialised = true;
    }

    static bool initialised;
    poker::Evaluator evaluator;
};

bool EvaluatorTest::initialised = false;

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
