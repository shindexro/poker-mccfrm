#include "game/card.h"
#include <iostream>
#include <string>

class Program
{
    public:
    static void Index()
    {
        cout << "Creating 2 card index..." << endl;
        Card card = Card("Ts");
        cout << card.ToString() << endl;
    }
};

int main(int argc, char *argv[])
{
    Program program;
    program.Index();
    
    return 0;
}
