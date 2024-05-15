#include "binary/main.h"

namespace poker
{
    class Program
    {
    public:
        static void Main(int argc, char** argv)
        {
            CreateIndexers();
            Global::handEvaluator->Initialise();
            CalculateInformationAbstraction();

            if (argc > 1 && strcmp(argv[1], "play") == 0)
            {
                StartGameForever();
            } else {
                Train();
            }
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

int main(int argc, char** argv)
{
    poker::Program program;

    program.Main(argc, argv);

    return 0;
}
