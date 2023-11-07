#include "game/deck.h"

Deck::Deck(ulong removedCards) : removedCards{removedCards},
                                 position{0},
                                 cards(Global::CARDS)
{
    for (int i = 0; i < Global::CARDS; i++)
    {
        cards[i] = 1ul << i;
    }
}

int Deck::NumRemainingCards()
{
    return Global::CARDS - position;
}

void Deck::Shuffle()
{
    std::random_device rd;
    std::mt19937 g(rd());
    shuffle(cards.begin() + position, cards.end(), g);
}

ulong Deck::Draw(int count)
{
    ulong hand = 0;
    for (int i = 0; i < count; i++)
    {
        while ((cards[position] & removedCards) != 0)
        {
            position++;
        }
        hand |= cards[position];
        position++;
    }
    return hand;
}
