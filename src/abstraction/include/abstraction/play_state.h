#ifndef __CLASS_PLAY_STATE_H__
#define __CLASS_PLAY_STATE_H__

#include "abstraction/state.h"
#include "abstraction/terminal_state.h"
#include "abstraction/chance_state.h"

#include <stdexcept>

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

        void CreateChildren() override;
        int GetValidActionsCount();

        // Require actions to be in increasing order of bet size, and fold at the end (if legal)
        vector<Action> GetValidActions();
        vector<int> GetValidBetSizes();

        bool IsPlayerTurn(int player) override;
        bool IsPlayerInHand(int player) override;
        Infoset GetInfoset() override;
        void UpdateInfoset(Infoset &infoset) override;

        ostream &Print(ostream &out) const override;

    private:
        void CreateCallChildren();
        void CreateRaiseChildren();
        void CreateAllInChildren();
        void CreateFoldChildren();

        void GenerateUniqueStringIdentifier();
    };
}

#endif
