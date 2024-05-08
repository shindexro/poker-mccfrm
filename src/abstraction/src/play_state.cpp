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

    vector<int> PlayState::GetValidBetSizes()
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

        if (Global::nodeMap.find(infosetString) == Global::nodeMap.end())
        {
            Infoset infoset = Infoset(GetValidActionsCount(), community.bettingRound);
            Global::nodeMap[infosetString] = infoset;
            return infoset;
        }
        return Global::nodeMap[infosetString];
    }

    void PlayState::GenerateUniqueStringIdentifier()
    {
        std::stringstream historyStringStream;
        for (auto h : history)
        {
            switch (h)
            {
                case poker::Action::Call:
                    historyStringStream << "C";
                case poker::Action::Fold:
                    historyStringStream << "F";
                case poker::Action::None:
                    historyStringStream << "N";
                case poker::Action::Allin:
                    historyStringStream << "A";
                case poker::Action::Raise:
                    historyStringStream << "R";
                case poker::Action::Raise1:
                    historyStringStream << "1";
                case poker::Action::Raise2:
                    historyStringStream << "2";
                case poker::Action::Raise3:
                    historyStringStream << "3";
                case poker::Action::Raise4:
                    historyStringStream << "4";
                case poker::Action::Raise5:
                    historyStringStream << "5";
                case poker::Action::Raise6:
                    historyStringStream << "6";
                case poker::Action::Raise7:
                    historyStringStream << "7";
                case poker::Action::Raise8:
                    historyStringStream << "8";
                case poker::Action::Raise9:
                    historyStringStream << "9";
            }
        }
        string historyString = historyStringStream.str();

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

    void PlayState::UpdateInfoset(Infoset &infoset, NodeMap &nodeMap)
    {
        nodeMap[infosetString] = infoset;
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
        auto raiseRatios = Global::raiseRatiosByRoundByPlayerCount.at(community.bettingRound)[GetNumberOfPlayersThatNeedToAct()];

        for (auto i = 0UL; i < raiseRatios.size(); ++i)
        {
            auto nextState = make_shared<PlayState>(community, players, history);

            // we add <raise> chips to our current bet
            int raise = (int)(raiseRatios[i] * GetPot());
            int additionalRaise = raise + players[community.playerToMove].bet - MinimumCall();
            /* For example, if an opponent bets $5, a player must raise by at least another $5,
                and they may not raise by only $2.
                If a player raises a bet of $5 by $7 (for a total of $12),
                the next re-raise would have to be by at least another $7 (the previous raise)
                more than the $12 (for a total of at least $19).
            */
            if (additionalRaise < community.minRaise)
                continue;

            if (raise >= players[community.playerToMove].stack)
                break;

            static const vector<Action> raiseRatioIdxToAction{
                Action::Raise1,
                Action::Raise2,
                Action::Raise3,
                Action::Raise4,
                Action::Raise5,
                Action::Raise6,
                Action::Raise7,
                Action::Raise8,
                Action::Raise9,
            };
            nextState->history.push_back(raiseRatioIdxToAction[i]);

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
