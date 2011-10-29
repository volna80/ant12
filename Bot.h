#ifndef BOT_H_
#define BOT_H_

#include "State.h"


const double GREEDY = 0.5;
const double COMMINITY = 1 - GREEDY;

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
};

#endif //BOT_H_
