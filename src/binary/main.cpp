#include "game/card.h"
#include "game/deck.h"
#include <iostream>
#include <string>

class Program
{
    public:
    static void Test()
    {
        cout << "Testing card creation" << endl;
        Card card = Card("Ts");
        cout << card.ToString() << endl;

        cout << "Testing deck creation" << endl;
        Deck deck = Deck(0);
        deck.Draw_(3);
        cout << deck.NumRemainingCards() << endl;
    }
};

int main(int argc, char *argv[])
{
    Program program;
    program.Test();
    
    return 0;
}
