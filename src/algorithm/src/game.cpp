#include "algorithm/game.h"

namespace poker
{
    Game::Game(vector<shared_ptr<Player>> &players) : players(players),
                                                      state{make_shared<ChanceState>()}
    {
    }

    void Game::Start()
    {
        std::cout << "===== Game start =====" << std::endl;
        shared_ptr<PlayState> roundStartState = nullptr;
        while (!(dynamic_cast<TerminalState *>(state.get())))
        {
            if (dynamic_cast<ChanceState *>(state.get()))
            {
                state = state->DoRandomAction();
            }
            else if (dynamic_cast<PlayState *>(state.get()))
            {
                auto playState = std::dynamic_pointer_cast<PlayState>(state);
                if (roundStartState == nullptr || roundStartState->BettingRound() < playState->BettingRound())
                {
                    roundStartState = playState;
                }

                state->CreateChildren();
                auto playerToMove = state->community.playerToMove;
                auto action = players[playerToMove]->NextAction(playState, roundStartState);

                std::cout << std::endl;
                std::cout << ">> Player " << playerToMove << " " << action << std::endl;

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

        for (auto i = 0UL; i < players.size(); i++)
        {
            players[i]->stack += state->GetReward(i);
        }
        state->PrettyPrint(std::cout);
        std::cout << std::endl;

        for (auto i = 0UL; i < players.size(); i++)
        {
            std::cout << "Player " << i << ": $" << players[i]->stack << "\t";
        }
        std::cout << std::endl << "====== Game end ======" << std::endl;
    }
} // namespace poker
