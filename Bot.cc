#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <algorithm>
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

    state.bug << "attackradius=" << state.attackradius << endl;
    state.bug << "viewradius=" << state.viewradius << endl;
    state.bug << "spawnradius=" << state.spawnradius << endl;


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

void addFoodPheromone(Location const &loc, State &state)
{
    state.grid[loc.row][loc.col].pheromone = FOOD_PHEROMONE;
}

void addHillPheromone(Location const &loc, State &state)
{
    state.grid[loc.row][loc.col].pheromone = HILL_PHEROMONE;
}



void Bot::updatePheromone()
{
    //if(state.turn % 25 == 0)
    {
        //init first paths
        for(vector<Location>::iterator food = state.food.begin(); food != state.food.end(); ++food)
        {
            //state.bug << "look a path for " << *food << endl;
            findPath(*food, state.myHills, addFoodPheromone);
        }

        for(vector<Location>::iterator hill = state.enemyHills.begin(); hill != state.enemyHills.end(); ++hill)
        {
            //state.bug << "look a path for " << *hill << endl;
            findPath(*hill, state.myHills, addHillPheromone);
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

    vector<BattleArea> battles = findBattles();
    state.bug << "time taken(split grous)" << state.timer.getTime() << "ms" << endl << endl;

    //make moves of fighting groups first
    for(int i = 0 ; i < battles.size(); i++)
    {
        BattleArea & area = battles[i];
        makeMoves(area);
    }
    state.bug << "time taken(make battle moves): " << state.timer.getTime() << "ms" << endl << endl;


    //picks out moves for rest ants
    for(int ant=0; ant<(int)state.myAnts.size(); ant++)
    {
        state.bug << "start turn ant[" << ant << "]" << endl;

        //check that the and isn't in a battle

        double w[TDIRECTIONS] = {0,0,0,0}; //weights of every path
        double sumW = 0;

        for(int d=0; d<TDIRECTIONS; d++)
        {
            Location loc = state.getLocation(state.myAnts[ant], d);
            Square * square = &state.grid[loc.row][loc.col];

            if(square->isWater || square->ant == 0 || (square->isHill && square->hillPlayer ==0) || square->isDeadlock == 1 || square->isFood)
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

        //make a decision

        if(state.turn % 5 == 0)
        {
            //random aproach
            double p[TDIRECTIONS] = {0,0,0,0}; //probability
            int r = rand() % 1000;

            //state.bug << "r:" << r << "; ";
            double aggr = 0;

            for(int d=0; d < TDIRECTIONS ; d++)
            {
                aggr += (w[d] / sumW);
                //state.bug << "p" << d << "=" << (1000 * aggr) << "; ";
                if(aggr * 1000 > ( r - 1))
                {
                    state.bug << "move to [" << CDIRECTIONS[d] << "]" << endl;
                    state.makeMove(state.myAnts[ant], d);
                    break;
                }
            }

        }
        else
        {
            //look for max
            int mov = -1;
            double maxP = 0;
            for(int i = 0; i < TDIRECTIONS; i++)
            {
                if(w[i] > 0 && w[i] > maxP)
                {
                    mov = i;
                    maxP = w[i];
                }
            }

            if(mov != -1)
            {
                state.bug << "move to [" << CDIRECTIONS[mov] << "]" << endl;
                state.makeMove(state.myAnts[ant], mov);
            }
            else
            {
                state.bug << "stay on the same place " << endl;
            }
        }

        //make decision


    }

    state.bug << "time taken: " << state.timer.getTime() << "ms" << endl << endl;
};



int Bot::calcDesirability(const Location &current, int direction)
{


    int d = 1;

    //go on in the same direction is more prefereable
    if(state.lastTurn[current.row][current.col] == direction)
    {
        d += D_SAME_DIRECTION;
        state.bug << "+dir[" << D_SAME_DIRECTION << "]; ";
    }
    else if(state.lastTurn[current.row][current.col] == (direction + 2) % TDIRECTIONS) //180%
    {
        //nothing
    }
    else
    {
        //90%
        d += D_SAME_DIRECTION / 4;
        state.bug << "+dir[" << (D_SAME_DIRECTION / 4 ) << "]; ";
    }

    for(int deep = 0; deep < 5 ; deep++)
    {
        Location ll = state.getLocation(current, direction, deep);

        if(state.grid[ll.row][ll.col].isWater)
        {
            break;
        }

        for(int j=-deep; j <= deep ; j++)
        {
            int r = current.row;
            int c = current.col;
            if(direction == NORTH)
            {
                r -= deep;
                c += j;
            }
            else if(direction == EAST)
            {
                r += j;
                c += deep;
            }
            else if(direction == SOUTH)
            {
                r += deep;
                c += j;
            }
            else     //direction WEST
            {
                r += j;
                c -= deep;
            }
            r = (r + state.rows) % state.rows;
            c = (c + state.cols) % state.cols;

            Location l = Location(r,c);

            Square * square = &state.grid[l.row][l.col];

//            state.bug << "check r:" <<  nLoc.row << ", c:" << nLoc.col << ", sq=" << square << endl;

            if(square->isFood)
            {
                int tmp;
                if(deep == 0)
                {
                    tmp = W_FOOD;
                }
                else
                {
                    tmp = W_FOOD/ deep;
                }
                d += tmp;
                state.bug << "+food[" << tmp << "]; ";
            }

            if(square->isHill && square->hillPlayer != 0)
            {
                int tmp;
                if(deep == 0)
                {
                    tmp = W_HILL;
                }
                else
                {
                    tmp = W_HILL / deep;
                }
                d += tmp;
                state.bug << "+hill[" << tmp << "]; ";
            }

            if(square->isLand == -1)
            {
                d += W_EXPLORATION;
                state.bug << "+expl[" << W_EXPLORATION << "]; ";
            }
        }
    }


    //new location
    //Location l = state.getLocation(current, direction);


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
