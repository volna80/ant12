#ifndef BOT_H_
#define BOT_H_

#include <vector>
#include "State.h"


const double GREEDY = 0.5;
const double COMMINITY = 1 - GREEDY;

const double W_FOOD = 10000; //weigh if we see food
const double W_HILL = 100000; // + if we see enemy hill

const int D_SAME_DIRECTION = 300;
const int FOOD_PHEROMONE = 1000; //add to a path from hill to a food
const int W_EXPLORATION = 100;


/*
    This struct represents your bot in the game of Ants
*/
struct Bot
{
    State state;

    Bot();

    int calcDesirability(const Location &l, int direction); //calculate a desirability of move in this direction from the location

    void playGame();    //plays a single game of Ants

    void makeMoves();   //makes moves for a single turn
    void endTurn();     //indicates to the engine that it has made its moves

    void updatePheromone(); //update paths for ants from all food&enemy hill's locations
    //void addPheromone(Location const &loc);


    //find a path from the loc to the nearest hill and add pheromone on a path
    //we use a wave pathfinder algorithm
    void findPath(Location const &from, std::vector<Location> const &to, void (*f)(const Location&, State&));
    void findPath(Location const &loc, std::vector<std::vector<int> > &graph, void (*f)(const Location&, State&));
};

#endif //BOT_H_
