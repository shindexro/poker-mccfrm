#include "algorithm/trainer.h"

using namespace poker;

Trainer::Trainer(int threadIndex) : rootState{make_shared<ChanceState>()},
                                    threadIndex{threadIndex}
{
    Global::deck = Deck();
}

/// <summary>
/// Reset game state to save resources
/// </summary>
void Trainer::ResetGame()
{
    rootState = make_shared<ChanceState>();
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
    if (rootState->community.bettingRound > 0)
    {
        throw invalid_argument("DAFUQ");
    }
    UpdateStrategy(rootState, traverser);
}

float Trainer::TraverseMCCFRPruned(int traverser)
{
    ResetGame();
    return TraverseMCCFRPruned(rootState, traverser);
}

float Trainer::TraverseMCCFR(int traverser, int iteration)
{
    ResetGame();
    return TraverseMCCFR(rootState, traverser, iteration);
}

float Trainer::TraverseMCCFRPruned(shared_ptr<State> gs, int traverser)
{
    if (dynamic_cast<TerminalState *>(gs.get()))
    {
        return gs->GetReward(traverser);
    }
    else if (!gs->IsPlayerInHand(traverser)) // we cant get the reward because this function is not implemented
    {
        return -gs->players[traverser].bet; // correct?
    }
    else if (dynamic_cast<ChanceState *>(gs.get()))
    {
        // sample a from chance
        return TraverseMCCFRPruned(gs->DoRandomAction(), traverser);
    }
    else if (gs->IsPlayerTurn(traverser))
    {
        // according to supp. mat. page 3, we do full MCCFR on the last betting round, otherwise skip low regret
        if (gs->community.bettingRound != 4)
        {
            // Infoset of player i that corresponds to h
            Infoset infoset = gs->GetInfoset();
            auto sigma = infoset.CalculateStrategy();
            float expectedVal = 0.0f;

            gs->CreateChildren();
            auto expectedValsChildren = vector<float>();
            auto explored = vector<bool>();
            for (auto i = 0UL; i < gs->children.size(); ++i)
            {
                if (infoset.regret[i] > Global::C)
                {
                    expectedValsChildren.push_back(TraverseMCCFRPruned(gs->children[i], traverser));
                    explored.push_back(true);
                    expectedVal += sigma[i] * expectedValsChildren[expectedValsChildren.size() - 1];
                }
                else
                {
                    explored.push_back(false);
                }
            }
            for (auto i = 0UL; i < gs->children.size(); ++i)
            {
                if (explored[i])
                {
                    infoset.regret[i] += expectedValsChildren[i] - expectedVal;
                    infoset.regret[i] = max({(float)Global::regretFloor, infoset.regret[i]});
                }
            }
            gs->UpdateInfoset(infoset);
            return expectedVal;
        }
        else
        {
            // do the same as in normal MCCFR
            // Infoset of player i that corresponds to h
            Infoset infoset = gs->GetInfoset();
            auto sigma = infoset.CalculateStrategy();
            float expectedVal = 0.0f;

            gs->CreateChildren();
            auto expectedValsChildren = vector<float>();
            for (auto i = 0UL; i < gs->children.size(); ++i)
            {
                expectedValsChildren.push_back(TraverseMCCFRPruned(gs->children[i], traverser));
                expectedVal += sigma[i] * expectedValsChildren[expectedValsChildren.size() - 1];
            }
            for (auto i = 0UL; i < gs->children.size(); ++i)
            {
                infoset.regret[i] += expectedValsChildren[i] - expectedVal;
                infoset.regret[i] = max({(float)Global::regretFloor, infoset.regret[i]});
            }
            gs->UpdateInfoset(infoset);
            return expectedVal;
        }
    }
    else
    {
        Infoset infoset = gs->GetInfoset();
        auto sigma = infoset.CalculateStrategy();

        int randomIndex = utils::SampleDistribution(sigma);
        gs->CreateChildren();

        return TraverseMCCFRPruned(gs->children[randomIndex], traverser);
    }
}

float Trainer::TraverseMCCFR(shared_ptr<State> gs, int traverser, int iteration)
{
    if (dynamic_cast<TerminalState *>(gs.get()))
    {
        return gs->GetReward(traverser);
    }
    else if (!gs->IsPlayerInHand(traverser)) // we cant get the reward because this function is not implemented
    {
        return -gs->players[traverser].bet; // correct?
    }
    else if (dynamic_cast<ChanceState *>(gs.get()))
    {
        // sample a from chance
        return TraverseMCCFR(gs->DoRandomAction(), traverser, iteration);
    }
    else if (gs->IsPlayerTurn(traverser))
    {
        // Infoset of player i that corresponds to h
        Infoset infoset = gs->GetInfoset();
        auto sigma = infoset.CalculateStrategy();
        float expectedVal = 0.0f;

        gs->CreateChildren();
        auto expectedValsChildren = vector<float>();
        for (auto i = 0UL; i < gs->children.size(); ++i)
        {
            auto childVal = TraverseMCCFR(gs->children[i], traverser, iteration);
            expectedValsChildren.push_back(childVal);
            expectedVal += sigma[i] * childVal;
            // cout << "state temp expected value is " << expectedVal << " with child node value "
            //      << childVal << endl;
        }
        for (auto i = 0UL; i < gs->children.size(); ++i)
        {
            infoset.regret[i] += expectedValsChildren[i] - expectedVal;
            infoset.regret[i] = max({(float)Global::regretFloor, infoset.regret[i]});
            // cout << "regret is " << infoset.regret[i] << endl;
        }
        gs->UpdateInfoset(infoset);
        return expectedVal;
    }
    else
    {
        Infoset infoset = gs->GetInfoset();
        auto sigma = infoset.CalculateStrategy();

        int randomIndex = utils::SampleDistribution(sigma);
        gs->CreateChildren();

        return TraverseMCCFR(gs->children[randomIndex], traverser, iteration);
    }
}

