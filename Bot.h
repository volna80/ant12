#ifndef BOT_H_
#define BOT_H_

#include <vector>
#include "State.h"


const double GREEDY = 0.7;
const double COMMINITY = 1 - GREEDY;
const double W_FOOD = 1000;
const double W_HILL = 10000;

/*
    This struct represents your bot in the game of Ants
*/
struct Bot
{
    State state;

    Bot();

    double calcDesirability(const Location &l); //calculate a desirability of move to this location

    void playGame();    //plays a single game of Ants

    void makeMoves();   //makes moves for a single turn
    void endTurn();     //indicates to the engine that it has made its moves

    void updatePheromone(); //update paths for ants from all food&enemy hill's locations
    //find a path from the loc to the nearest hill and add pheromone on a path
    //we use a wave pathfinder algorithm
    void findPath(Location &loc);
    void findPath(Location &loc, std::vector<std::vector<int> > &graph);
};

#endif //BOT_H_
