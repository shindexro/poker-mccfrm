#include "algorithm/trainer.h"

using namespace poker;

Trainer::Trainer(int threadIndex) : rootState{ChanceState()},
                                    threadIndex{threadIndex}
{
    Global::deck = Deck();
}

/// <summary>
/// Reset game state to save resources
/// </summary>
void Trainer::ResetGame()
{
    rootState = ChanceState();
}

/// <summary>
/// Recursively update the strategy for the tree of player
/// </summary>
void Trainer::UpdateStrategy(State gs, int traverser)
{
    if (gs.BettingRound() > 1 || dynamic_cast<const TerminalState *>(&gs) != nullptr || !gs.IsPlayerInHand(traverser))
    {
        return;
    }
    else if (dynamic_cast<const ChanceState *>(&gs) != nullptr)
    {
        gs = gs.DoRandomAction();
        UpdateStrategy(gs, traverser);
    }
    else if (gs.IsPlayerTurn(traverser))
    {
        Infoset infoset = gs.GetInfoset();
        auto sigma = infoset.CalculateStrategy();
        int randomIndex = utils::SampleDistribution(sigma);
        gs.CreateChildren();
        gs = gs.children[randomIndex];
        infoset.actionCounter[randomIndex]++;

        UpdateStrategy(gs, traverser);
    }
    else
    {
        gs.CreateChildren();
        for (State state : gs.children)
        {
            UpdateStrategy(state, traverser);
        }
    }
}

