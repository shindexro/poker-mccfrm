#ifndef __CLASS_PLAY_STATE_H__
#define __CLASS_PLAY_STATE_H__

#include "abstraction/state.h"
#include "abstraction/terminal_state.h"
#include "abstraction/chance_state.h"

using namespace std;

namespace poker
{
    class ChanceState;

    class PlayState : public State
    {
    public:
        static const string type;

        PlayState();
        PlayState(CommunityInfo &community, vector<PlayerInfo> &players, vector<Action> &history);

        void CreateChildren();
        int GetValidActionsCount();

        vector<Action> GetValidActions();

        bool IsPlayerTurn(int player);
        bool IsPlayerInHand(int player);
        Infoset GetInfoset();
        Infoset GetInfosetSecondary();
        void UpdateInfoset(Infoset &infoset);

    private:
        void CreateCallChildren();
        void CreateRaiseChildren();
        void CreateAllInChildren();
        void CreateFoldChildren();
    };
}

#endif
