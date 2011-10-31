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

void Bot::findPath(Location &loc, vector<vector<int> > &graph){
    //add pheromone to a loc
    state.grid[loc.row][loc.col].pheromone = 1000;
    state.bug << "add pheromone at " << loc << endl;
    int time = graph[loc.row][loc.col];
    if(time == 0){
        //finish
        state.bug << "finish at " << loc << endl;
        return;
    }
    time--;
    for(int d=0; d < TDIRECTIONS; d++){
        Location nLoc = state.getLocation(loc,d);
        if(graph[nLoc.row][nLoc.col] == time){
            findPath(nLoc,graph);
            return;
        }
    }
};


void Bot::updatePheromone(){
    if(state.turn % 25 == 0){
        //init first paths
        for(vector<Location>::iterator food = state.food.begin(); food != state.food.end(); ++food) {
            state.bug << "look a path for " << *food << endl;

            vector<vector<int> > graph = vector<vector<int> >(state.rows,vector<int>(state.cols,-1));

            vector<Location> newFront;
            vector<Location> oldFront;

            int time = 0;

            //step one: init
            graph[food->row][food->col] = time;
            oldFront.push_back(*food);

            while(true){
                time++;
                //update near vertex
                state.bug << "1# update near vertex " << endl;
                for(vector<Location>::iterator currentVertex = oldFront.begin(); currentVertex != oldFront.end(); ++currentVertex){
                    for(int d=0; d<TDIRECTIONS; d++){
                        Location loc = state.getLocation( *currentVertex, d);
                        if(graph[loc.row][loc.col] == -1 && (state.grid[loc.row][loc.col].isLand == 1)){
                            graph[loc.row][loc.col] = time;
                            newFront.push_back(loc);
                            state.bug << "new front= " << loc << endl;
                        }
                    }
                }

                if(newFront.size() == 0){
                    //couldn't find a path
                    state.bug << "couldn't find a path" << endl;
                    break;
                }


                for(vector<Location>::iterator myHill = state.myHills.begin(); myHill != state.myHills.end(); ++myHill){
                    //Location * p = find(newFront.begin(), newFront.end(), *myHill);
                    for(int i=0; i < newFront.size(); i++){
                        if(*myHill == newFront[i]){
                            //found a path
                            //add pheromone on a path

                            findPath(*myHill, graph);

                            goto pathFound;
                        }
                    }
                }

                //next round
                state.bug << "swap for next round " << endl;
                oldFront = newFront;
                newFront.clear();
            }

            pathFound:
            state.bug << "found a path" <<endl;

        }
    }

}


//makes the bots moves for the turn
void Bot::makeMoves()
{

    //picks out moves for each ant
    for(int ant=0; ant<(int)state.myAnts.size(); ant++)
    {

        double w[TDIRECTIONS] = {0,0,0,0}; //weights of every path
        double sumW = 0;

        for(int d=0; d<TDIRECTIONS; d++)
        {
            Location loc = state.getLocation(state.myAnts[ant], d);
            Square * square = &state.grid[loc.row][loc.col];

            if(square->isWater || square->ant != -1 || (square->isHill && square->hillPlayer !=0)) {
                //state.bug << "couldn't move to " << d << endl;
                //state.bug << "loc:" << loc.row << ":" << loc.col << "; w:" << square.isWater << "; h:" << square.isHill << "; a:" << square.ant << endl;
                continue;
            }

            w[d] = pow(square->pheromone, COMMINITY) * pow(calcDesirability(loc), GREEDY);
            state.bug <<  "w[" << d << "]=" << w[d] << "; ";
            sumW += w[d];

        //    if(!state.grid[loc.row][loc.col].isWater)
        //    {
        //       state.makeMove(state.myAnts[ant], d);
        //        break;
        //    }
        //#1 calculate probability of every options

        }

        state.bug << "sumW=" << sumW << endl;


        if(sumW == 0) {
            continue;
        }

        double p[TDIRECTIONS] = {0,0,0,0}; //probability
        int r = rand() % 1000;

        state.bug << "r:" << r << "; ";
        double aggr = 0;

        for(int d=0; d < TDIRECTIONS ; d++){
            aggr += (w[d] / sumW);
            state.bug << "p" << d << "=" << (1000 * aggr) << "; ";
            if(aggr * 1000 > ( r - 1)){
                state.bug << "makeMove: " << ant << ":" << d << endl;
                state.makeMove(state.myAnts[ant], d);
                break;
            }
        }

        //make decision


    }

    state.bug << "time taken: " << state.timer.getTime() << "ms" << endl << endl;
};

double Bot::calcDesirability(const Location &l){
    double d = 1;


    for(int c= 0 - state.viewradius; c <= state.viewradius; c++) {
        for(int r= 0-state.viewradius; r<= state.viewradius; r++){
            Location nLoc = Location((l.row + r + state.rows) % state.rows, (l.col + c + state.cols) % state.cols);

            Square * square = &state.grid[nLoc.row][nLoc.col];
//            state.bug << "check r:" <<  nLoc.row << ", c:" << nLoc.col << ", sq=" << square << endl;

            if(square->isFood){
                d += W_FOOD / state.distance(l, nLoc);
            }

            if(square->isHill && square->hillPlayer != 0) {
                d += W_HILL / state.distance(l, nLoc);
            }
        }
    }


    return d;
}

//finishes the turn
void Bot::endTurn()
{
    if(state.turn > 0){
        state.reset();
    }
    state.turn++;

    cout << "go" << endl;
};
