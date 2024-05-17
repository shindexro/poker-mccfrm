#ifndef __ENUM_BETTING_ROUND_H__
#define __ENUM_BETTING_ROUND_H__

#include <string>

namespace poker {
enum BettingRound { Preflop = 0, Flop, Turn, River };

std::ostream &operator<<(std::ostream &out, const BettingRound &value);
BettingRound &operator++(BettingRound &round);
} // namespace poker
#endif
