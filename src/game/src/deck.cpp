#include "game/deck.h"

Deck::Deck(ulong removedCards) : removedCards{removedCards},
                                 position{0},
                                 cards(Global::CARDS)
{
    for (int i = 0; i < cards.size(); i++)
    {
        cards[i] = 1ul << i;
    }
    numRemovedCards = __builtin_popcountll(removedCards);
}

int Deck::NumRemainingCards()
{
    return cards.size() - numRemovedCards - position;
}

void Deck::Shuffle()
{
    random_shuffle(cards.begin() + position, cards.end());
}

ulong Deck::Draw(int count)
{
    ulong hand = 0ul;
    for (int i = 0; i < count; i++)
    {
        while (position < cards.size() && (cards[position] & removedCards))
        {
            position++;
        }

        if (position >= cards.size())
        {
            throw invalid_argument("Insufficient cards in deck.");
        }

        hand |= cards[position];
        position++;
    }
    return hand;
}

/* WARN: this doesn't take into account removedCards
    but removedCards appears to be legacy.
*/
ulong Deck::Peek(int idx)
{
    return cards[idx];
}
