#include "binary/main.h"

namespace poker
{
    void StateDFS(State &state, int depth = 0)
    {
        state.CreateChildren();
        // for (auto &child : state.children)
        // {
        //     StateDFS(*child, depth + 1);
        // }
        if (state.children.size())
        {
            StateDFS(*state.children[randint(0, state.children.size())], depth + 1);
        }
    }

    class Program
    {
    public:
        static void Main()
        {
            // auto state = ChanceState();
            // StateDFS(state);
            // state.PrettyPrintTree();

            CreateIndexers();
            Global::handEvaluator->Initialise();
            CalculateInformationAbstraction();
            LoadFromFile();

            // while (true)
            // {
            //     StartGame();
            // }

            Train();
        }

        static void StartGame()
        {
            auto humanPlayer = make_shared<InteractivePlayer>(0, Global::buyIn);
            auto aiPlayer = make_shared<AIPlayer>(1, Global::buyIn);
            auto players = vector<shared_ptr<Player>>();
            players.push_back(humanPlayer);
            for (int i = 1; i < 6; i++)
            {
                players.push_back(make_shared<AIPlayer>(i, Global::buyIn));
            }
            auto game = Game(players);

            game.Start();
        }

    private:
        static void CreateIndexers()
        {
            HandIndexer::Initialise();
            vector<int> cardsPerRound;

            std::cout << "Creating 2 card index... " << std::endl;
            cardsPerRound = vector<int>({2});
            Global::indexer_2.Construct(cardsPerRound);
            std::cout << Global::indexer_2.roundSize[0] << " non-isomorphic hands found" << std::endl;

            std::cout << "Creating 2 & 3 card index... " << std::endl;
            cardsPerRound = vector<int>({2, 3});
            Global::indexer_2_3.Construct(cardsPerRound);
            std::cout << Global::indexer_2_3.roundSize[1] << " non-isomorphic hands found" << std::endl;

            std::cout << "Creating 2 & 4 card index... " << std::endl;
            cardsPerRound = vector<int>({2, 4});
            Global::indexer_2_4.Construct(cardsPerRound);
            std::cout << Global::indexer_2_4.roundSize[1] << " non-isomorphic hands found" << std::endl;

            std::cout << "Creating 2 & 5 card index... " << std::endl;
            cardsPerRound = vector<int>({2, 5});
            Global::indexer_2_5.Construct(cardsPerRound);
            std::cout << Global::indexer_2_5.roundSize[1] << " non-isomorphic hands found" << std::endl;

            // std::cout << "Creating 2 & 5 & 2 card index... " << std::endl;
            // cardsPerRound = vector<int>({2, 5, 2});
            // Global::indexer_2_5_2.Construct(cardsPerRound);
            // std::cout << Global::indexer_2_5_2.roundSize[2] << " non-isomorphic hands found" << std::endl;

            // std::cout << "Creating 2 & 3 & 1 card index... " << std::endl;
            // cardsPerRound = vector<int>({2, 3, 1});
            // Global::indexer_2_3_1.Construct(cardsPerRound);
            // std::cout << Global::indexer_2_3_1.roundSize[2] << " non-isomorphic hands found" << std::endl;

            // std::cout << "Creating 2 & 3 & 1 & 1 card index... " << std::endl;
            // cardsPerRound = vector<int>({2, 3, 1, 1});
            // Global::indexer_2_3_1_1.Construct(cardsPerRound);
            // std::cout << Global::indexer_2_3_1_1.roundSize[3] << " non-isomorphic hands found" << std::endl;
        }

        static void CalculateInformationAbstraction()
        {
            std::cout << "Calculating information abstractions... " << std::endl;

            OCHSTable ochsTable = OCHSTable();
            ochsTable.Init();

            EMDTable emdTable = EMDTable();
            emdTable.Init();
        }

