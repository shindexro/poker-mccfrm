#include "game/card.h"
#include "game/deck.h"
#include "abstraction/global.h"
#include "abstraction/infoset.h"
#include "abstraction/chance_state.h"
#include "tables/hand_indexer.h"
#include "tables/ochs_table.h"
#include "tables/emd_table.h"
#include "utils/utils.h"
#include "utils/random.h"
#include "algorithm/trainer.h"
#include "abstraction/state.h"

#include <iostream>
#include <string>

namespace poker
{
    void StateDFS(State &state, int depth = 0)
    {
        if (depth >= 4)
            return;

        state.CreateChildren();
        for (auto &child : state.children)
        {
            StateDFS(*child, depth + 1);
        }
    }

    class Program
    {
    public:
        static void Main()
        {
            auto state = ChanceState();
            StateDFS(state);
            state.PrettyPrintTree();

            // CreateIndexers();
            // Global::handEvaluator.Initialise();
            // CalculateInformationAbstraction();
            // Train();
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

            std::cout << "Creating 2 & 5 & 2 card index... " << std::endl;
            cardsPerRound = vector<int>({2, 5, 2});
            Global::indexer_2_5_2.Construct(cardsPerRound);
            std::cout << Global::indexer_2_5_2.roundSize[2] << " non-isomorphic hands found" << std::endl;

            std::cout << "Creating 2 & 3 & 1 card index... " << std::endl;
            cardsPerRound = vector<int>({2, 3, 1});
            Global::indexer_2_3_1.Construct(cardsPerRound);
            std::cout << Global::indexer_2_3_1.roundSize[2] << " non-isomorphic hands found" << std::endl;

            std::cout << "Creating 2 & 3 & 1 & 1 card index... " << std::endl;
            cardsPerRound = vector<int>({2, 3, 1, 1});
            Global::indexer_2_3_1_1.Construct(cardsPerRound);
            std::cout << Global::indexer_2_3_1_1.roundSize[3] << " non-isomorphic hands found" << std::endl;
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

            LoadFromFile();
            LoadFromFile_d();

            Trainer trainer = Trainer(0);
            trainer.EnumerateActionSpace();

            chrono::steady_clock::time_point start = chrono::steady_clock::now();

            oneapi::tbb::parallel_for(0, Global::NOF_THREADS,
                                      [&](int index)
                                      {
                                          Trainer trainer = Trainer(index);

                                          for (auto t = 1;; t++) // bb rounds
                                          {
                                              if (t % 1000 == 0)
                                              {
                                                  sharedLoopCounter += 1000;
                                                  std::cout << "Training steps " << sharedLoopCounter << std::endl;
                                              }

                                              if (t % testGamesInterval == 0 && index == 0) // implement progress bar later
                                              {
                                                  trainer.PrintStartingHandsChart();
                                                  trainer.PrintStatistics(sharedLoopCounter);

                                                  std::cout << "Sample games (against self)" << std::endl;
                                                  ;
                                                  for (auto z = 0; z < 20; z++)
                                                  {
                                                      trainer.PlayOneGame();
                                                  }

                                                  // std::cout << "Sample games (against baseline)");
                                                  // float mainScore = 0.0f;
                                                  // for (auto x = 0; x < 100; x++) // 100 games not statistically significant
                                                  //{
                                                  //     if (x < 20)
                                                  //     {
                                                  //         mainScore += trainer.PlayOneGame_d(x % 2, true);
                                                  //     }
                                                  //     mainScore += trainer.PlayOneGame_d(x % 2, false);
                                                  // }
                                                  // WritePlotStatistics((mainScore / 10000) / Global::BB);
                                                  // std::cout << "BBs per hand: {0}", (mainScore / 10000) / Global::BB);

                                                  chrono::steady_clock::time_point end = chrono::steady_clock::now();
                                                  auto elapsed = chrono::duration_cast<std::chrono::seconds>(end - start).count();
                                                  std::cout << "Iterations per second: " << sharedLoopCounter / (elapsed + 1) << std::endl;
                                              }
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
                                                          trainer.TraverseMCCFR(traverser, t);
                                                      }
                                                      else
                                                      {
                                                          trainer.TraverseMCCFRPruned(traverser);
                                                      }
                                                  }
                                                  else
                                                  {
                                                      trainer.TraverseMCCFR(traverser, t);
                                                  }
                                              }
                                              if (t % SaveToDiskInterval == 0 && index == 0) // allow only one thread to do saving
                                              {
                                                  //   std::cout << "Saving nodeMap to disk disabled!" << std::endl;
                                                  SaveToFile();
                                              }

                                              // discount all infosets (for all players)
                                              if (t < LCFRThreshold && t % DiscountInterval == 0 && index == 0) // allow only one thread to do discounting
                                              {
                                                  float d = ((float)t / DiscountInterval) / ((float)t / DiscountInterval + 1);
                                                  trainer.DiscountInfosets(d);
                                              }
                                          }
                                      });
        }

        // static void WritePlotStatistics(float bbWins)
        // {
        //     using(StreamWriter file = new StreamWriter("progress.txt", true))
        //     {
        //         file.WriteLine(Math.Round(bbWins, 2));
        //     }
        // }

        static void SaveToFile_d()
        {
            std::cout << "Saving dictionary to file nodeMap_d.txt" << std::endl;
            utils::SaveToFile(Global::nodeMapBaseline, "nodeMap_d.txt");
        }

        static void LoadFromFile_d()
        {
            if (!utils::FileExists("nodeMap_d.txt"))
                return;
            std::cout << "Loading nodes from file nodeMap_d.txt..." << std::endl;
            utils::LoadFromFile(Global::nodeMapBaseline, "nodeMap_d.txt");
        }

        static void SaveToFile()
        {
            std::cout << "Saving dictionary to file nodeMap.txt" << std::endl;
            utils::SaveToFile(Global::nodeMap, "nodeMap.txt");

            std::cout << "Saving dictionary to file nodeMap.txt" << std::endl;
        }

        static void LoadFromFile()
        {
            if (!utils::FileExists("nodeMap.txt"))
                return;
            std::cout << "Loading nodes from file nodeMap.txt..." << std::endl;
            utils::LoadFromFile(Global::nodeMap, "nodeMap.txt");
        }

        // template <typename T>
        // static vector<byte> SerializeToBytes<T>(T item)
        // {
        //     var formatter = new BinaryFormatter();
        //     using var stream = new MemoryStream();
        //     formatter.Serialize(stream, item);
        //     stream.Seek(0, SeekOrigin.Begin);
        //     return stream.ToArray();
        // }

        // static Infoset Deserialize(vector<byte> &byteArray)
        // {
        //     if (byteArray == null)
        //     {
        //         return null;
        //     }
        //     using var memStream = new MemoryStream();
        //     var binForm = new BinaryFormatter();
        //     memStream.Write(byteArray, 0, byteArray.Length);
        //     memStream.Seek(0, SeekOrigin.Begin);
        //     Infoset obj = (Infoset)binForm.Deserialize(memStream);
        //     return obj;
        // }
    };
} // namespace poker

int main()
{
    poker::Program program;
    program.Main();

    return 0;
}
