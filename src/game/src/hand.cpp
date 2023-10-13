#include "game/hand.h"

Hand::Hand()
{
}

Hand::Hand(ulong bitmap)
{
    char ranks[] = "23456789TJQKA";
    char suits[] = "shdc";

    for (int r = 0; r < 14; r++)
    {
        for (int s = 0; s < 4; s++)
        {
            int shift = r * 4 + s;
            if (((1ul << shift) & bitmap) != 0)
            {
                char card[] = {ranks[r], suits[s]};
                cards.push_back(Card(card));
            }
        }
    }
}

void Hand::PrintColoredCards(string &end)
{
    for (int i = 0; i < cards.size(); i++)
    {
        Card card = cards[i];
        cout << card.ToString();
    }
    cout << end;
}

string Hand::ToString()
{
    string s;
    for (Card card : cards)
    {
        s += card.ToString();
    }
    return s;
}
