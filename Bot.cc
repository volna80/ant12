#include <math.h>
#include "Bot.h"

using namespace std;

//constructor
Bot::Bot()
{

};

//plays a single game of Ants.
void Bot::playGame()
{
    //reads the game parameters and sets up
    cin >> state;
    state.setup();
    endTurn();

    //continues making moves while the game is not over
    while(cin >> state)
    {
        state.updateVisionInformation();
        makeMoves();
        endTurn();
    }
};

//makes the bots moves for the turn
void Bot::makeMoves()
{
    state.bug << "turn " << state.turn << ":" << endl;
    state.bug << state << endl;

    //picks out moves for each ant
    for(int ant=0; ant<(int)state.myAnts.size(); ant++)
    {

        double p[TDIRECTIONS] = {0,0,0,0};
        for(int d=0; d<TDIRECTIONS; d++)
        {
            Location loc = state.getLocation(state.myAnts[ant], d);
            Square square = state.grid[loc.row][loc.col];

            if(square.isWater || square.ant == -1) {
                continue;
            }

            p[d] = pow(square.pheromone, COMMINITY) * pow(calcDesirability(loc), GREEDY);

        //    if(!state.grid[loc.row][loc.col].isWater)
        //    {
        //       state.makeMove(state.myAnts[ant], d);
        //        break;
        //    }
        //#1 calculate probability of every options

        }
    }

    state.bug << "time taken: " << state.timer.getTime() << "ms" << endl << endl;
};

//finishes the turn
void Bot::endTurn()
{
    if(state.turn > 0){
        state.reset();
    }
    state.turn++;

    cout << "go" << endl;
};
