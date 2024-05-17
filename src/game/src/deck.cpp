#include "game/deck.h"

namespace poker {
Deck::Deck(int size, ulong removedCards)
    : removedCards{removedCards}, position{0}, cards(size) {
  for (auto i = 0UL; i < cards.size(); i++) {
    cards[i] = 1ul << i;
  }
  numRemovedCards = __builtin_popcountll(removedCards);
}

int Deck::NumRemainingCards() {
  return cards.size() - numRemovedCards - position;
}

void Deck::Shuffle() {
  std::random_shuffle(cards.begin() + position, cards.end());
}

ulong Deck::Draw(int count) {
  ulong hand = 0ul;
  for (auto i = 0; i < count; i++) {
    while (position < cards.size() && (cards[position] & removedCards)) {
      position++;
    }

    if (position >= cards.size()) {
      throw std::invalid_argument("Insufficient cards in deck.");
    }

    hand |= cards[position];
    position++;
  }
  return hand;
}

/* WARN: this doesn't take into account removedCards
    but removedCards appears to be legacy.
*/
ulong Deck::Peek(int idx) { return cards[idx]; }
} // namespace poker
