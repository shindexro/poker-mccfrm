#include "algorithm/game.h"

namespace poker
{
    void Game::Start()
    {
        while (!(dynamic_cast<TerminalState *>(state.get())))
        {
            std::cout << state << std::endl;
            if (dynamic_cast<ChanceState *>(state.get()))
            {
                state = state->DoRandomAction();
            }
            else if (auto playState = dynamic_cast<PlayState *>(state.get()))
            {
                state->CreateChildren();
                auto action = players[state->community.playerToMove].NextAction(*playState);
                for (auto child : state->children)
                {
                    if (child->history.back() == action)
                    {
                        state = child;
                        break;
                    }
                }
            }
        }
    }
} // namespace poker
