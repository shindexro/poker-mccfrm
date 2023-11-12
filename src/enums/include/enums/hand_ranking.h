enum HandRanking
{
    HighCard,
    Pair,
    TwoPair,
    ThreeOfAKind,
    Straight,
    Flush,
    FullHouse,
    FourOfAKind,
    StraightFlush,
};

std::ostream &operator<<(std::ostream &out, const HandRanking &value)
{
    return out << [value]
    {
#define PROCESS_VAL(p) \
    case (p):          \
        return #p;
        switch (value)
        {
            PROCESS_VAL(HighCard);
            PROCESS_VAL(HandRanking::Pair);
            PROCESS_VAL(TwoPair);
            PROCESS_VAL(ThreeOfAKind);
            PROCESS_VAL(Straight);
            PROCESS_VAL(Flush);
            PROCESS_VAL(FullHouse);
            PROCESS_VAL(FourOfAKind);
            PROCESS_VAL(StraightFlush);
        }
#undef PROCESS_VAL
    }();
}
