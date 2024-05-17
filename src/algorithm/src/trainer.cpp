#include "algorithm/trainer.h"

namespace poker
{

    Trainer::Trainer() : rootState{make_shared<ChanceState>()},
                         nodeMapBuffer()
    {
    }

    /// <summary>
    /// Reset game state to save resources
    /// </summary>
    void Trainer::ResetGame()
    {
        rootState = make_shared<ChanceState>();
    }

    void Trainer::TrainOneIteration(int traverser, bool pruneEnabled)
    {
        if (pruneEnabled)
        {
            float q = randDouble();
            if (q < 0.05)
            {
                TraverseMCCFR(traverser, false);
            }
            else
            {
                TraverseMCCFR(traverser, true);
            }
        }
        else
        {
            TraverseMCCFR(traverser, false);
        }
    }

    /// <summary>
    /// Recursively update the strategy for the tree of player
    /// </summary>
    void Trainer::UpdateStrategy(shared_ptr<State> gs, int traverser)
    {
        /* average stretegy only tracked on first betting round, other rounds use real-time search
            Since CFR’s average strategy is not guaranteed to converge to a Nash equilibrium in six player
            poker, there is no theoretical benefit to using the average strategy as opposed to the
            current strategy. */
        if (gs->BettingRound() > BettingRound::Preflop || dynamic_cast<TerminalState *>(gs.get()) || !gs->IsPlayerInHand(traverser))
            return;

        if (ChanceState *cs = dynamic_cast<ChanceState *>(gs.get()))
        {
            UpdateStrategy(cs->DoRandomAction(), traverser);
        }
        else if (gs->IsPlayerTurn(traverser))
        {
            Infoset infoset = GetInfoset(gs);
            int randomIndex = infoset.SampleAction();
            gs->CreateChildren();
            infoset.actionCounter[randomIndex]++;
            UpdateInfoset(gs, infoset);

            gs = gs->children[randomIndex];
            UpdateStrategy(gs, traverser);
        }
        else
        {
            gs->CreateChildren();
            for (auto state : gs->children)
            {
                UpdateStrategy(state, traverser);
            }
        }
    }

    void Trainer::UpdateStrategy(int traverser)
    {
        ResetGame();
        UpdateStrategy(rootState, traverser);
    }

    int Trainer::TraverseMCCFR(int traverser, bool pruned)
    {
        ResetGame();
        return TraverseMCCFR(rootState, traverser, pruned);
    }

    int Trainer::TraverseMCCFR(shared_ptr<State> gs, int traverser, bool pruned)
    {
        int ret = 0;
        if (dynamic_cast<TerminalState *>(gs.get()))
        {
            ret = gs->GetReward(traverser);
        }
        else if (!gs->IsPlayerInHand(traverser)) // we cant get the reward because this function is not implemented
        {
            ret = -gs->players[traverser].bet; // correct?
        }
        else if (dynamic_cast<ChanceState *>(gs.get()))
        {
            // sample a from chance
            ret = TraverseMCCFR(gs->DoRandomAction(), traverser, pruned);
        }
        else if (gs->IsPlayerTurn(traverser))
        {
            // according to supp. mat. page 3, we do full MCCFR on the last betting round, otherwise skip low regret
            Infoset infoset = GetInfoset(gs);
            auto sigma = infoset.CalculateStrategy();
            int expectedVal = 0;

            gs->CreateChildren();
            auto expectedValsChildren = vector<int>(gs->children.size());
            auto explored = vector<bool>(gs->children.size(), true);

            // calculate value of the current node
            // based on weighted average value of children
            for (auto i = 0UL; i < gs->children.size(); ++i)
            {
                if (pruned && infoset.regret[i] < Global::regretPrunedThreshold)
                {
                    explored[i] = false;
                    continue;
                }
                expectedValsChildren[i] = TraverseMCCFR(gs->children[i], traverser, pruned);
                expectedVal += sigma[i] * expectedValsChildren[i];
            }
            for (auto i = 0UL; i < gs->children.size(); ++i)
            {
                if (!explored[i])
                    continue;
                infoset.regret[i] += expectedValsChildren[i] - expectedVal;
                infoset.regret[i] = max({Global::regretFloor, infoset.regret[i]});
            }
            UpdateInfoset(gs, infoset);
            ret = expectedVal;
        }
        else
        {
            Infoset infoset = GetInfoset(gs);
            int randomIndex = infoset.SampleAction();
            gs->CreateChildren();

            ret = TraverseMCCFR(gs->children[randomIndex], traverser, pruned);
        }

        return ret;
    }

