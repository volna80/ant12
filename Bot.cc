#include <stdio.h>
#include <stdlib.h>
#include <time.h>
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

    srand( state.seed);


    //continues making moves while the game is not over
    while(cin >> state)
    {
        state.updateVisionInformation();

        state.bug << "turn " << state.turn << ":" << endl;
        state.bug << state << endl;

        updatePheromone();
        makeMoves();
        endTurn();
    }
};

void addPheromone(Location const &loc, State &state){
    state.grid[loc.row][loc.col].pheromone = FOOD_PHEROMONE;
    //state.bug << "add pheromone at " << loc << endl;
}



void Bot::updatePheromone()
{
    //if(state.turn % 25 == 0)
    {
        //init first paths
        for(vector<Location>::iterator food = state.food.begin(); food != state.food.end(); ++food)
        {
            //state.bug << "look a path for " << *food << endl;
            findPath(*food, state.myHills, addPheromone);
        }

        for(vector<Location>::iterator hill = state.enemyHills.begin(); hill != state.enemyHills.end(); ++hill)
        {
            //state.bug << "look a path for " << *hill << endl;
            findPath(*hill, state.myHills, addPheromone);
        }
    }

    //decrease pheromone
    for(int row=0; row<state.rows; row++)
        for(int col=0; col<state.cols; col++)
        {
            if(state.grid[row][col].pheromone != 1)
            {
                state.grid[row][col].pheromone -= 25;
            }
            if(state.grid[row][col].pheromone < 1)
            {
                state.grid[row][col].pheromone=1;
            }

        }


}



//makes the bots moves for the turn
void Bot::makeMoves()
{

    //picks out moves for each ant
    for(int ant=0; ant<(int)state.myAnts.size(); ant++)
    {
        state.bug << "start turn ant[" << ant << "]" << endl;

        double w[TDIRECTIONS] = {0,0,0,0}; //weights of every path
        double sumW = 0;

        for(int d=0; d<TDIRECTIONS; d++)
        {
            Location loc = state.getLocation(state.myAnts[ant], d);
            Square * square = &state.grid[loc.row][loc.col];

            if(square->isWater || square->ant == 0 || (square->isHill && square->hillPlayer ==0) || square->isDeadlock == 1)
            {
                //state.bug << "couldn't move to " << d << endl;
                //state.bug << "loc:" << loc.row << ":" << loc.col << "; w:" << square.isWater << "; h:" << square.isHill << "; a:" << square.ant << endl;
                continue;
            }

            int phe = square->pheromone;
            int des = calcDesirability(state.myAnts[ant], d);
            w[d] = pow(phe, COMMINITY) * pow(des, GREEDY);
            state.bug << CDIRECTIONS[d] << " phe=" << phe << "; des=" << des << "; w=" << w[d] << endl;
            sumW += w[d];

            //    if(!state.grid[loc.row][loc.col].isWater)
            //    {
            //       state.makeMove(state.myAnts[ant], d);
            //        break;
            //    }
            //#1 calculate probability of every options

        }

        // state.bug << "sumW=" << sumW << endl;


        if(sumW == 0)
        {
            continue;
        }

        double p[TDIRECTIONS] = {0,0,0,0}; //probability
        int r = rand() % 1000;

        state.bug << "r:" << r << "; ";
        double aggr = 0;

        for(int d=0; d < TDIRECTIONS ; d++)
        {
            aggr += (w[d] / sumW);
            state.bug << "p" << d << "=" << (1000 * aggr) << "; ";
            if(aggr * 1000 > ( r - 1))
            {
                state.bug << "makeMove: " << ant << ":" << CDIRECTIONS[d] << endl;
                state.makeMove(state.myAnts[ant], d);
                break;
            }
        }

        //make decision


    }

    state.bug << "time taken: " << state.timer.getTime() << "ms" << endl << endl;
};

int Bot::calcDesirability(const Location &current, int direction)
{
    int d = 1;


    //new location
    Location l = state.getLocation(current, direction);


    for(int c= 0 - state.viewradius; c <= state.viewradius; c++)
    {
        for(int r= 0-state.viewradius; r<= state.viewradius; r++)
        {
            Location nLoc = Location((l.row + r + state.rows) % state.rows, (l.col + c + state.cols) % state.cols);

            Square * square = &state.grid[nLoc.row][nLoc.col];
//            state.bug << "check r:" <<  nLoc.row << ", c:" << nLoc.col << ", sq=" << square << endl;

            if(square->isFood)
            {
                int tmp = W_FOOD / state.distance(l, nLoc);
                d += tmp;
                state.bug << "+food[" << tmp << "]; ";
            }

            if(square->isHill && square->hillPlayer != 0)
            {
                int tmp;
                if(c==0 && r==0)
                {
                    tmp = W_HILL;
                }
                else
                {
                    tmp = W_HILL / state.distance(l, nLoc);
                }
                d += tmp;
                state.bug << "+hill[" << tmp << "]";
            }
        }
    }

    //go on in the same direction is more prefereable
    if(state.lastTurn[current.row][current.col] == direction){
        d += D_SAME_DIRECTION;
        state.bug << "+dir[" << D_SAME_DIRECTION << "]; ";
    }

    state.bug << endl;

    return d;
}

//finishes the turn
void Bot::endTurn()
{
    if(state.turn > 0)
    {
        state.reset();
    }
    state.turn++;

    cout << "go" << endl;
};
