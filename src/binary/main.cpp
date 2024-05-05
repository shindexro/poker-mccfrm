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

            StartGameForever();

            Train();
        }

        static void StartGameForever()
        {
            auto trainerManager = TrainerManager();
            trainerManager.LoadTrainedData();
            while (true)
            {
                StartGame();
            }
        }


        static void StartGame()
        {
            auto humanPlayer = make_shared<InteractivePlayer>(0, Global::buyIn);
            auto players = vector<shared_ptr<Player>>();
            players.push_back(humanPlayer);
            for (int i = 1; i < Global::nofPlayers; i++)
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
            auto trainerManager = TrainerManager(Global::NOF_THREADS);
            trainerManager.StartTraining();
        }
    };
} // namespace poker

int main()
{
    poker::Program program;

    program.Main();

    return 0;
}
