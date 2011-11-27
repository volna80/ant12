#ifndef LOCATION_H_
#define LOCATION_H_

#include <vector>
#include "Bug.h"


/*
    struct for representing locations in the grid.
*/
struct Location
{
    int row, col;

    Location()
    {
        row = col = 0;
    };

    Location(int r, int c)
    {
        row = r;
        col = c;
    };


    bool operator==(const Location &other) const {
        return row == other.row && col == other.col;
    }

    bool operator!=(const Location &other) const {
        return row != other.row || col != other.col;
    }



};

typedef std::vector<Location> ants_t;

Bug& operator<<(Bug &os, const Location &state) ;



#endif //LOCATION_H_
