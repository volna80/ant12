#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <algorithm>
#include "Bot.h"


using namespace std;

//constructor
Bot::Bot()
{

};

//plays a single game of Ants.
void Bot::playGame()
{

    //reads the game parameters and sets up
    cin >> state;
    state.setup();
    endTurn();

    srand( state.seed);

    state.bug << "attackradius=" << state.attackradius << endl;
    state.bug << "viewradius=" << state.viewradius << endl;
    state.bug << "spawnradius=" << state.spawnradius << endl;


    //continues making moves while the game is not over
    while(cin >> state)
    {
        state.updateVisionInformation();

        state.bug << "turn " << state.turn << ":" << endl;
        state.bug << state << endl;

        updatePheromone();
        makeMoves();
        endTurn();
    }
};

void addFoodPheromone(Location const &loc, State &state)
{
    state.grid[loc.row][loc.col].pheromone = FOOD_PHEROMONE;
}

void addHillPheromone(Location const &loc, State &state)
{
    state.grid[loc.row][loc.col].pheromone = HILL_PHEROMONE;
}



void Bot::updatePheromone()
{
    //if(state.turn % 25 == 0)
    {
        //init first paths
        for(vector<Location>::iterator food = state.food.begin(); food != state.food.end(); ++food)
        {
            //state.bug << "look a path for " << *food << endl;
            findPath(*food, state.myHills, addFoodPheromone);
        }

        for(vector<Location>::iterator hill = state.enemyHills.begin(); hill != state.enemyHills.end(); ++hill)
        {
            //state.bug << "look a path for " << *hill << endl;
            findPath(*hill, state.myHills, addHillPheromone);
        }
    }

    //decrease pheromone
    for(int row=0; row<state.rows; row++)
        for(int col=0; col<state.cols; col++)
        {
            if(state.grid[row][col].pheromone != 1)
            {
                state.grid[row][col].pheromone -= 25;
            }
            if(state.grid[row][col].pheromone < 1)
            {
                state.grid[row][col].pheromone=1;
            }

        }


}


vector<BattleArea> Bot::findBattles()
{
    //list of ants which aleady assign to any battle
    vector<Location> inBattle;
    vector<BattleArea> battles;

    //define fighting groups\\
    //vector<Location> copyAnts = vector<Location>(state.myAnts);
    //vector<Location> copyAnts;
    vector<Location> copyAnts(state.myAnts);

    for(int ant=0; ant < (int) copyAnts.size(); ant++)
    {
        Location loc = copyAnts[ant];


        //check that this ant isn't in any battle
        if(inBattle.size() > 0)
        {
            vector<Location>::iterator it2 = std::find(inBattle.begin(), inBattle.end(), loc);
            if(*it2 == loc)
            {
                //next
                continue;
            }
        }

        vector<Location> myAnts;
        vector<Location> enemies;
        bool hasEnemy = false;

        //check the square
        for(int r = -5; r <= 5; r++)
        {
            if(myAnts.size() == MAX_BATTLE_GROUP)
            {
                break;
            }
            for(int c = -5; c <= 5; c++)
            {
                if(r == 0 && c ==0)
                {
                    continue;
                }
                int n_row = (r + loc.row + state.rows) % state.rows;
                int n_col = (c + loc.col + state.cols) % state.cols;

                if(state.grid[n_row][n_col].ant > 0 )
                {
                    hasEnemy = true;
                    enemies.push_back(Location(n_row, n_col));
                }
                else
                {
                    myAnts.push_back(Location(n_row,n_col));
                }

                if(myAnts.size() == MAX_BATTLE_GROUP)
                {
                    //it is enough for calculation
                    break;
                }

            }
        }

        if(hasEnemy)
        {
            //create battle
            BattleArea area;
            //add you-self
            myAnts.push_back(loc);

            //check that ants not in another battle
            for(int i=0; i < (int) myAnts.size(); i++)
            {
                Location loc2 = myAnts[i];
                //if this ant in the state.myAnts. So, it hasn't been assigned to any battle
                if(state.myAnts.size() > 0)
                {
                    vector<Location>::iterator it = find(state.myAnts.begin(), state.myAnts.end(), loc2);
                    if(* it == loc2)
                    {
                        //add ants to the battle
                        area.myAnts.push_back(loc2);
                        //remove ants from state.myAnts
                        state.myAnts.erase(it);
                        //remember that this ant already is assigned to a battle
                        inBattle.push_back(loc2);
                    }
                }

            }

            state.bug << "created a battle " << area << endl;
            battles.push_back(area);

        }
    }
    return battles;
}


