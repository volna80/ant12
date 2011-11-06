#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "Bot.h"

using namespace std;

void Bot::findPath(Location const &loc, vector<vector<int> > &graph, void (*f)(const Location&, State&))
{
    //add pheromone to a loc
    //state.grid[loc.row][loc.col].pheromone = 1000;
    //state.bug << "add pheromone at " << loc << endl;

    (*f)(loc, state);

    int time = graph[loc.row][loc.col];
    if(time == 0)
    {
        //finish
        //state.bug << "finish at " << loc << endl;
        return;
    }
    time--;
    for(int d=0; d < TDIRECTIONS; d++)
    {
        Location nLoc = state.getLocation(loc,d);
        if(graph[nLoc.row][nLoc.col] == time)
        {
            findPath(nLoc,graph, f);
            return;
        }
    }
};

void Bot::findPath(Location const &from, vector<Location> const &to, void (*f)(const Location&, State&))
{
    vector<vector<int> > graph = vector<vector<int> >(state.rows,vector<int>(state.cols,-1));

    vector<Location> newFront;
    vector<Location> oldFront;

    int time = 0;

    //step one: init
    graph[from.row][from.col] = time;
    oldFront.push_back(from);

    while(true)
    {
        time++;
        //update near vertex
        //state.bug << "1# update near vertex " << endl;
        for(vector<Location>::iterator currentVertex = oldFront.begin(); currentVertex != oldFront.end(); ++currentVertex)
        {
            for(int d=0; d<TDIRECTIONS; d++)
            {
                Location loc = state.getLocation( *currentVertex, d);
                if(graph[loc.row][loc.col] == -1 && (state.grid[loc.row][loc.col].isLand == 1))
                {
                    graph[loc.row][loc.col] = time;
                    newFront.push_back(loc);
                }
            }
        }

        if(newFront.size() == 0)
        {
            //couldn't find a path
            //state.bug << "couldn't find a path" << endl;
            break;
        }


        for(vector<Location>::const_iterator target = to.begin(); target != to.end(); ++target)
        {
            //Location * p = find(newFront.begin(), newFront.end(), *myHill);
            for(int i=0; i < newFront.size(); i++)
            {
                if(*target == newFront[i])
                {
                    //found a path
                    //add pheromone on a path

                    //state.bug << "found a path" <<endl;

                    findPath(*target, graph, f);

                    return;
                }
            }
        }

        //next round
        //state.bug << "swap for next round " << endl;
        oldFront = newFront;
        newFront.clear();
    }
}
