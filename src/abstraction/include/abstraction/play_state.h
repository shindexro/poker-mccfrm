#ifndef __CLASS_PLAY_STATE_H__
#define __CLASS_PLAY_STATE_H__

#include "abstraction/state.h"

using namespace std;

namespace poker
{
    class PlayState : public State
    {
    public:
        PlayState(CommunityInfo &community, vector<PlayerInfo> &players, vector<Action> &history);

        void CreateChildren();
        int GetValidActionsCount();

        vector<Action> GetValidActions();

        bool IsPlayerTurn(int player);
        bool IsPlayerInHand(int player);
        Infoset GetInfoset();
        Infoset GetInfosetSecondary();
        void UpdateInfoset(Infoset &infoset);
    };
}

#endif
