#ifndef __ENUM_RANK_H__
#define __ENUM_RANK_H__

#include <ostream>
#include <string>

namespace poker {
enum Rank {
  Two,
  Three,
  Four,
  Five,
  Six,
  Seven,
  Eight,
  Nine,
  Ten,
  Jack,
  Queen,
  King,
  Ace,
};

std::ostream &operator<<(std::ostream &out, const Rank &value);
} // namespace poker

#endif
