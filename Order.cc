#include "Order.h"
#include "State.h"

using namespace std;



ostream& operator<<(ostream &os, const Order &order) {
    os << "o " << order.from.row << " " << order.from.col << " " << CDIRECTIONS[order.direction] << endl;
    return os;
}
