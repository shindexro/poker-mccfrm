#include "card.h"

using namespace std;
using namespace Enums;

class Card
{
public:
    Rank rank;
    Suit suit;

    Card(string &s)
    {
        if (s.length() != 2)
        {
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

    Card(int index)
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

    bool Equals(const Card &other)
    {
        return rank == other.rank && suit == other.suit;
    }

    int PrimeRank()
    {
        return rankPrimes[(int)rank];
    }

    int PrimeSuit()
    {
        return suitPrimes[(int)suit];
    }

    int HashCode(Card &c)
    {
        return c.PrimeRank() * c.PrimeSuit();
    }

    int Index()
    {
        return (int)rank * 4 + (int)suit;
    }

private:
    const static int rankPrimes[];
    const static int suitPrimes[];
};