void Bot::makeMoves(BattleArea area)
{
    state.bug << "move the battle[" << area << endl;
    // -1 - stay on the same place
    vector<int> steps = vector<int>(area.myAnts.size(), DONT_MOVE);
    vector<int> best_step = steps;
    int max_w = -100; //initial
    bool lastTurn = false;
    int combination = 0;
    while( !lastTurn )
    {
        combination ++;
        //state.bug << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << endl;
        //state.bug << "turn " << combination << endl;
        //calculate current step
        int cur_w = calculate_battle_result(area, steps);
        //state.bug << "turn result " << cur_w << endl;

        if(cur_w > max_w)
        {
            max_w = cur_w;
            best_step = steps; //make a copy
        }
        //increment steps;
        for(int j = 0; j < steps.size(); j++)
        {
            steps[j] = steps[j] + 1;

            if(steps[j] > 3) //ok, go to next ant
            {
                if(j+1 == steps.size())
                {
                    //the end
                    lastTurn = true;
                    break;
                }
                else
                {
                    //return the init value and will increment next move;
                    steps[j] = DONT_MOVE;
                    continue;
                }
            }
            else
            {
                break;
            }
        }
    }

    state.bug << "the result: " << max_w << endl;

    //make moves;
    for(int ant = 0; ant < area.myAnts.size(); ant++)
    {
        Location & loc = area.myAnts[ant];
        int direction = best_step[ant];
        if(direction == -1)
        {
            //do noting
            continue;
        }
        state.bug << "ant[" << loc << "] move to [" << CDIRECTIONS[direction] << "]" << endl;
        state.makeMove(loc, direction);
    }
}

//makes the bots moves for the turn
void Bot::makeMoves()
{

    vector<BattleArea> battles = findBattles();

    //make moves of fighting groups first
    for(int i = 0 ; i < battles.size(); i++)
    {
        BattleArea & area = battles[i];
        makeMoves(area);
    }


    //picks out moves for rest ants
    for(int ant=0; ant<(int)state.myAnts.size(); ant++)
    {
        state.bug << "start turn ant[" << ant << "]" << endl;

        //check that the and isn't in a battle

        double w[TDIRECTIONS] = {0,0,0,0}; //weights of every path
        double sumW = 0;

        for(int d=0; d<TDIRECTIONS; d++)
        {
            Location loc = state.getLocation(state.myAnts[ant], d);
            Square * square = &state.grid[loc.row][loc.col];

            if(square->isWater || square->ant == 0 || (square->isHill && square->hillPlayer ==0) || square->isDeadlock == 1)
            {
                //state.bug << "couldn't move to " << d << endl;
                //state.bug << "loc:" << loc.row << ":" << loc.col << "; w:" << square.isWater << "; h:" << square.isHill << "; a:" << square.ant << endl;
                continue;
            }

            int phe = square->pheromone;
            int des = calcDesirability(state.myAnts[ant], d);
            w[d] = pow(phe, COMMINITY) * pow(des, GREEDY);
            state.bug << CDIRECTIONS[d] << " phe=" << phe << "; des=" << des << "; w=" << w[d] << endl;
            sumW += w[d];

            //    if(!state.grid[loc.row][loc.col].isWater)
            //    {
            //       state.makeMove(state.myAnts[ant], d);
            //        break;
            //    }
            //#1 calculate probability of every options

        }

        // state.bug << "sumW=" << sumW << endl;


        if(sumW == 0)
        {
            continue;
        }

        //make a decision

        if(state.turn % 5 == 0)
        {
            //random aproach
            double p[TDIRECTIONS] = {0,0,0,0}; //probability
            int r = rand() % 1000;

            //state.bug << "r:" << r << "; ";
            double aggr = 0;

            for(int d=0; d < TDIRECTIONS ; d++)
            {
                aggr += (w[d] / sumW);
                //state.bug << "p" << d << "=" << (1000 * aggr) << "; ";
                if(aggr * 1000 > ( r - 1))
                {
                    state.bug << "move to [" << CDIRECTIONS[d] << "]" << endl;
                    state.makeMove(state.myAnts[ant], d);
                    break;
                }
            }

        }
        else
        {
            //look for max
            int mov = -1;
            double maxP = 0;
            for(int i = 0; i < TDIRECTIONS; i++)
            {
                if(w[i] > 0 && w[i] > maxP)
                {
                    mov = i;
                    maxP = w[i];
                }
            }

            if(mov != -1)
            {
                state.bug << "move to [" << CDIRECTIONS[mov] << "]" << endl;
                state.makeMove(state.myAnts[ant], mov);
            }
            else
            {
                state.bug << "stay on the same place " << endl;
            }
        }

        //make decision


    }

    state.bug << "time taken: " << state.timer.getTime() << "ms" << endl << endl;
};