void Trainer::UpdateStrategy(int traverser)
{
    ResetGame();
    if (rootState.bettingRound > 0)
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

float Trainer::TraverseMCCFRPruned(State gs, int traverser)
{
    if (dynamic_cast<const TerminalState *>(&gs) != nullptr)
    {
        return gs.GetReward(traverser);
    }
    else if (!gs.IsPlayerInHand(traverser)) // we cant get the reward because this function is not implemented
    {
        return -gs.bets[traverser]; // correct?
    }
    else if (dynamic_cast<const ChanceState *>(&gs) != nullptr)
    {
        // sample a from chance
        return TraverseMCCFRPruned(gs.DoRandomAction(), traverser);
    }
    else if (gs.IsPlayerTurn(traverser))
    {
        // according to supp. mat. page 3, we do full MCCFR on the last betting round, otherwise skip low regret
        if (gs.bettingRound != 4)
        {
            // Infoset of player i that corresponds to h
            Infoset infoset = gs.GetInfoset();
            auto sigma = infoset.CalculateStrategy();
            float expectedVal = 0.0f;

            gs.CreateChildren();
            auto expectedValsChildren = vector<float>();
            auto explored = vector<bool>();
            for (int i = 0; i < gs.children.size(); ++i)
            {
                if (infoset.regret[i] > Global::C)
                {
                    expectedValsChildren.push_back(TraverseMCCFRPruned(gs.children[i], traverser));
                    explored.push_back(true);
                    expectedVal += sigma[i] * expectedValsChildren[expectedValsChildren.size() - 1];
                }
                else
                {
                    explored.push_back(false);
                }
            }
            for (int i = 0; i < gs.children.size(); ++i)
            {
                if (explored[i])
                {
                    infoset.regret[i] += expectedValsChildren[i] - expectedVal;
                    infoset.regret[i] = max({(float)Global::regretFloor, infoset.regret[i]});
                }
            }
            return expectedVal;
        }
        else
        {
            // do the same as in normal MCCFR
            // Infoset of player i that corresponds to h
            Infoset infoset = gs.GetInfoset();
            auto sigma = infoset.CalculateStrategy();
            float expectedVal = 0.0f;

            gs.CreateChildren();
            auto expectedValsChildren = vector<float>();
            for (int i = 0; i < gs.children.size(); ++i)
            {
                expectedValsChildren.push_back(TraverseMCCFRPruned(gs.children[i], traverser));
                expectedVal += sigma[i] * expectedValsChildren[expectedValsChildren.size() - 1];
            }
            for (int i = 0; i < gs.children.size(); ++i)
            {
                infoset.regret[i] += expectedValsChildren[i] - expectedVal;
                infoset.regret[i] = max({(float)Global::regretFloor, infoset.regret[i]});
            }
            return expectedVal;
        }
    }
    else
    {
        Infoset infoset = gs.GetInfoset();
        auto sigma = infoset.CalculateStrategy();

        int randomIndex = utils::SampleDistribution(sigma);
        gs.CreateChildren();

        return TraverseMCCFRPruned(gs.children[randomIndex], traverser);
    }
}

void Trainer::PlayOneGame()
{
    ResetGame();
    State gs = rootState;
    bool first = true;
    while (!(dynamic_cast<const TerminalState *>(&gs) != nullptr))
    {
        if (dynamic_cast<const ChanceState *>(&gs) != nullptr)
        {
            // sample a from chance
            gs = gs.DoRandomAction();

            std::cout << endl;

            if (first)
            {
                std::cout << "Player Cards: ";
                for (int i = 0; i < Global::nofPlayers; ++i)
                {
                    auto [card1, card2] = gs.playerCards[i];
                    auto playerCards = vector<Card>({card1, card2});
                    playerCards[0].PrintBeautifulString();
                    playerCards[1].PrintBeautifulString(" ");
                }
                first = false;
            }
            else
            {
                if (gs.tableCards.size() != 0)
                    std::cout << "Table Cards: " << endl;
                for (int i = 0; i < gs.tableCards.size(); ++i)
                {
                    Card(gs.tableCards[i]).PrintBeautifulString();
                }
            }
        }
        else if (dynamic_cast<const PlayState *>(&gs) != nullptr)
        {
            std::cout << endl;
            std::cout << "Player " << gs.playerToMove << "'s turn : ";
            Infoset infoset = gs.GetInfoset();
            auto sigma = infoset.CalculateStrategy();

            int randomIndex = utils::SampleDistribution(sigma);
            gs.CreateChildren();
            gs = gs.children[randomIndex];
            std::cout << gs.history[gs.history.size() - 1];
        }
    }
    std::cout << endl;
    std::cout << "Rewards: ";
    for (int i = 0; i < Global::nofPlayers; ++i)
    {
        std::cout << gs.GetReward(i) << " ";
    }
    std::cout << endl;
}

float Trainer::PlayOneGame_d(int mainPlayer, bool display)
{
    ResetGame();
    State gs = rootState;
    bool first = true;
    while (!utils:: instanceof <TerminalState>(&gs))
    {
        if (utils:: instanceof <ChanceState>(&gs))
        {
            // sample a from chance
            gs = gs.DoRandomAction();

            if (display)
                std::cout << endl;

            if (first)
            {
                if (display)
                    std::cout << "Player Cards: ";
                for (int i = 0; i < Global::nofPlayers; ++i)
                {
                    auto [card1, card2] = gs.playerCards[i];
                    auto playerCards = vector<Card>({card1, card2});
                    if (display)
                        playerCards[0].PrintBeautifulString();
                    if (display)
                        playerCards[1].PrintBeautifulString(" ");
                }
                first = false;
            }
            else
            {
                if (gs.tableCards.size() != 0)
                    if (display)
                        std::cout << "Table Cards: ";
                for (int i = 0; i < gs.tableCards.size(); ++i)
                {
                    if (display)
                        Card(gs.tableCards[i]).PrintBeautifulString();
                }
            }
        }
        else if (utils:: instanceof <PlayState>(&gs))
        {
            if (display)
                std::cout << endl;
            if (display)
                std::cout << "Player " << gs.playerToMove << "'s turn : ";

            Infoset infoset;
            if (mainPlayer == gs.playerToMove)
            {
                infoset = gs.GetInfoset();
            }
            else
            {
                infoset = gs.GetInfosetSecondary();
            }

            auto sigma = infoset.CalculateStrategy();

            int randomIndex = utils::SampleDistribution(sigma);
            gs.CreateChildren();
            gs = gs.children[randomIndex];
            if (display)
                std::cout << gs.history[gs.history.size() - 1];
        }
    }
    if (display)
        std::cout << endl;
    if (display)
        std::cout << "Rewards: ";
    for (int i = 0; i < Global::nofPlayers; ++i)
    {
        if (display)
            std::cout << gs.GetReward(i) << " ";
    }
    if (display)
        std::cout << endl;
    return gs.GetReward(mainPlayer);
}

float Trainer::TraverseMCCFR(State gs, int traverser, int iteration)
{
    if (utils:: instanceof <TerminalState>(&gs))
    {
        return gs.GetReward(traverser);
    }
    else if (!gs.IsPlayerInHand(traverser)) // we cant get the reward because this function is not implemented
    {
        return -gs.bets[traverser]; // correct?
    }
    else if (utils:: instanceof <ChanceState>(&gs))
    {
        // sample a from chance
        return TraverseMCCFR(gs.DoRandomAction(), traverser, iteration);
    }
    else if (gs.IsPlayerTurn(traverser))
    {
        // Infoset of player i that corresponds to h
        Infoset infoset = gs.GetInfoset();
        auto sigma = infoset.CalculateStrategy();
        float expectedVal = 0.0f;

        gs.CreateChildren();
        auto expectedValsChildren = vector<float>();
        for (int i = 0; i < gs.children.size(); ++i)
        {
            expectedValsChildren.push_back(TraverseMCCFR(gs.children[i], traverser, iteration));
            expectedVal += sigma[i] * expectedValsChildren[expectedValsChildren.size() - 1];
        }
        for (int i = 0; i < gs.children.size(); ++i)
        {
            infoset.regret[i] += expectedValsChildren[i] - expectedVal;
            infoset.regret[i] = max({(float)Global::regretFloor, infoset.regret[i]});
        }
        return expectedVal;
    }
    else
    {
        Infoset infoset = gs.GetInfoset();
        auto sigma = infoset.CalculateStrategy();

        int randomIndex = utils::SampleDistribution(sigma);
        gs.CreateChildren();

        return TraverseMCCFR(gs.children[randomIndex], traverser, iteration);
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
        for (int i = 0; i < infoset.regret.size(); ++i)
        {
            infoset.regret[i] *= d;
            infoset.actionCounter[i] *= d;
        }
    }
}

void Trainer::PrintStartingHandsChart()
{
    ResetGame();
    vector<PlayState> gs = dynamic_cast<ChanceState *>(&rootState)->GetFirstActionStates();

    for (int i = 0; i < gs[0].GetValidActions().size(); ++i)
    {
        if (gs[0].GetValidActions()[i] == Action::FOLD)
        {
            std::cout << "FOLD Table" << endl;
        }
        if (gs[0].GetValidActions()[i] == Action::CALL)
        {
            std::cout << "CALL Table" << endl;
        }
        if (gs[0].GetValidActions()[i] == Action::RAISE1)
        {
            std::cout << Global::raises[0] << "*POT RAISE "
                      << "Table" << endl;
        }
        if (gs[0].GetValidActions()[i] == Action::RAISE2)
        {
            std::cout << Global::raises[1] << "*POT RAISE "
                      << "Table" << endl;
        }
        if (gs[0].GetValidActions()[i] == Action::RAISE3)
        {
            std::cout << Global::raises[2] << "*POT RAISE "
                      << "Table" << endl;
        }
        if (gs[0].GetValidActions()[i] == Action::ALLIN)
        {
            std::cout << "ALLIN Table" << endl;
        }

        std::cout << "    2    3    4    5    6    7    8    9    T    J    Q    K    A (suited)" << endl;
        for (int j = 0; j < gs.size(); ++j)
        {
            PlayState ps = gs[j];
            Infoset infoset = ps.GetInfoset();
            // List<float> sigma = infoset.CalculateStrategy();
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

            std::cout << to_string(phi[i]) << " ";
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
    vector<PlayState> gs = dynamic_cast<ChanceState *>(&rootState)->GetFirstActionStates();

    int maxOutput = -1; // todo
    for (PlayState ps : gs)
    {
        if (maxOutput < 0)
            break;
        maxOutput--;

        Infoset infoset = ps.GetInfoset();

        auto [card1, card2] = ps.playerCards[ps.playerToMove];
        Hand hand = Hand();
        hand.cards.push_back(card1);
        hand.cards.push_back(card2);

        hand.PrintColoredCards("\n");
        auto actions = ps.GetValidActions();

        for (int j = 0; j < actions.size(); ++j)
        {
            if (actions[j] == Action::FOLD)
            {
                std::cout << "FOLD: ";
            }
            if (actions[j] == Action::CALL)
            {
                std::cout << "CALL: ";
            }
            if (actions[j] == Action::RAISE1)
            {
                std::cout << Global::raises[0] << "*POT ";
            }
            if (actions[j] == Action::RAISE2)
            {
                std::cout << Global::raises[1] << "*POT ";
            }
            if (actions[j] == Action::RAISE3)
            {
                std::cout << Global::raises[2] << "*POT ";
            }
            if (actions[j] == Action::ALLIN)
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

void Trainer::EnumerateActionSpace(State gs)
{
    if (utils:: instanceof <TerminalState>(&gs))
    {

        string outstring = "";
        for (auto action : gs.history)
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
        gs.CreateChildren();

        for (int i = 0; i < gs.children.size(); ++i)
        {
            EnumerateActionSpace(gs.children[i]);
        }
    }
}

void Trainer::EnumerateActionSpace()
{
    EnumerateActionSpace(rootState);
}
