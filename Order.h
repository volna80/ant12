#ifndef ORDER_H_INCLUDED
#define ORDER_H_INCLUDED

#include <iostream>
#include <stdio.h>
#include "Location.h"

struct Order {
    Location from;
    int direction;
};

std::ostream& operator<<(std::ostream &os, const Order &order);



#endif // ORDER_H_INCLUDED