void Trainer::TraverseMCCFRPruned()
{
    throw invalid_argument("TraverseMCCFRPruned not implemented");
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
    auto gs = (dynamic_cast<ChanceState *>(rootState.get()))->GetFirstActionStates();

    for (auto i = 0UL; i < gs[0]->GetValidActions().size(); ++i)
    {
        if (gs[0]->GetValidActions()[i] == Action::Fold)
        {
            std::cout << "FOLD Table" << endl;
        }
        if (gs[0]->GetValidActions()[i] == Action::Call)
        {
            std::cout << "CALL Table" << endl;
        }
        if (gs[0]->GetValidActions()[i] == Action::Raise1)
        {
            std::cout << Global::raiseRatios[0] << "*POT RAISE "
                      << "Table" << endl;
        }
        if (gs[0]->GetValidActions()[i] == Action::Raise2)
        {
            std::cout << Global::raiseRatios[1] << "*POT RAISE "
                      << "Table" << endl;
        }
        if (gs[0]->GetValidActions()[i] == Action::Raise3)
        {
            std::cout << Global::raiseRatios[2] << "*POT RAISE "
                      << "Table" << endl;
        }
        if (gs[0]->GetValidActions()[i] == Action::Allin)
        {
            std::cout << "ALLIN Table" << endl;
        }

        std::cout << "    2    3    4    5    6    7    8    9    T    J    Q    K    A (suited)" << endl;
        for (auto j = 0UL; j < gs.size(); ++j)
        {
            auto ps = gs[j];
            Infoset infoset = ps->GetInfoset();
            // auto sigma = infoset.CalculateStrategy();
            auto phi = infoset.GetFinalStrategy();

            if (j % Global::RANKS == 0 && j + 1 < gs.size())
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

            if (phi[i] <= 0.25)
                std::cout << "\033[1;31m";
            else if (phi[i] <= 0.5)
                std::cout << "\033[1;32m";
            else if (phi[i] <= 0.75)
                std::cout << "\033[1;33m";
            else if (phi[i] <= 1.0)
                std::cout << "\033[1;34m";

            std::cout << fixed << setprecision(2) << phi[i] << " ";
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
                std::cout << Global::raiseRatios[0] << "*POT ";
            }
            if (actions[j] == Action::Raise2)
            {
                std::cout << Global::raiseRatios[1] << "*POT ";
            }
            if (actions[j] == Action::Raise3)
            {
                std::cout << Global::raiseRatios[2] << "*POT ";
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
        // cout << "Reached a terminal state of depth " << gs->history.size()
        //      << " actions taken:";
        // for (auto a : gs->history)
        // {
        //     cout << vector<string>({"NONE",
        //                             "FOLD",
        //                             // CHECK, combined with CALL, basically calling a 0 raise
        //                             "CALL",
        //                             "RAISE",
        //                             "RAISE1",
        //                             "RAISE2",
        //                             "RAISE3",
        //                             "RAISE4",
        //                             "RAISE5",
        //                             "RAISE6",
        //                             "ALLIN"})[a]
        //          << " ";
        // }
        // cout << endl;

        string outstring = "";
        for (auto action : gs->history)
        {
            outstring += action;
        }
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
        // cout << "Enumerating action space for state: " << gs.get()
        //      << " with " << gs->children.size() << " children" << endl;
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

            std::cout << endl;

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

float Trainer::PlayOneGame_d(int mainPlayer, bool display)
{
    ResetGame();
    shared_ptr<State> gs = rootState;
    bool first = true;
    while (!dynamic_cast<TerminalState *>(gs.get()))
    {
        if (dynamic_cast<ChanceState *>(gs.get()))
        {
            // sample a from chance
            gs = gs->DoRandomAction();

            if (display)
                std::cout << endl;

            if (first)
            {
                if (display)
                    std::cout << "Player Cards: ";
                for (auto i = 0; i < Global::nofPlayers; ++i)
                {
                    auto [card1, card2] = gs->players[i].cards;
                    auto playerCards = vector<Card>({Card(card1), Card(card2)});
                    if (display)
                        playerCards[0].PrintBeautifulString();
                    if (display)
                        playerCards[1].PrintBeautifulString(" ");
                }
                first = false;
            }
            else
            {
                if (gs->community.cards.size() != 0)
                    if (display)
                        std::cout << "Table Cards: ";
                for (auto i = 0UL; i < gs->community.cards.size(); ++i)
                {
                    if (display)
                        Card(gs->community.cards[i]).PrintBeautifulString();
                }
            }
        }
        else if (dynamic_cast<PlayState *>(gs.get()))
        {
            if (display)
                std::cout << endl;
            if (display)
                std::cout << "Player " << gs->community.playerToMove << "'s turn : ";

            Infoset infoset;
            if (mainPlayer == gs->community.playerToMove)
            {
                infoset = gs->GetInfoset();
            }
            else
            {
                infoset = gs->GetInfosetSecondary();
            }

            auto sigma = infoset.CalculateStrategy();

            int randomIndex = utils::SampleDistribution(sigma);
            gs->CreateChildren();
            gs = gs->children[randomIndex];
            if (display)
                std::cout << gs->history[gs->history.size() - 1];
        }
    }
    if (display)
        std::cout << endl;
    if (display)
        std::cout << "Rewards: ";
    for (auto i = 0; i < Global::nofPlayers; ++i)
    {
        if (display)
            std::cout << gs->GetReward(i) << " ";
    }
    if (display)
        std::cout << endl;
    return gs->GetReward(mainPlayer);
}
