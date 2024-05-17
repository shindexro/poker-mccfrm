#include "game/hand_strength.h"

namespace poker {
HandStrength::HandStrength() : kickers{vector<Rank>()} {}

int HandStrength::Compare(const HandStrength &other) const {
  if (this->handRanking > other.handRanking)
    return 1;
  else if (this->handRanking < other.handRanking)
    return -1;

  for (auto i = 0UL; i < this->kickers.size(); i++) {
    if (this->kickers[i] > other.kickers[i])
      return 1;
    else if (this->kickers[i] < other.kickers[i])
      return -1;
  }
  return 0;
}

bool HandStrength::operator<(const HandStrength &rhs) const {
  return this->Compare(rhs) < 0;
}

bool HandStrength::operator==(const HandStrength &rhs) const {
  return this->Compare(rhs) == 0;
}

ostream &operator<<(ostream &out, const HandStrength &strength) {
  out << strength.handRanking << " ";
  for (auto kicker : strength.kickers) {
    out << kicker;
  }
  return out;
}
} // namespace poker
