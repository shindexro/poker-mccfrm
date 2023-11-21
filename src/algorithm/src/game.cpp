#include "algorithm/game.h"

namespace poker
{
    Game::Game(vector<shared_ptr<Player>> &players) : players(players),
                                                      state{make_shared<ChanceState>()}
    {
    }

    void Game::Start()
    {
        while (!(dynamic_cast<TerminalState *>(state.get())))
        {
            if (dynamic_cast<ChanceState *>(state.get()))
            {
                state = state->DoRandomAction();
            }
            else if (auto playState = dynamic_cast<PlayState *>(state.get()))
            {
                state->CreateChildren();
                auto playerToMove = state->community.playerToMove;
                auto action = players[playerToMove]->NextAction(*playState);

                std::cout << "Player " << playerToMove << " " << action << std::endl;

                for (auto child : state->children)
                {
                    if (child->history.back() == action)
                    {
                        state = child;
                        break;
                    }
                }
            }
            state->PrettyPrint(std::cout);
            std::cout << std::endl;
        }

        for (auto i = 0; i < players.size(); i++)
        {
            players[i]->stack += state->players[i].reward;
        }
        state->PrettyPrint(std::cout);
        std::cout << std::endl;
    }
} // namespace poker
