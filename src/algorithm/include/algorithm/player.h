#ifndef __CLASS_PLAYER_H__
#define __CLASS_PLAYER_H__

#include "abstraction/play_state.h"
#include "abstraction/state.h"
#include "enums/action.h"

namespace poker {
class Player {
public:
  const int id;
  int stack;

  Player(int id, int stack);

  // the whole state is passed for simlicity but a player should not access
  // other player's hand
  virtual Action NextAction(shared_ptr<PlayState> /*state*/) {
    throw invalid_argument("Not implemented");
  };
  virtual Action NextAction(shared_ptr<PlayState> /*state*/,
                            shared_ptr<PlayState> /**/) {
    throw invalid_argument("Not implemented");
  };
};
} // namespace poker

#endif