int Bot::calculate_battle_result(const BattleArea &area, const vector<int> &steps)
{
    //state.bug << "calculate_battle_result() area=" << area << endl;
    std::vector<std::vector<Square> > grid = state.grid; //make a copy

    //make moves
    for(int ant=0; ant < area.myAnts.size(); ant++)
    {
        const Location & loc = area.myAnts[ant];
        int direction = steps[ant];
        //state.bug << "ant[" << loc << "]; d=" << direction << endl;
        if(direction == DONT_MOVE)
        {
            //do nothing
        }
        else
        {
            Location nLoc = state.getLocation(loc, direction);
            Square & square = grid[nLoc.row][nLoc.col];
            if(square.isWater || square.ant == 0 || square.isDeadlock == 1)
            {
                //state.bug << "impossible step";
                return -10000000; //impossible step

            }
            grid[nLoc.row][nLoc.col].ant = grid[loc.row][loc.col].ant;
            grid[loc.row][loc.col].ant = -1;
        }
    }

    //calculate the battle result
    int numMyDeadAnts = getNumberOfDeadAnts(area.myAnts, steps, grid);
    int numEnemyDeadAnts = getNumberOfDeadAnts(area.enemy, steps, grid);
    //state.bug << "start calculating the battle result" << endl;



    return numEnemyDeadAnts - 2 * numMyDeadAnts; //So, I think 2 dead enemies vs 1 dead mine is a good result
}

int Bot::getNumberOfDeadAnts(vector<Location> myAnts, const vector<int> &steps, std::vector<std::vector<Square> > grid)
{
    int numDeadAnts = 0;
    for(int ant=0; ant < myAnts.size(); ant++)
    {
        const Location &loc = myAnts[ant]; //TODO copy-past
        int direction = steps[ant];
        Location nLoc = state.getLocation(loc, direction);

        // how to check if an ant dies
        //for every ant:
        //    for each enemy in range of ant (using attackadius2):
        //        if (enemies(of ant) in range of ant) >= (enemies(of enemy) in range of enemy) then
        //            the ant is marked dead (actual removal is done after all battles are resolved)
        //            break out of enemy loop
        //state.bug << "checking if an ant dies; ant[" << nLoc << endl;
        vector<Location> enemies_v = enemies(nLoc, grid, grid[nLoc.row][nLoc.col].ant); //list of enemies for the ant
        for(int enemy = 0; enemy < enemies_v.size(); enemy++)
        {
            Location & loc_enemy = enemies_v[enemy];
            if(enemies_v.size() >= enemies(loc_enemy, grid, grid[loc_enemy.row][loc_enemy.col].ant).size())
            {
                //we dead
                numDeadAnts ++;
                break;
            }

        }
    }
    return numDeadAnts;

}

