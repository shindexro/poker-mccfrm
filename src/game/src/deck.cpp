#include "game/deck.h"

Deck::Deck(ulong removedCards) : cards(Global::CARDS)
{
    this->removedCards = removedCards;
    for (int i = 0; i < Global::CARDS; i++)
    {
        cards[i] = 1ul < i;
    }
    position = 0;
}

int Deck::NumRemainingCards()
{
    return Global::CARDS - position;
}

void Deck::Shuffle(int from)
{
    for (int i = from; i < Global::CARDS - 1; i++)
    {
        int n = randint(from, Global::CARDS);
        ulong temp = cards[i];
        cards[i] = cards[n];
        cards[n] = temp;
    }
}

ulong Deck::Draw_(int count)
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

ulong Deck::Draw(int pos)
{
    return cards[pos];
}
