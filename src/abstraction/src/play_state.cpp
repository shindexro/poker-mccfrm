#include "abstraction/play_state.h"

namespace poker
{
    PlayState::PlayState() : State()
    {
    }

    PlayState::PlayState(CommunityInfo &community,
                         vector<PlayerInfo> &players,
                         vector<Action> &history) : State(community, players, history)
    {
    }

    int PlayState::GetValidActionsCount()
    {
        CreateChildren();
        return children.size();
    }

    vector<Action> PlayState::GetValidActions()
    {
        auto validActions = vector<Action>();
        CreateChildren();
        for (auto child : children)
        {
            validActions.push_back(child->history.back());
        }
        return validActions;
    }

    vector<int> PlayState::GetBetSizes()
    {
        auto validBets = vector<int>();
        CreateChildren();
        for (auto child : children)
        {
            auto newStack = child->players[community.playerToMove].stack;
            auto curStack = players[community.playerToMove].stack;
            validBets.push_back(curStack - newStack);
        }
        return validBets;
    }

    bool PlayState::IsPlayerTurn(int player)
    {
        return community.playerToMove == player;
    }

    bool PlayState::IsPlayerInHand(int player)
    {
        return players[player].isStillInGame;
    }

    Infoset PlayState::GetInfoset()
    {
        // Betting history R, A, CH, C, F
        // Player whose turn it is // not needed?
        // Cards of player whose turn it is
        // community cards

        if (!infosetStringGenerated)
        {
            GenerateUniqueStringIdentifier();
        }

        NodeMapAccessor accessor;
        bool nodeMapEntryExists = Global::nodeMap.find(accessor, infosetString);
        if (!nodeMapEntryExists)
        {
            Infoset infoset = Infoset(GetValidActionsCount());
            Global::nodeMap.insert({infosetString, infoset});
        }
        Global::nodeMap.find(accessor, infosetString);
        return accessor->second;
    }

    void PlayState::GenerateUniqueStringIdentifier()
    {
        string historyString = "";
        for (auto h : history)
        {
            historyString += h;
        }

        auto cards = vector<int>{
            Card::GetIndexFromBitmask(get<0>(players[community.playerToMove].cards)),
            Card::GetIndexFromBitmask(get<1>(players[community.playerToMove].cards))};
        for (auto i = 0UL; i < community.cards.size(); ++i)
        {
            cards.push_back(Card::GetIndexFromBitmask(community.cards[i]));
        }

        string cardString = "";
        if (community.cards.size() == 0)
        {
            long index = Global::indexer_2.IndexLastRound(cards);
            cardString += "P" + to_string(index);
        }
        else if (community.cards.size() == 3)
        {
            long index = EMDTable::flopIndices[Global::indexer_2_3.IndexLastRound(cards)];
            cardString += "F" + to_string(index);
        }
        else if (community.cards.size() == 4)
        {
            long index = EMDTable::turnIndices[Global::indexer_2_4.IndexLastRound(cards)];
            cardString += "T" + to_string(index);
        }
        else
        {
            long index = OCHSTable::riverIndices[Global::indexer_2_5.IndexLastRound(cards)];
            cardString += "R" + to_string(index);
        }
        infosetString = historyString + cardString;
        infosetStringGenerated = true;
    }

    void PlayState::UpdateInfoset(Infoset &infoset)
    {
        NodeMapAccessor accessor;
        Global::nodeMap.insert(accessor, infosetString);
        accessor->second = infoset;
    }

    void PlayState::CreateChildren()
    {
        if (children.size() != 0)
            return;

        CreateCallChildren();
        CreateRaiseChildren();
        CreateAllInChildren();
        CreateFoldChildren();
    }

    void PlayState::CreateCallChildren()
    {
        // call possible if needed chips is LESS (otherwise its all in), if same its a check
        int call = MinimumCall();
        int additionBet = call - players[community.playerToMove].bet;
        if (additionBet >= players[community.playerToMove].stack)
            return;

        shared_ptr<State> nextState;
        if (NextActivePlayer() != -1)
        {
            nextState = make_shared<PlayState>(community, players, history);
        }
        else if (community.bettingRound < BettingRound::River)
        {
            nextState = make_shared<ChanceState>(community, players, history);
            ++nextState->community.bettingRound;
        }
        else
        {
            nextState = make_shared<TerminalState>(community, players, history);
        }

        nextState->history.push_back(Action::Call);
        auto &playerWhoCalled = nextState->players[community.playerToMove];
        playerWhoCalled.lastAction = Action::Call;
        playerWhoCalled.bet += additionBet;
        playerWhoCalled.stack -= additionBet;

        if (dynamic_cast<PlayState *>(nextState.get()))
        {
            nextState->community.playerToMove = NextActivePlayer();
        }

        children.push_back(nextState);
    }

