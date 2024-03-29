#include "State.h"
#include "BattleArea.h"

using namespace std;

//constructor
State::State()
{
    gameover = 0;
    turn = 0;
    bug.open("./debug.txt");
};

//deconstructor
State::~State()
{
    bug.close();
};

//sets the state up
void State::setup()
{
    grid = vector<vector<Square> >(rows, vector<Square>(cols, Square()));
    lastTurn = vector<vector<int> >(rows, vector<int>(cols, -1));
};

//resets all non-water squares to land and clears the bots ant vector
void State::reset()
{
    myAnts.clear();
    enemyAnts.clear();
    myHills.clear();
    enemyHills.clear();
    food.clear();
    for(int row=0; row<rows; row++)
        for(int col=0; col<cols; col++)
            if(!grid[row][col].isWater)
                grid[row][col].reset();
};

//outputs move information to the engine
void State::makeMove(const Location &loc, int direction)
{
    if(grid[loc.row][loc.col].hasMoved)
    {
        //TODO BUG ???
        return;
    }

    cout << "o " << loc.row << " " << loc.col << " " << CDIRECTIONS[direction] << endl;

    Location nLoc = getLocation(loc, direction);
    grid[nLoc.row][nLoc.col].ant = grid[loc.row][loc.col].ant;
    grid[loc.row][loc.col].ant = -1;
    //remember from what direction we come
    lastTurn[nLoc.row][nLoc.col] = direction;
    lastTurn[loc.row][loc.col] = -1;

    grid[loc.row][loc.col].hasMoved = true;

};

//returns the euclidean distance between two locations with the edges wrapped
double State::distance(const Location &loc1, const Location &loc2)
{
    int d1 = abs(loc1.row-loc2.row),
        d2 = abs(loc1.col-loc2.col),
        dr = min(d1, rows-d1),
        dc = min(d2, cols-d2);
    return sqrt(dr*dr + dc*dc);
};

//returns the new location from moving in a given direction with the edges wrapped
Location State::getLocation(const Location &loc, int direction)
{
    if(direction == DONT_MOVE) return loc;

    return Location( (loc.row + DIRECTIONS[direction][0] + rows) % rows,
                     (loc.col + DIRECTIONS[direction][1] + cols) % cols );
};

Location State::getLocation(const Location &loc, int direction, int distance)
{
    return Location( (loc.row + DIRECTIONS[direction][0] * distance + rows) % rows,
                     (loc.col + DIRECTIONS[direction][1] * distance + cols) % cols );
};

/*
    This function will update update the lastSeen value for any squares currently
    visible by one of your live ants.

    BE VERY CAREFUL IF YOU ARE GOING TO TRY AND MAKE THIS FUNCTION MORE EFFICIENT,
    THE OBVIOUS WAY OF TRYING TO IMPROVE IT BREAKS USING THE EUCLIDEAN METRIC, FOR
    A CORRECT MORE EFFICIENT IMPLEMENTATION, TAKE A LOOK AT THE GET_VISION FUNCTION
    IN ANTS.PY ON THE CONTESTS GITHUB PAGE.
*/
void State::updateVisionInformation()
{
    std::queue<Location> locQueue;
    Location sLoc, cLoc, nLoc;

    for(int a=0; a<(int) myAnts.size(); a++)
    {
        sLoc = myAnts[a];
        locQueue.push(sLoc);

        std::vector<std::vector<bool> > visited(rows, std::vector<bool>(cols, 0));
        grid[sLoc.row][sLoc.col].isVisible = 1;
        grid[sLoc.row][sLoc.col].isLand = 1;
        visited[sLoc.row][sLoc.col] = 1;

        while(!locQueue.empty())
        {
            cLoc = locQueue.front();
            locQueue.pop();

            for(int d=0; d<TDIRECTIONS; d++)
            {
                nLoc = getLocation(cLoc, d);

                if(!visited[nLoc.row][nLoc.col] && distance(sLoc, nLoc) <= viewradius)
                {
                    grid[nLoc.row][nLoc.col].isVisible = 1;
                    locQueue.push(nLoc);

                    if(grid[nLoc.row][nLoc.col].isWater){
                        grid[nLoc.row][nLoc.col].isLand = 0;
                    } else {
                        grid[nLoc.row][nLoc.col].isLand = 1;
                    }
                }
                visited[nLoc.row][nLoc.col] = 1;
            }
        }
    }


    //every 20 turn, check deadlock areas;
    //if((turn % 20 == 0) {
    updateDeadlock();
    //}

    for(int a = 0; a <(int) myAnts.size(); a++)
    {
        Location &loc = myAnts[a];
        enemies(loc, grid, 0);
    }

    for(int a = 0; a <(int) enemyAnts.size(); a++)
    {
        Location &loc = enemyAnts[a];
        enemies(loc, grid, grid[loc.row][loc.col].ant);
    }
};


