#ifndef __ENUM_SUIT_H__
#define __ENUM_SUIT_H__

#include <ostream>
#include <string>

namespace poker {
enum Suit {
  Spades,
  Hearts,
  Diamonds,
  Clubs,
};

std::ostream &operator<<(std::ostream &out, const Suit &value);
} // namespace poker
#endif
