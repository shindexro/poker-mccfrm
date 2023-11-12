#include "abstraction/play_state.h"

namespace poker
{
    const string PlayState::type = "Play";

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
        if (community.actionCount != -1)
            return community.actionCount;

        if (children.size() != 0)
            community.actionCount = children.size();
        else
            community.actionCount = GetValidActions().size();

        return community.actionCount;
    }

    vector<Action> PlayState::GetValidActions()
    {
        auto validActions = vector<Action>();
        int pot = GetPot();
        int currentCall = MinimumCall();

        if (GetNumberOfActivePlayers() == 0)
        {
            throw invalid_argument("There must always be >= one player in hand");
        }
        if (GetNumberOfActivePlayers() == 1)
        {
            return validActions; // no valid actions
        }

        // raises
        if (community.isBettingOpen)
        {
            int raise;
            for (auto i = 0UL; i < Global::raiseRatios.size(); ++i)
            {
                raise = (int)(Global::raiseRatios[i] * pot);
                int actualRaise = raise - (currentCall - players[community.playerToMove].bet);

                if (actualRaise < community.minRaise || raise >= players[community.playerToMove].stack)
                    continue;

                // valid raise, if stack is equal it would be an all in
                if (i == 0)
                    validActions.push_back(Action::Raise1);
                if (i == 1)
                    validActions.push_back(Action::Raise2);
                if (i == 2)
                    validActions.push_back(Action::Raise3);
            }

            raise = players[community.playerToMove].stack;
            // all-in
            if (raise > 0)
            {
                //(currently, multiple all-ins in a row dont accumulate the raises and reopen betting round but probably they should)
                // int actualRaise = (raise + players[community.playerToMove].bet) - currentCall;
                validActions.push_back(Action::Allin);
            }
        }
        if (currentCall > players[community.playerToMove].bet)
        {
            // fold
            validActions.push_back(Action::Fold);
        }
        if (currentCall - players[community.playerToMove].bet < players[community.playerToMove].stack)
        {
            // call
            validActions.push_back(Action::Call);
        }

        return validActions;
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

        if (infosetStringGenerated == false)
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

    Infoset PlayState::GetInfosetSecondary()
    {
        // Betting history R, A, CH, C, F
        // Player whose turn it is // not needed?
        // Cards of player whose turn it is
        // community cards

        if (infosetStringGenerated == false)
        {
            string historyString = "";
            for (auto h : history)
            {
                historyString += h;
                historyString += ",";
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
                cardString += "Preflop" + to_string(index);
            }
            else if (community.cards.size() == 3)
            {
                long index = EMDTable::flopIndices[Global::indexer_2_3.IndexLastRound(cards)];
                cardString += "Flop" + to_string(index);
            }
            else if (community.cards.size() == 4)
            {
                long index = EMDTable::turnIndices[Global::indexer_2_4.IndexLastRound(cards)];
                cardString += "Turn" + to_string(index);
            }
            else
            {
                long index = OCHSTable::riverIndices[Global::indexer_2_5.IndexLastRound(cards)];
                cardString += "River" + to_string(index);
            }
            infosetString = historyString + cardString;
        }

        tbb::concurrent_hash_map<string, Infoset>::accessor accessor;
        if (Global::nodeMap.find(accessor, infosetString))
        {
            return accessor->second;
        }
        else
        {
            Infoset infoset = Infoset(GetValidActionsCount());
            Global::nodeMap.insert({infosetString, infoset});
            return infoset;
        }
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

        //// TODO: add this assetion to test
        // int totalStack = accumulate(stacks.begin(), stacks.end(), 0);
        // int totalBet = accumulate(bets.begin(), bets.end(), 0);
        // if (totalStack + totalBet != Global::buyIn * Global::nofPlayers)
        // {
        //     throw invalid_argument("Impossible chip counts");
        // }
    }

    void PlayState::CreateCallChildren()
    {
        // call possible if needed chips is LESS (otherwise its all in), if same its a check
        int call = MinimumCall();
        int additionBet = call - players[community.playerToMove].bet;
        if (additionBet >= players[community.playerToMove].stack)
            return;

        shared_ptr<State> nextState;
        if (GetNextPlayer() == -1)
            nextState = make_shared<PlayState>(community, players, history);
        else if (community.bettingRound < BettingRound::River)
            nextState = make_shared<ChanceState>(community, players, history);
        else
            nextState = make_shared<TerminalState>(community, players, history);

        nextState->history.push_back(Action::Call);
        auto &playerWhoCalled = nextState->players[community.playerToMove];
        playerWhoCalled.lastAction = Action::Call;
        playerWhoCalled.bet += additionBet;
        playerWhoCalled.stack -= additionBet;

        if (dynamic_cast<PlayState *>(nextState.get()))
        {
            nextState->community.playerToMove = GetNextPlayer(community.playerToMove);
        }

        children.push_back(nextState);
    }

    void PlayState::CreateRaiseChildren()
    {
        if (!community.isBettingOpen)
            return;

        // raises
        for (auto i = 0UL; i < Global::raiseRatios.size(); ++i)
        {
            auto nextState = make_shared<PlayState>(*this);

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

            nextState->community.lastPlayer = GetLastPlayer(community.playerToMove);
            nextState->community.playerToMove = nextState->GetNextPlayer();

            nextState->community.isBettingOpen = true;
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
        // all-in
        if (players[community.playerToMove].stack <= 0)
            return;

        // (currently, multiple all-ins in a row dont accumulate the raises and re-open betting round but probably they should)
        int raise = players[community.playerToMove].stack;
        int additionalRaise = (raise + players[community.playerToMove].bet) - MinimumCall();

        auto nextState = make_shared<PlayState>(*this);
        auto &playerWhoAllIn = nextState->players[community.playerToMove];
        nextState->history.push_back(Action::Allin);
        playerWhoAllIn.lastAction = Action::Allin;
        playerWhoAllIn.bet += raise;
        playerWhoAllIn.stack = 0;

        nextState->community.lastPlayer = GetLastPlayer(community.playerToMove);
        nextState->community.playerToMove = GetNextPlayer(community.playerToMove);

        if (additionalRaise >= community.minRaise)
        {
            // re-open betting if raise high enough
            nextState->community.minRaise = additionalRaise;
        }
        else
        {
            //// TODO: this doesn't consider players >=2
            nextState->community.isBettingOpen = false;
        }

        // check if there is any player that has to play..
        if (nextState->community.playerToMove != -1)
        {
            children.push_back(nextState);
        }
        else if (community.bettingRound < BettingRound::River)
        {
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

        auto nextState = make_shared<PlayState>(*this);

        nextState->history.push_back(Action::Fold);
        nextState->players[community.playerToMove].lastAction = Action::Fold;
        nextState->players[community.playerToMove].isStillInGame = false;

        int nextPlayer = GetNextPlayer();

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
            if (community.bettingRound != 4)
            {
                // chance
                children.push_back(make_shared<ChanceState>(nextState->community, nextState->players, nextState->history));
            }
            else
            {
                children.push_back(make_shared<TerminalState>(nextState->community, nextState->players, nextState->history));
            }
        }
    }
} // namespace poker
