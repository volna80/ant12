#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <algorithm>
#include "Bot.h"

using namespace std;

vector<BattleArea> Bot::findBattles()
{
    //list of ants which aleady assign to any battle
    ants_t inBattle;
    vector<BattleArea> battles;

    //define fighting groups

    ants_t copyAnts(state.myAnts);

    for(int ant=0; ant < (int) copyAnts.size(); ant++)
    {
        Location & loc = copyAnts[ant];


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

        ants_t myAnts;
        ants_t enemies;
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
                int n_row = (r + loc.row + state.rows) % state.rows;
                int n_col = (c + loc.col + state.cols) % state.cols;

                if(state.grid[n_row][n_col].ant > 0 )
                {
                    hasEnemy = true;
                    enemies.push_back(Location(n_row, n_col));
                }
                else if(state.grid[n_row][n_col].ant == 0)
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
                Location & loc2 = myAnts[i];
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


//move an ant from old location to new one and update a grid
//true - if a move is possible, false
bool Bot::updateGrid(Location &ant, int from, int to, grid_t & grid)
{
    Location old_loc = state.getLocation(ant, from);
    Location to_loc = state.getLocation(ant, to);

    //check if we can do a move

    Square & oldSqr = grid[old_loc.row][old_loc.col];

    //clean up the old square
    oldSqr.ant = -1;
    //remove the ant from vision of this ant
    for(int a = 0; a < (int) oldSqr.listOfEnemies.size(); a++)
    {
        Location &enemy = oldSqr.listOfEnemies[a];
        Square & enemySquare = grid[enemy.row][enemy.col];
        remove(enemySquare.listOfEnemies.begin(), enemySquare.listOfEnemies.end(), old_loc);
    }
    oldSqr.listOfEnemies.clear();

    Square & square = grid[to_loc.row][to_loc.col];
    //check if we can move to new square
    if(square.isWater || square.ant >= 0 || square.isDeadlock == 1 || square.isFood)
    {
        return false;
    }

    //swap ants
    square.ant =  0;


    state.enemies(to_loc, grid, square.ant);

    //add the ant to the vision of new enemies
    for(int a=0; a <(int) square.listOfEnemies.size(); a++)
    {
        Location &enemy = square.listOfEnemies[a];
        Square & enemySquare = grid[enemy.row][enemy.col];
        enemySquare.listOfEnemies.push_back(to_loc);
    }



    return true;

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
    grid_t grid = state.grid; //make a copy of the map
    while( !lastTurn )
    {
        combination ++;
        //state.bug << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << endl;
        //state.bug << "turn " << combination << endl;
        //calculate current step
        int cur_w = calculate_battle_result(area, steps, grid);
        //state.bug << "turn result " << cur_w << endl;

        if(cur_w > max_w)
        {
            max_w = cur_w;
            best_step = steps; //make a copy
        }
        else if(cur_w == max_w && (rand() % 10) == 7)
        {
            best_step = steps;
        }
        //increment steps;
        for(int j = 0; j < steps.size(); j++)
        {
            int oldD = steps[j];
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
                    updateGrid(area.myAnts[j], oldD, steps[j], grid); //DONT_MOVE should always be a correct move
                    continue;
                }
            }
            else
            {
                if(updateGrid(area.myAnts[j], oldD, steps[j], grid))
                {
                    break;
                }
                else
                {
                    //impossible move
                    continue;
                }
            }
        }
    }

    state.bug << "the result: " << max_w << endl;

    //make moves;
    for(int ant = 0; ant < area.myAnts.size(); ant++)
    {
        Location & loc = area.myAnts[ant];
        int direction = best_step[ant];
        if(direction == DONT_MOVE)
        {
            //do noting
            continue;
        }
        state.bug << "ant[" << loc << "] move to [" << CDIRECTIONS[direction] << "]" << endl;
        state.makeMove(loc, direction);
    }
}

int Bot::calculate_battle_result(const BattleArea &area, const vector<int> &steps, grid_t &grid)
{

    state.bug << "time taken (start calculate battle result): " << state.timer.getTime() << "ms" << endl << endl;

    //calculate the battle result
    int numMyDeadAnts = getNumberOfDeadAnts(area.myAnts, steps, grid);
    int numEnemyDeadAnts = getNumberOfDeadAnts(area.enemy, steps, grid);
    //state.bug << "start calculating the battle result" << endl;

    state.bug << "time taken (calculate dead ants): " << state.timer.getTime() << "ms" << endl << endl;



    return numEnemyDeadAnts - 2 * numMyDeadAnts; //So, I think 2 dead enemies vs 1 dead mine is a good result
}

int Bot::getNumberOfDeadAnts(const ants_t & myAnts, const vector<int> &steps, grid_t &grid)
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
        ants_t & enemies_v = grid[nLoc.row][nLoc.col].listOfEnemies; //list of enemies for the ant
        for(int enemy = 0; enemy < enemies_v.size(); enemy++)
        {
            Location & loc_enemy = enemies_v[enemy];

            ants_t & enemies_of_enemy = grid[loc_enemy.row][loc_enemy.col].listOfEnemies;

            if(enemies_v.size() >= enemies_of_enemy.size())
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
void State::enemies(const Location &ant, grid_t & grid_p, int owner)
{

    //if(grid_p[ant.row][ant.col].enemiesCached)
    //{
    //    return;
    //}


    for(int r = -attackradius - 1; r < attackradius + 1; r++)
    {
        for(int c = -attackradius - 1; c < attackradius + 1; c++)
        {
            int enemy_row = (r + ant.row + rows) % rows;
            int enemy_col = (c + ant.col + cols) % cols;
            Location enemy(enemy_row, enemy_col);
            if(
                grid_p[enemy_row][enemy_col].ant != -1 &&
                grid_p[enemy_row][enemy_col].ant != owner &&
                distance(ant, enemy) <= (attackradius + 1))
            {
                //ok, it is ant, and it isn't our ant, and it in the attack radius
                grid_p[ant.row][ant.col].listOfEnemies.push_back(enemy);
            }
        }
    }

    //grid_p[ant.row][ant.col].enemiesCached = true;
}
