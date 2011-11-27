#ifndef SQUARE_H_
#define SQUARE_H_

#include <vector>
#include "Location.h"

/*
    struct for representing a square in the grid.
*/
struct Square
{
    bool isVisible, isWater, isHill, isFood;
    int ant, hillPlayer;
    std::vector<int> deadAnts;

    int isLand; // -1 - unknown, 0 - no, 1 - yes
    // example
    //   ###
    // ### ###
    int isDeadlock; //-1 - unknown, 0 - no, 1 - yes // this square has only one open direction

    double pheromone;

    ants_t listOfEnemies; //list of enemies in attack range

    bool hasMoved;

    Square()
    {
        isVisible = isWater = isHill = isFood = 0;
        ant = hillPlayer = -1;
        pheromone = 1;
        isLand = -1;
        isDeadlock = -1;
        hasMoved = false;
    };

    //resets the information for the square except water information
    void reset()
    {
        isVisible = 0;
        isHill = 0;
        isFood = 0;
        ant = hillPlayer = -1;
        deadAnts.clear();
        listOfEnemies.clear();
        hasMoved = false;
    };
};

typedef std::vector<std::vector<Square> > grid_t;

#endif //SQUARE_H_
