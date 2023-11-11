#ifndef __CLASS_CHANCE_STATE_H__
#define __CLASS_CHANCE_STATE_H__

#include "abstraction/state.h"

using namespace std;

namespace poker
{
    class State;
    class PlayState;
    class TerminalState;

    class ChanceState : public State
    {
        // this is the root state
    public:
        ChanceState();
        ChanceState(CommunityInfo &community, vector<PlayerInfo> &players, vector<Action> &history);

        void CreateChildren();

        shared_ptr<State> DoRandomAction();

        vector<shared_ptr<PlayState>> GetFirstActionStates();

        bool IsPlayerInHand(int player);
    };
}

#endif