//return enemies in range of ant
vector<Location> Bot::enemies(const Location &ant, const std::vector<std::vector<Square> > grid, int owner)
{
    vector<Location> e;
    for(int r = -state.attackradius - 1; r < state.attackradius + 1; r++)
    {
        for(int c = -state.attackradius - 1; c < state.attackradius + 1; c++)
        {
            int enemy_row = (r + ant.row + state.rows) % state.rows;
            int enemy_col = (c + ant.col + state.cols) % state.cols;
            Location enemy(enemy_row, enemy_col);
            if(
                grid[enemy_row][enemy_col].ant != -1 &&
                grid[enemy_row][enemy_col].ant != owner &&
                state.distance(ant, enemy) <= (state.attackradius + 1))
            {
                //ok, it is ant, and it isn't our ant, and it in the attack radius
                e.push_back(enemy);
            }
        }
    }
    return e;
}

int Bot::calcDesirability(const Location &current, int direction)
{


    int d = 1;

    //go on in the same direction is more prefereable
    if(state.lastTurn[current.row][current.col] == direction)
    {
        d += D_SAME_DIRECTION;
        state.bug << "+dir[" << D_SAME_DIRECTION << "]; ";
    }
    else if(state.lastTurn[current.row][current.col] == (direction + 2) % TDIRECTIONS) //180%
    {
        //nothing
    }
    else
    {
        //90%
        d += D_SAME_DIRECTION / 4;
        state.bug << "+dir[" << (D_SAME_DIRECTION / 4 ) << "]; ";
    }

    for(int deep = 0; deep < 5 ; deep++)
    {
        Location ll = state.getLocation(current, direction, deep);

        if(state.grid[ll.row][ll.col].isWater)
        {
            break;
        }

        for(int j=-deep; j <= deep ; j++)
        {
            int r = current.row;
            int c = current.col;
            if(direction == NORTH)
            {
                r -= deep;
                c += j;
            }
            else if(direction == EAST)
            {
                r += j;
                c += deep;
            }
            else if(direction == SOUTH)
            {
                r += deep;
                c += j;
            }
            else     //direction WEST
            {
                r += j;
                c -= deep;
            }
            r = (r + state.rows) % state.rows;
            c = (c + state.cols) % state.cols;

            Location l = Location(r,c);

            Square * square = &state.grid[l.row][l.col];

//            state.bug << "check r:" <<  nLoc.row << ", c:" << nLoc.col << ", sq=" << square << endl;

            if(square->isFood)
            {
                int tmp;
                if(deep == 0)
                {
                    tmp = W_FOOD;
                }
                else
                {
                    tmp = W_FOOD/ deep;
                }
                d += tmp;
                state.bug << "+food[" << tmp << "]; ";
            }

            if(square->isHill && square->hillPlayer != 0)
            {
                int tmp;
                if(deep == 0)
                {
                    tmp = W_HILL;
                }
                else
                {
                    tmp = W_HILL / deep;
                }
                d += tmp;
                state.bug << "+hill[" << tmp << "]; ";
            }

            if(square->isLand == -1)
            {
                d += W_EXPLORATION;
                state.bug << "+expl[" << W_EXPLORATION << "]; ";
            }
        }
    }


    //new location
    //Location l = state.getLocation(current, direction);


    state.bug << endl;

    return d;
}

//finishes the turn
void Bot::endTurn()
{
    if(state.turn > 0)
    {
        state.reset();
    }
    state.turn++;

    cout << "go" << endl;
};
