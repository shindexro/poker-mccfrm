#ifndef __CLASS_GAME_H__
#define __CLASS_GAME_H__

#include "abstraction/state.h"
#include "abstraction/chance_state.h"
#include "abstraction/play_state.h"
#include "abstraction/terminal_state.h"
#include "algorithm/player.h"

namespace poker
{
    class Game
    {
    public:
        Game(std::vector<Player> &players);

        void Start();
        
    private:
        std::vector<Player> players; 
        std::shared_ptr<State> state;
    };
} // namespace poker

#endif
