#ifndef __CLASS_STATE_H__
#define __CLASS_STATE_H__

#include "enums/action.h"
#include "abstraction/infoset.h"
#include "abstraction/global.h"
#include "enums/action.h"
#include "utils/utils.h"
#include "tables/ochs_table.h"
#include "tables/emd_table.h"

#include <vector>
#include <string>
#include <oneapi/tbb/concurrent_hash_map.h>
#include <boost/algorithm/string.hpp>

using namespace std;

class State
{
public:
    vector<tuple<ulong, ulong>> playerCards;
    vector<ulong> tableCards;
    vector<int> stacks;
    vector<int> bets;
    vector<float> rewards;
    vector<bool> isPlayerIn;
    vector<State> children;

    int playerToMove;
    int bettingRound;
    int playersInHand;
    int lastPlayer;
    int minRaise;
    bool isBettingOpen;
    int actionCount;

    string infosetString;
    bool infosetStringGenerated;

    vector<Action> history;
    vector<Action> lastActions;

    State();
    State(vector<int> &stacks, vector<int> &bets, vector<Action> &history,
          vector<tuple<ulong, ulong>> &playerCards, vector<ulong> &tableCards, vector<Action> &lastActions,
          vector<bool> &isPlayerIn, int playersInHand, int bettingRound);

    int GetNextPlayer();
    int GetNextPlayer(int lastToMoveTemp);
    int GetLastPlayer(int playerThatRaised);
    int GetNumberOfPlayersThatNeedToAct();

    int GetActivePlayers(vector<bool> &newIsPlayerIn);
    int GetNumberOfAllInPlayers();
    void CreateChildren();
    bool IsPlayerInHand(int traverser);

    Infoset GetInfoset();
    Infoset GetInfosetSecondary();
    bool IsPlayerTurn(int traverser);
    int BettingRound();
    State DoRandomAction();
    float GetReward(int traverser);
};

class TerminalState : public State
{
private:
    bool rewardGenerated = false;

public:
    TerminalState(vector<int> &stacks, vector<int> &bets, vector<Action> &history,
                  vector<tuple<ulong, ulong>> &playerCards, vector<ulong> &tableCards, vector<Action> &lastActions,
                  vector<bool> &isPlayerIn);
    float GetReward(int player);

    void CreateRewards();
};

class ChanceState : public State
{
    // this is the root state
public:
    ChanceState();

    ChanceState(int bettingRound, int playersInHand, vector<int> &stacks, vector<int> &bets, vector<Action> &history,
                vector<tuple<ulong, ulong>> &playerCards, vector<ulong> &tableCards, vector<Action> &lastActions,
                vector<bool> &isPlayerIn);
    void CreateChildren();

    State DoRandomAction();

    vector<PlayState> GetFirstActionStates();

    bool IsPlayerInHand(int player);
};

class PlayState : public State
{
public:
    PlayState(int bettingRound, int playerToMove, int lastToMove, int minRaise,
              int playersInHand, vector<int> &stacks, vector<int> &bets, vector<Action> &history,
              vector<tuple<ulong, ulong>> &playerCards, vector<ulong> &tableCards, vector<Action> &lastActions,
              vector<bool> &isPlayerIn, bool isBettingOpen);
    void CreateChildren();
    int GetValidActionsCount();

    vector<Action> GetValidActions();

    bool IsPlayerTurn(int player);
    bool IsPlayerInHand(int player);
    Infoset GetInfoset();
    Infoset GetInfosetSecondary();
};

#endif