        static void Train()
        {
            std::cout << "Starting Monte Carlo Counterfactual Regret Minimization (MCCFRM)..." << std::endl;

            long StrategyInterval = max(1, 10000 / Global::NOF_THREADS); // bb rounds before updating player strategy (recursive through tree) 10k
            long PruneThreshold = 20000000 / Global::NOF_THREADS;        // bb rounds after this time we stop checking all actions, 200 minutes
            long LCFRThreshold = 20000000 / Global::NOF_THREADS;         // bb rounds when to stop discounting old regrets, no clue what it should be
            long DiscountInterval = 1000000 / Global::NOF_THREADS;       // bb rounds, discount values periodically but not every round, 10 minutes
            long SaveToDiskInterval = 100000 / Global::NOF_THREADS;
            long testGamesInterval = 100000 / Global::NOF_THREADS;

            atomic<long> sharedLoopCounter = 0;

            // Trainer trainer = Trainer(0);
            // trainer.EnumerateActionSpace(); // just for debugging purpose, I don't think it's necessary to enumeration the entire action space

            chrono::steady_clock::time_point start = chrono::steady_clock::now();

            oneapi::tbb::parallel_for(0, Global::NOF_THREADS,
                                      [&](int index)
                                      {
                                          Trainer trainer = Trainer(index);

                                          for (auto t = 1;; t++) // bb rounds
                                          {

                                              for (auto traverser = 0; traverser < Global::nofPlayers; traverser++) // traverser
                                              {
                                                  if (t % StrategyInterval == 0 && index == 0)
                                                  {
                                                      trainer.UpdateStrategy(traverser);
                                                  }

                                                  if (t > PruneThreshold)
                                                  {
                                                      float q = randDouble();
                                                      if (q < 0.05)
                                                      {
                                                          trainer.TraverseMCCFR(traverser, false);
                                                      }
                                                      else
                                                      {
                                                          trainer.TraverseMCCFR(traverser, true);
                                                      }
                                                  }
                                                  else
                                                  {
                                                      trainer.TraverseMCCFR(traverser, false);
                                                  }
                                              }

                                              if (t % 10000 == 0)
                                              {
                                                  sharedLoopCounter += 10000;
                                                  std::cout << "Training steps " << sharedLoopCounter << " "
                                                      << "thread " << index
                                                      << std::endl;

                                                  chrono::steady_clock::time_point end = chrono::steady_clock::now();
                                                  auto elapsed = chrono::duration_cast<std::chrono::seconds>(end - start).count();
                                                  std::cout << "Iterations per second: " << sharedLoopCounter / (elapsed + 1) << std::endl;
                                              }

                                              if (index != 0)
                                                  continue;

                                              if (t % testGamesInterval == 0) // implement progress bar later
                                              {
                                                  trainer.PrintStartingHandsChart();
                                                  trainer.PrintStatistics(sharedLoopCounter);

                                                  std::cout << "Sample games (against self)" << std::endl;
                                                  for (auto z = 0; z < 20; z++)
                                                  {
                                                      trainer.PlayOneGame();
                                                  }
                                              }
                                              if (t % SaveToDiskInterval == 0) // allow only one thread to do saving
                                              {
                                                  SaveToFile();
                                              }

                                              // discount all infosets (for all players)
                                              if (t < LCFRThreshold && t % DiscountInterval == 0) // allow only one thread to do discounting
                                              {
                                                  float d = ((float)t / DiscountInterval) / ((float)t / DiscountInterval + 1);
                                                  trainer.DiscountInfosets(d);
                                              }
                                          }
                                      });
        }

        static void SaveToFile()
        {
            std::cout << "Saving dictionary to file nodeMap.txt" << std::endl;
            utils::SaveToFile(Global::nodeMap, "nodeMap.txt");
        }

        static void LoadFromFile()
        {
            if (!utils::FileExists("nodeMap.txt"))
                return;
            std::cout << "Loading nodes from file nodeMap.txt..." << std::endl;
            utils::LoadFromFile(Global::nodeMap, "nodeMap.txt");
        }
    };
} // namespace poker

int main()
{
    poker::Program program;

    program.Main();

    return 0;
}