    void Trainer::DiscountInfosets(float d)
    {
        for (auto &[stateId, infoset] : Global::nodeMap)
        {
            for (auto &r : infoset.regret)
                r *= d;

            for (auto &a : infoset.actionCounter)
                a *= d;
        }
    }

    void Trainer::UpdateInfoset(shared_ptr<State> state, Infoset &infoset)
    {
        auto stateId = state->StringId();
        nodeMapBuffer[stateId] = infoset;

        // TODO: change this arbitrary number
        if (nodeMapBuffer.size() < 10000)
            return;

        for (auto &[k, v] : nodeMapBuffer)
        {
            Global::nodeMap.try_emplace_l(k, [&](auto globalv) {
                    for (auto i = 0UL; i < v.regret.size(); i++)
                    {
                    globalv.second.regret[i] += v.regret[i];
                    }
                    }, v);
        }
        nodeMapBuffer.clear();
    }

    Infoset Trainer::GetInfoset(shared_ptr<State> state)
    {
        auto stateId = state->StringId();

        Infoset infoset;
        Global::nodeMap.if_contains(stateId, [&](auto v){
                infoset = v.second;
                });

        for (auto i = 0UL; i < infoset.regret.size(); i++)
        {
            infoset.regret[i] += nodeMapBuffer[stateId].regret[i];
        }
        return infoset;
    }

    void Trainer::PrintStartingHandsChart()
    {
        ResetGame();
        auto states = (dynamic_cast<ChanceState *>(rootState.get()))->GetFirstActionStates();

        for (auto i = 0UL; i < states[0]->GetValidActions().size(); ++i)
        {
            auto raiseRatios = Global::raiseRatiosByRoundByPlayerCount.at(BettingRound::Preflop)[6];

            if (states[0]->GetValidActions()[i] == Action::Fold)
            {
                std::cout << "FOLD Table" << endl;
            }
            if (states[0]->GetValidActions()[i] == Action::Call)
            {
                std::cout << "CALL Table" << endl;
            }
            if (states[0]->GetValidActions()[i] == Action::Raise1)
            {
                std::cout << raiseRatios[0] << "*POT RAISE "
                          << "Table" << endl;
            }
            if (states[0]->GetValidActions()[i] == Action::Raise2)
            {
                std::cout << raiseRatios[1] << "*POT RAISE "
                          << "Table" << endl;
            }
            if (states[0]->GetValidActions()[i] == Action::Raise3)
            {
                std::cout << raiseRatios[2] << "*POT RAISE "
                          << "Table" << endl;
            }
            if (states[0]->GetValidActions()[i] == Action::Allin)
            {
                std::cout << "ALLIN Table" << endl;
            }

            std::cout << "    2    3    4    5    6    7    8    9    T    J    Q    K    A (suited)" << endl;
            for (auto j = 0UL; j < states.size(); ++j)
            {
                auto state = states[j];
                Infoset infoset = GetInfoset(state);
                auto sigma = infoset.CalculateStrategy();
                // auto phi = infoset.GetFinalStrategy();

                if (j % Global::RANKS == 0 && j + 1 < states.size())
                {
                    if (j / Global::RANKS < 8)
                        std::cout << (j / Global::RANKS + 2) << " ";
                    else if (j / Global::RANKS == 8)
                        std::cout << "T ";
                    else if (j / Global::RANKS == 9)
                        std::cout << "J ";
                    else if (j / Global::RANKS == 10)
                        std::cout << "Q ";
                    else if (j / Global::RANKS == 11)
                        std::cout << "K ";
                    else
                        std::cout << "A ";
                }

                if (sigma[i] <= 0.2)
                    std::cout << "\e[38;5;196m";
                else if (sigma[i] <= 0.4)
                    std::cout << "\e[38;5;202m";
                else if (sigma[i] <= 0.6)
                    std::cout << "\e[38;5;208m";
                else if (sigma[i] <= 0.8)
                    std::cout << "\e[38;5;148m";
                else
                    std::cout << "\e[38;5;154m";

                std::cout << fixed << setprecision(2) << sigma[i] << " ";
                std::cout << "\e[0m";

                if ((j + 1) % Global::RANKS == 0)
                    std::cout << endl;
            }

            std::cout << endl;
            std::cout << endl;
        }
    }