    void PlayState::CreateRaiseChildren()
    {
        // raises
        for (auto i = 0UL; i < Global::raiseRatios.size(); ++i)
        {
            auto nextState = make_shared<PlayState>(community, players, history);

            // we add <raise> chips to our current bet
            int raise = (int)(Global::raiseRatios[i] * GetPot());
            int additionalRaise = raise + players[community.playerToMove].bet - MinimumCall();
            /* For example, if an opponent bets $5, a player must raise by at least another $5,
                and they may not raise by only $2.
                If a player raises a bet of $5 by $7 (for a total of $12),
                the next re-raise would have to be by at least another $7 (the previous raise)
                more than the $12 (for a total of at least $19).
            */
            if (additionalRaise < community.minRaise || raise >= players[community.playerToMove].stack)
                continue;

            // valid raise, if stack is equal it would be an all in
            // TODO: dont hardcode this
            if (i == 0)
                nextState->history.push_back(Action::Raise1);
            if (i == 1)
                nextState->history.push_back(Action::Raise2);
            if (i == 2)
                nextState->history.push_back(Action::Raise3);

            auto &playerWhoRaised = nextState->players[community.playerToMove];
            playerWhoRaised.stack -= raise;
            playerWhoRaised.bet += raise;
            playerWhoRaised.lastAction = Action::Raise;

            nextState->community.lastPlayer = PrevActivePlayer();
            nextState->community.playerToMove = NextActivePlayer();

            nextState->community.minRaise = additionalRaise;

            if (nextState->community.playerToMove != -1)
            {
                children.push_back(nextState);
            }
            //// TODO: move assertion to test
            // else
            // {
            //     throw invalid_argument("Someone raised but there is no one left to play next");
            // }
        }
    }

    void PlayState::CreateAllInChildren()
    {
        // (currently, multiple all-ins in a row dont accumulate the raises and re-open betting round but probably they should)
        int raise = players[community.playerToMove].stack;
        int additionalRaise = (raise + players[community.playerToMove].bet) - MinimumCall();

        auto nextState = make_shared<PlayState>(community, players, history);
        auto &playerWhoAllIn = nextState->players[community.playerToMove];
        nextState->history.push_back(Action::Allin);
        playerWhoAllIn.lastAction = Action::Allin;
        playerWhoAllIn.bet += raise;
        playerWhoAllIn.stack = 0;

        nextState->community.lastPlayer = PrevActivePlayer();
        nextState->community.playerToMove = nextState->NextActivePlayer();

        if (additionalRaise >= community.minRaise)
        {
            // re-open betting if raise high enough
            nextState->community.minRaise = additionalRaise;
        }

        // check if there is any player that has to play..
        if (nextState->community.playerToMove != -1)
        {
            children.push_back(nextState);
        }
        else if (community.bettingRound < BettingRound::River)
        {
            ++nextState->community.bettingRound;
            children.push_back(make_shared<ChanceState>(nextState->community, nextState->players, nextState->history));
        }
        else
        {
            children.push_back(make_shared<TerminalState>(nextState->community, nextState->players, nextState->history));
        }
    }

    void PlayState::CreateFoldChildren()
    {
        if (MinimumCall() <= players[community.playerToMove].bet)
            return;

        auto nextState = make_shared<PlayState>(community, players, history);

        nextState->history.push_back(Action::Fold);
        nextState->players[community.playerToMove].lastAction = Action::Fold;
        nextState->players[community.playerToMove].isStillInGame = false;

        int nextPlayer = NextActivePlayer();
        nextState->community.playerToMove = nextPlayer;

        if (nextState->GetNumberOfActivePlayers() == 1)
        {
            // terminal state
            children.push_back(make_shared<TerminalState>(nextState->community, nextState->players, nextState->history));
        }
        else if (nextPlayer != -1)
        {
            children.push_back(nextState);
        }
        else
        {
            // here the betting round is over, there is more than 1 player left
            if (community.bettingRound < BettingRound::River)
            {
                ++nextState->community.bettingRound;
                children.push_back(make_shared<ChanceState>(nextState->community, nextState->players, nextState->history));
            }
            else
            {
                children.push_back(make_shared<TerminalState>(nextState->community, nextState->players, nextState->history));
            }
        }
    }

    ostream &PlayState::Print(ostream &out) const
    {
        out << "Play | ";
        return State::Print(out);
    }
} // namespace poker
