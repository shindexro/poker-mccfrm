#ifndef __CLASS_CHANCE_STATE_H__
#define __CLASS_CHANCE_STATE_H__

#include "abstraction/state.h"
#include "abstraction/play_state.h"
#include "abstraction/terminal_state.h"
#include "game/deck.h"

using namespace std;

namespace poker
{
    class PlayState;

    class ChanceState : public State
    {
        // this is the root state
    public:
        static const string type;

        ChanceState();
        ChanceState(CommunityInfo &community, vector<PlayerInfo> &players, vector<Action> &history);

        void CreateChildren();

        shared_ptr<State> DoRandomAction() override;

        vector<shared_ptr<PlayState>> GetFirstActionStates();

        bool IsPlayerInHand(int player) override;

        ostream &Print(ostream &out) const override;

    private:
        void DealCards(CommunityInfo &newCommunity, vector<PlayerInfo> &newPlayers);
    };
}

#endif
