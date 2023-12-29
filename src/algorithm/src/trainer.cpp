#include "algorithm/trainer.h"

namespace poker
{

    Trainer::Trainer(int threadIndex) : rootState{make_shared<ChanceState>()},
                                        threadIndex{threadIndex}
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
        if (gs->BettingRound() > 0 || dynamic_cast<TerminalState *>(gs.get()) || !gs->IsPlayerInHand(traverser))
            return;

        if (ChanceState *cs = dynamic_cast<ChanceState *>(gs.get()))
        {
            UpdateStrategy(cs->DoRandomAction(), traverser);
        }
        else if (gs->IsPlayerTurn(traverser))
        {
            Infoset infoset = gs->GetInfoset();
            auto sigma = infoset.CalculateStrategy();
            int randomIndex = utils::SampleDistribution(sigma);
            gs->CreateChildren();
            infoset.actionCounter[randomIndex]++;
            gs->UpdateInfoset(infoset);

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
            Infoset infoset = gs->GetInfoset();
            auto sigma = infoset.CalculateStrategy();
            int expectedVal = 0;

            gs->CreateChildren();
            auto expectedValsChildren = vector<int>(gs->children.size());
            auto explored = vector<bool>(gs->children.size(), true);

            for (auto i = 0UL; i < gs->children.size(); ++i)
            {
                if (!pruned || (pruned && infoset.regret[i] > Global::regretPrunedThreshold))
                {
                    expectedValsChildren[i] = TraverseMCCFR(gs->children[i], traverser, pruned);
                    expectedVal += sigma[i] * expectedValsChildren[i];
                }
                else
                {
                    explored[i] = false;
                }
            }
            for (auto i = 0UL; i < gs->children.size(); ++i)
            {
                if (explored[i])
                {
                    infoset.regret[i] += expectedValsChildren[i] - expectedVal;
                    infoset.regret[i] = max({Global::regretFloor, infoset.regret[i]});
                }
            }
            gs->UpdateInfoset(infoset);
            ret = expectedVal;
        }
        else
        {
            Infoset infoset = gs->GetInfoset();
            auto sigma = infoset.CalculateStrategy();

            int randomIndex = utils::SampleDistribution(sigma);
            gs->CreateChildren();

            ret = TraverseMCCFR(gs->children[randomIndex], traverser, pruned);
        }

        return ret;
    }

    void Trainer::DiscountInfosets(float d)
    {
        for (auto [infosetString, infoset] : Global::nodeMap)
        {
            for (auto i = 0UL; i < infoset.regret.size(); ++i)
            {
                infoset.regret[i] *= d;
                infoset.actionCounter[i] *= d;
            }
        }
    }

    void Trainer::PrintStartingHandsChart()
    {
        ResetGame();
        auto states = (dynamic_cast<ChanceState *>(rootState.get()))->GetFirstActionStates();

        for (auto i = 0UL; i < states[0]->GetValidActions().size(); ++i)
        {
            auto raiseRatios = Global::raiseRatiosByRound[BettingRound::Preflop];

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
                Infoset infoset = state->GetInfoset();
                auto sigma = infoset.CalculateStrategy();
                // auto phi = infoset.GetFinalStrategy(); // ain't got time to wait for convergence during tests

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

                if (sigma[i] <= 0.25)
                    std::cout << "\033[1;31m";
                else if (sigma[i] <= 0.5)
                    std::cout << "\033[1;32m";
                else if (sigma[i] <= 0.75)
                    std::cout << "\033[1;33m";
                else if (sigma[i] <= 1.0)
                    std::cout << "\033[1;34m";

                std::cout << fixed << setprecision(2) << sigma[i] << " ";
                std::cout << "\033[0m";

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

        int maxOutput = 5; // todo
        for (auto ps : gs)
        {
            if (maxOutput < 0)
                break;
            maxOutput--;

            Infoset infoset = ps->GetInfoset();

            auto [card1, card2] = ps->players[ps->community.playerToMove].cards;
            Hand hand = Hand();
            hand.cards.push_back(card1);
            hand.cards.push_back(card2);

            hand.PrintColoredCards("\n");
            auto actions = ps->GetValidActions();

            for (auto j = 0UL; j < actions.size(); ++j)
            {
                auto raiseRatios = Global::raiseRatiosByRound[BettingRound::Preflop];

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
        if (dynamic_cast<TerminalState *>(gs.get()))
        {
            std::stringstream outstringStream;
            for (auto action : gs->history)
            {
                outstringStream << action;
            }

            std::string outstring = outstringStream.str();
            regex_replace(outstring, regex("RAISE1"), "R0");
            regex_replace(outstring, regex("RAISE2"), "R1");
            regex_replace(outstring, regex("RAISE3"), "R2");
            regex_replace(outstring, regex("ALLIN"), "A");
            regex_replace(outstring, regex("CALL"), "C");
            regex_replace(outstring, regex("CHECK"), "C");
            regex_replace(outstring, regex("FOLD"), "F");

            std::ofstream outfile;
            outfile.open("actionSpace.txt", std::ios_base::app); // append instead of overwrite
            outfile << outstring << endl;
            outfile.close();
        }
        else
        {
            gs->CreateChildren();
            for (auto i = 0UL; i < gs->children.size(); ++i)
            {
                EnumerateActionSpace(gs->children[i]);
            }
        }
    }

    void Trainer::EnumerateActionSpace()
    {
        EnumerateActionSpace(rootState);
    }

    void Trainer::PlayOneGame()
    {
        ResetGame();
        shared_ptr<State> gs = rootState;
        bool first = true;
        while (!(dynamic_cast<TerminalState *>(gs.get())))
        {
            if (dynamic_cast<ChanceState *>(gs.get()))
            {
                // sample a from chance
                gs = gs->DoRandomAction();

                // std::cout << endl;

                // if (first)
                // {
                //     std::cout << "Player Cards: ";
                //     for (auto i = 0; i < Global::nofPlayers; ++i)
                //     {
                //         auto [card1, card2] = gs->players[i].cards;
                //         auto playerCards = vector<Card>({Card(card1), Card(card2)});
                //         playerCards[0].PrintBeautifulString();
                //         playerCards[1].PrintBeautifulString(" ");
                //     }
                //     first = false;
                // }
                // else
                // {
                //     if (gs->community.cards.size() != 0)
                //         std::cout << "Table Cards: " << endl;
                //     for (auto i = 0UL; i < gs->community.cards.size(); ++i)
                //     {
                //         Card(gs->community.cards[i]).PrintBeautifulString();
                //     }
                // }
            }
            else if (dynamic_cast<PlayState *>(gs.get()))
            {
                // std::cout << endl;
                // std::cout << "Player " << gs->community.playerToMove << "'s turn : ";
                Infoset infoset = gs->GetInfoset();
                auto sigma = infoset.CalculateStrategy();

                int randomIndex = utils::SampleDistribution(sigma);
                gs->CreateChildren();
                gs = gs->children[randomIndex];
                // std::cout << gs->history[gs->history.size() - 1];
            }
        }
        // std::cout << endl;
        // std::cout << "Rewards: ";
        // for (auto i = 0; i < Global::nofPlayers; ++i)
        // {
        //     std::cout << gs->GetReward(i) << " ";
        // }
        // std::cout << endl;
    }
} // namespace poker