void State::updateDeadlock()
{
    for(int r=0; r < rows; r++)
    {
        for(int c=0; c < cols; c++)
        {
            if(grid[r][c].isDeadlock == -1 && grid[r][c].isLand == 1)
            {
                int free = 0; //number of free directions
                Location loc = Location(r,c);
                for(int d=0; d<TDIRECTIONS; d++)
                {
                    Location nLoc = getLocation(loc, d);
                    if(grid[nLoc.row][nLoc.col].isLand == 1){
                        free++;
                    } else if(grid[nLoc.row][nLoc.col].isLand == -1){
                        free = -1;
                        break;
                    }
                }

                if(free == -1){
                    //do nothing
                } else if(free == 1){
                    grid[r][c].isDeadlock = 1;
                } else {
                    grid[r][c].isDeadlock = 0;
                }
            }
        }
    }
}

/*
    This is the output function for a state. It will add a char map
    representation of the state to the output stream passed to it.

    For example, you might call "cout << state << endl;"
*/
ostream& operator<<(ostream &os, const State &state)
{
    for(int row=0; row<state.rows; row++)
    {
        for(int col=0; col<state.cols; col++)
        {
            if(state.grid[row][col].isWater)
                os << '%';
            else if(state.grid[row][col].isFood)
                os << '*';
            else if(state.grid[row][col].isHill)
                os << (char)('A' + state.grid[row][col].hillPlayer);
            else if(state.grid[row][col].ant >= 0)
                os << (char)('a' + state.grid[row][col].ant);
            else if(state.grid[row][col].isDeadlock == 1)
                os << 'x';
            else if(state.grid[row][col].isVisible)
            {
                if(state.grid[row][col].pheromone > 750)
                {
                    os << '!';
                }
                else if(state.grid[row][col].pheromone > 1)
                {
                    os << ' ';
                }
                else
                {
                    os << '.';
                }

            }
            else
                os << '?';
        }
        os << endl;
    }

    return os;
};

//input function
istream& operator>>(istream &is, State &state)
{
    int row, col, player;
    string inputType, junk;

    //finds out which turn it is
    while(is >> inputType)
    {
        if(inputType == "end")
        {
            state.gameover = 1;
            break;
        }
        else if(inputType == "turn")
        {
            is >> state.turn;
            break;
        }
        else //unknown line
            getline(is, junk);
    }

    if(state.turn == 0)
    {
        //reads game parameters
        while(is >> inputType)
        {
            if(inputType == "loadtime")
                is >> state.loadtime;
            else if(inputType == "turntime")
                is >> state.turntime;
            else if(inputType == "rows")
                is >> state.rows;
            else if(inputType == "cols")
                is >> state.cols;
            else if(inputType == "turns")
                is >> state.turns;
            else if(inputType == "viewradius2")
            {
                is >> state.viewradius;
                state.viewradius = sqrt(state.viewradius);
            }
            else if(inputType == "attackradius2")
            {
                is >> state.attackradius;
                state.attackradius = sqrt(state.attackradius);
            }
            else if(inputType == "spawnradius2")
            {
                is >> state.spawnradius;
                state.spawnradius = sqrt(state.spawnradius);
            }
            else if(inputType == "ready") //end of parameter input
            {
                state.timer.start();
                break;
            }
            else if(inputType == "player_seed"){
                is >> state.seed;
                break;
            }

            else    //unknown line
                getline(is, junk);
        }
    }
    else
    {
        //reads information about the current turn
        while(is >> inputType)
        {
            if(inputType == "w") //water square
            {
                is >> row >> col;
                state.grid[row][col].isWater = 1;
            }
            else if(inputType == "f") //food square
            {
                is >> row >> col;
                state.grid[row][col].isFood = 1;
                state.food.push_back(Location(row, col));
            }
            else if(inputType == "a") //live ant square
            {
                is >> row >> col >> player;
                state.grid[row][col].ant = player;
                if(player == 0)
                    state.myAnts.push_back(Location(row, col));
                else
                    state.enemyAnts.push_back(Location(row, col));
            }
            else if(inputType == "d") //dead ant square
            {
                is >> row >> col >> player;
                state.grid[row][col].deadAnts.push_back(player);
            }
            else if(inputType == "h")
            {
                is >> row >> col >> player;
                state.grid[row][col].isHill = 1;
                state.grid[row][col].hillPlayer = player;
                if(player == 0)
                    state.myHills.push_back(Location(row, col));
                else
                    state.enemyHills.push_back(Location(row, col));

            }
            else if(inputType == "players") //player information
                is >> state.noPlayers;
            else if(inputType == "scores") //score information
            {
                state.scores = vector<double>(state.noPlayers, 0.0);
                for(int p=0; p<state.noPlayers; p++)
                    is >> state.scores[p];
            }
            else if(inputType == "go") //end of turn input
            {
                if(state.gameover)
                    is.setstate(std::ios::failbit);
                else
                    state.timer.start();
                break;
            }
            else //unknown line
                getline(is, junk);
        }
    }

    return is;
};


Bug& operator<<(Bug &os, const Location &state) {
        os << "loc[" << state.row << ":" << state.col << "]";
        return os;
}

Bug& operator<<(Bug &os, const BattleArea &area) {
    os << "battle[";
    for(int i =0 ; i < area.myAnts.size(); i++)
    {
        os << area.myAnts[i] << ",";
    }
    os << "]";
    return os;
}

