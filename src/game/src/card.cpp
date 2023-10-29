#include "game/card.h"

Card::Card(const char *s)
{
    Card(string(s));
}

Card::Card(const string &s)
{
    if (s.length() != 2)
    {
        cerr << "Received card string " << s << endl;
        throw invalid_argument("Card string must be length 2");
    }
    switch (s[0])
    {
    case '2':
        rank = Rank::Two;
        break;
    case '3':
        rank = Rank::Three;
        break;
    case '4':
        rank = Rank::Four;
        break;
    case '5':
        rank = Rank::Five;
        break;
    case '6':
        rank = Rank::Six;
        break;
    case '7':
        rank = Rank::Seven;
        break;
    case '8':
        rank = Rank::Eight;
        break;
    case '9':
        rank = Rank::Nine;
        break;
    case 'T':
        rank = Rank::Ten;
        break;
    case 'J':
        rank = Rank::Jack;
        break;
    case 'Q':
        rank = Rank::Queen;
        break;
    case 'K':
        rank = Rank::King;
        break;
    case 'A':
        rank = Rank::Ace;
        break;
    default:
        throw invalid_argument("Invalid card string rank");
    }

    switch (s[1])
    {
    case 's':
        suit = Suit::Spades;
        break;
    case 'h':
        suit = Suit::Hearts;
        break;
    case 'd':
        suit = Suit::Diamonds;
        break;
    case 'c':
        suit = Suit::Clubs;
        break;
    default:
        throw invalid_argument("Invalid card string suit");
    }
}

Card::Card(int index)
{
    switch (index / 4 + 2)
    {
    case 2:
        rank = Rank::Two;
        break;
    case 3:
        rank = Rank::Three;
        break;
    case 4:
        rank = Rank::Four;
        break;
    case 5:
        rank = Rank::Five;
        break;
    case 6:
        rank = Rank::Six;
        break;
    case 7:
        rank = Rank::Seven;
        break;
    case 8:
        rank = Rank::Eight;
        break;
    case 9:
        rank = Rank::Nine;
        break;
    case 10:
        rank = Rank::Ten;
        break;
    case 11:
        rank = Rank::Jack;
        break;
    case 12:
        rank = Rank::Queen;
        break;
    case 13:
        rank = Rank::King;
        break;
    case 14:
        rank = Rank::Ace;
        break;
    default:
        throw invalid_argument("Invalid card string rank");
    }

    switch (index % 4)
    {
    case 0:
        suit = Suit::Spades;
        break;
    case 1:
        suit = Suit::Hearts;
        break;
    case 2:
        suit = Suit::Diamonds;
        break;
    case 3:
        suit = Suit::Clubs;
        break;
    default:
        throw invalid_argument("Invalid card string suit");
    }
}

bool Card::Equals(const Card &other)
{
    return rank == other.rank && suit == other.suit;
}

int Card::PrimeRank()
{
    return rankPrimes[(int)rank];
}

int Card::PrimeSuit()
{
    return suitPrimes[(int)suit];
}

int Card::HashCode(Card &c)
{
    return c.PrimeRank() * c.PrimeSuit();
}

int Card::Index()
{
    return (int)rank * 4 + (int)suit;
}

string Card::ToString()
{
    char ranks[] = {'2', '3', '4', '5', '6', '7', '8', '9', 'T', 'J', 'Q', 'K'};
    char suits[] = {'s', 'h', 'd', 'c'};
    // char suits[] = {'♠', '♥', '♦', '♣'};
    string s = string({ranks[(int)rank],
                       suits[(int)suit]});
    return s;
}
