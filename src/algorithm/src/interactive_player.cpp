#include "algorithm/interactive_player.h"

namespace poker
{
    InteractivePlayer::InteractivePlayer(int id) : Player(id)
    {
    }

    Action InteractivePlayer::NextAction(PlayState &state)
    {
        auto validActions = state.GetValidActions();

        std::cout << "Choose an action: ";
        for (auto i = 0UL; i < validActions.size(); i++)
        {
            std::cout << i << ". " << validActions[i] << "  " << std::endl;
        }
        std::cout << "Enter action: ";

        int chosenActionIdx;
        std::cin >> chosenActionIdx;

        return validActions[chosenActionIdx];
    }
} // namespace poker