    void Trainer::PrintStatistics(long iterations)
    {
        ResetGame();
        auto gs = dynamic_cast<ChanceState *>(rootState.get())->GetFirstActionStates();

        // int maxOutput = Global::RANKS * Global::RANKS;
        int maxOutput = 3;
        for (auto ps : gs)
        {
            if (maxOutput < 0)
                break;
            maxOutput--;

            Infoset infoset = GetInfoset(ps);

            auto [card1, card2] = ps->players[ps->community.playerToMove].cards;
            Hand hand = Hand();
            hand.cards.push_back(card1);
            hand.cards.push_back(card2);

            hand.PrintColoredCards("\n");
            auto actions = ps->GetValidActions();

            for (auto j = 0UL; j < actions.size(); ++j)
            {
                auto raiseRatios = Global::raiseRatiosByRoundByPlayerCount.at(BettingRound::Preflop)[6];

                if (actions[j] == Action::Fold)
                {
                    std::cout << "FOLD: ";
                }
                if (actions[j] == Action::Call)
                {
                    std::cout << "CALL: ";
                }
                if (actions[j] == Action::Raise1)
                {
                    std::cout << raiseRatios[0] << "*POT ";
                }
                if (actions[j] == Action::Raise2)
                {
                    std::cout << raiseRatios[1] << "*POT ";
                }
                if (actions[j] == Action::Raise3)
                {
                    std::cout << raiseRatios[2] << "*POT ";
                }
                if (actions[j] == Action::Allin)
                {
                    std::cout << "ALLIN: ";
                }
                std::cout << "Regret: " << infoset.regret[j] << " ";
                std::cout << "ActionCounter: " << infoset.actionCounter[j] << " ";
                std::cout << endl;
            }
            std::cout << endl;
        }
        std::cout << "Number of infosets: " << Global::nodeMap.size() << endl;
        std::cout << "Number of training iterations: " << iterations << endl;
    }

    void Trainer::EnumerateActionSpace(shared_ptr<State> gs)
    {
        static atomic<int> count = 0;
        thread_local static int threadCount = 0;
        if (dynamic_cast<TerminalState *>(gs.get()))
        {
            threadCount++;
            if (threadCount == 100000)
            {
                count += threadCount;
                threadCount = 0;
                std::cout << count << std::endl;
            }
            // std::stringstream outstringStream;
            // for (auto action : gs->history)
            // {
            //     outstringStream << action;
            // }
            // std::cout << outstringStream.str() << std::endl;
        }
        else
        {
            gs->CreateChildren();
            for (auto i = 0UL; i < gs->children.size(); ++i)
            {
                EnumerateActionSpace(gs->children[i]);
            }
            gs->children.clear();
        }
    }

    void Trainer::EnumerateActionSpace()
    {
        EnumerateActionSpace(rootState);
    }
} // namespace poker
