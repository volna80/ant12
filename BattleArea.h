#ifndef BATTLEAREA_H_INCLUDED
#define BATTLEAREA_H_INCLUDED

#include <vector>
#include "Location.h"
#include "Bug.h"

struct BattleArea
{
    ants_t myAnts;
    ants_t enemy;

};

Bug& operator<<(Bug &os, const BattleArea &state) ;


#endif // BATTLEAREA_H_INCLUDED
