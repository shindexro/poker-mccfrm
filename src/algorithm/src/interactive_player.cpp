#include "algorithm/interactive_player.h"

namespace poker
{
    InteractivePlayer::InteractivePlayer(int id, int stack) : Player(id, stack)
    {
    }

    Action InteractivePlayer::NextAction(PlayState &state)
    {
        auto validActions = state.GetValidActions();

        std::cout << "Choose an action: " << std::endl;
        for (auto i = 0UL; i < validActions.size(); i++)
        {
            std::cout << i << ". " << validActions[i] << "  " << std::endl;
        }
        std::cout << "Your action: ";

        int chosenActionIdx;
        std::cin >> chosenActionIdx;

        return validActions[chosenActionIdx];
    }
} // namespace poker
