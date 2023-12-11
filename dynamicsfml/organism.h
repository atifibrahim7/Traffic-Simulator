#include <SFML/Graphics.hpp>
#include <utility>
#include <cstdlib>
#include <iostream>
#include <vector>

using namespace std;
enum Object_type { EMPTY, GREEN_BLOB, RED_BLOB };
enum Direction { UP, RIGHT, DOWN, LEFT, NUMBER_OF_DIRECTIONS };

struct My_coord
{
    int x;
    int y;
};


const int BOARD_HEIGHT = 20;
const int BOARD_WIDTH = 20;

class World;
class Tile;

class Tile
{
public:
    Tile(int x, int y, World& world);
    virtual void display(sf::RenderWindow& window) {};
    virtual void turn() {};
    virtual Object_type who() { return EMPTY; }
    virtual void breeding() {

    }
protected:
    My_coord coord;
    World& world;
    int breedcounter = 0;
    int starvecounter = 0; 
};

class World
{
public:
    vector<Tile*>& operator[](int index);           // correctly implemented
    const vector<Tile*>& operator[](int index) const;       //just returns
    World();        // creates the world with empty tiles and then makes  red and green blobs at random positions
    void simulate_a_turn();
    void display_world(sf::RenderWindow& window);
    void run_simulation();

private:
    vector<vector<Tile*>> world;
};

class Green_blob : public Tile
{
public:
    Green_blob(int x, int y, World& world);
    //move
    virtual void display(sf::RenderWindow& window);
    virtual void turn();
    void teleport();
    virtual Object_type who() { return GREEN_BLOB; }
    //who
    //breed
    //turn
    virtual void breeding()
    {
        if (breedcounter >= 3)
        {
            switch (rand() % NUMBER_OF_DIRECTIONS)
            {
            case RIGHT : 
                if ((coord.x +1 <20)  &&  world[coord.y][coord.x + 1]->who() == EMPTY  )
                {
                    delete world[coord.y][coord.x + 1];
                    world[coord.y][coord.x + 1] = new Green_blob(coord.x + 1, coord.y, world);
                    breedcounter = 0;
                    break;
                }
            case LEFT: 
                if (coord.x - 1 > 0 &&  world[coord.y][coord.x-1]->who() == EMPTY)
                {
                    delete world[coord.y][coord.x -1];
                    world[coord.y][coord.x -1] = new Green_blob(coord.x -1, coord.y, world);
                    breedcounter = 0;

                    break;
                }
#
            case UP : 
                if ( coord.y - 1 > 0 && world[coord.y-1][coord.x]->who() == EMPTY)
                {
                    delete world[coord.y-1][coord.x];
                    world[coord.y-1][coord.x ] = new Green_blob(coord.x , coord.y - 1, world);
                    breedcounter = 0;

                    break;
                }
            case DOWN: 
                if ( coord.y+1 <20 &&  world[coord.y+1][coord.x ]->who() == EMPTY)
                {
                    delete world[coord.y+1][coord.x];
                    world[coord.y+1][coord.x ] = new Green_blob(coord.x , coord.y +1, world);
                    breedcounter = 0;
                    break;
                }

            }
        }
    }
private:
    int turns;

    //radius
};

class Empty_tile : public Tile
{
public:
    virtual void display(sf::RenderWindow& window);
    virtual void turn();
private:
};

class Red_blob : public Tile
{
public:
    Red_blob(int x, int y, World& world);
    //move
    virtual void display(sf::RenderWindow& window);
    virtual void turn();
    virtual Object_type who() { return RED_BLOB; }
    virtual void breeding()
    {
       
          if (breedcounter >= 8)
        {
            switch (rand() % NUMBER_OF_DIRECTIONS)
            {
            case RIGHT:
                if ((coord.x + 1 < 20) && world[coord.y][coord.x + 1]->who() == EMPTY)
                {
                    delete world[coord.y][coord.x + 1];
                    world[coord.y][coord.x + 1] = new Red_blob(coord.x + 1, coord.y, world);
                    breedcounter = 0;
                    break;
                }
            case LEFT:
                if (coord.x - 1 > 0 && world[coord.y][coord.x - 1]->who() == EMPTY)
                {
                    delete world[coord.y][coord.x - 1];
                    world[coord.y][coord.x - 1] = new Red_blob(coord.x - 1, coord.y, world);
                    breedcounter = 0;

                    break;
                }
#
            case UP:
                if (coord.y - 1 > 0 && world[coord.y - 1][coord.x]->who() == EMPTY)
                {
                    delete world[coord.y - 1][coord.x];
                    world[coord.y - 1][coord.x] = new Red_blob(coord.x, coord.y - 1, world);
                    breedcounter = 0;

                    break;
                }
            case DOWN:
                if (coord.y + 1 < 20 && world[coord.y + 1][coord.x]->who() == EMPTY)
                {
                    delete world[coord.y + 1][coord.x];
                    world[coord.y + 1][coord.x] = new Red_blob(coord.x, coord.y + 1, world);
                    breedcounter = 0;
                    break;
                }

            }
        }
    }

private:
    int turns = 0;          // counter for 3  step cycle

        
};


Green_blob::Green_blob(int x, int y, World& world)
    :Tile(x, y, world)
{
    turns = 0;
}

void Green_blob::display(sf::RenderWindow& window)
{   
    breedcounter++;
    starvecounter++;
    breeding();
    sf::CircleShape shape(9.f);
    shape.setFillColor(sf::Color::Green);
    sf::Vector2f display_position;

    display_position.x = coord.x * 20 + 10;
    display_position.y = coord.y * 20 + 10;
    shape.setPosition(display_position);
    window.draw(shape);
    turns++; 
   // cout << "Turns : " << turns << endl;
}

void Green_blob::teleport()
{
    coord.x = rand() % BOARD_WIDTH;
    coord.y = rand() % BOARD_HEIGHT;
}

World::World()
{
    world.resize(BOARD_HEIGHT);
    //for loop to resize all internal vectors
    for (int i = 0; i < BOARD_HEIGHT; i++)
    {
        world[i].resize(BOARD_WIDTH);
    }

    for (int y = 0; y < BOARD_HEIGHT; y++)
    {
        for (int x = 0; x < BOARD_WIDTH; x++)
        {
            world[y][x] = new Tile(x, y, *this);
        }
    }

    int green_blobs_ready = 100;
    // creates green blobs for work
    while (green_blobs_ready > 0)
    {
        int randX;
        int randY;
        randX = rand() % BOARD_WIDTH;
        randY = rand() % BOARD_HEIGHT;
        if (world[randY][randX]->who() == EMPTY)
        {
            delete world[randY][randX];
            world[randY][randX] = new Green_blob(randX, randY, *this);
            green_blobs_ready--;
        }
    }

    int red_blobs_ready = 5;
    //creating red blobs
    while (red_blobs_ready > 0)
    {
        int randX;
        int randY;
        randX = rand() % BOARD_WIDTH;
        randY = rand() % BOARD_HEIGHT;
        if (world[randY][randX]->who() == EMPTY)
        {
            delete world[randY][randX];
            world[randY][randX] = new Red_blob(randX, randY, *this);
            red_blobs_ready--;
        }
    }

}


void World::display_world(sf::RenderWindow& window)
{
    for (int y = 0; y < BOARD_HEIGHT; y++)
    {
        for (int x = 0; x < BOARD_WIDTH; x++)
        {
            world[y][x]->display(window);
        }
    }
}
void World::run_simulation()
{

}


// is a virtual fuc
void Empty_tile::display(sf::RenderWindow& window)
{

}
//no need to turn empty tile
void Empty_tile::turn() {
}

void Green_blob::turn() {
    //try to move?
    //choose a direction a random

    switch (rand() % NUMBER_OF_DIRECTIONS)
    {
    case UP:
        if (coord.y > 0)
        {
            if (world[coord.y - 1][coord.x]->who() == EMPTY)
            {
                swap(world[coord.y - 1][coord.x], world[coord.y][coord.x]);
                coord.y--;
            }
        }
        break;
    case RIGHT:
        if (coord.x < BOARD_WIDTH - 1)
        {
            if (world[coord.y][coord.x + 1]->who() == EMPTY)
            {
                swap(world[coord.y][coord.x + 1], world[coord.y][coord.x]);
                coord.x++;
            }

        }
        break;
    case DOWN:
        if (coord.y < BOARD_HEIGHT - 1)
        {
            if (world[coord.y + 1][coord.x]->who() == EMPTY)
            {
                swap(world[coord.y + 1][coord.x], world[coord.y][coord.x]);
                coord.y++;
            }
        }
        break;
    case LEFT:
        if (coord.x > 0)
        {
            if (world[coord.y][coord.x - 1]->who() == EMPTY)
            {
                swap(world[coord.y][coord.x - 1], world[coord.y][coord.x]);
                coord.x--;
            }
        }
        break;
    }
    //find out if that direction is available?
    //move there (means updating the blob, updating the world)
}

void World::simulate_a_turn()
{           
    
    
    vector<Tile*> red_blobs;
    for (int y = 0; y < BOARD_HEIGHT; y++)
    {
        for (int x = 0; x < BOARD_WIDTH; x++)
        {
            if (world[y][x]->who() == RED_BLOB)
            {
                red_blobs.push_back(world[y][x]);
            }
        }
    }
    if (red_blobs.size() == 0)
    {
        cout << "Predetor wiped ";
        exit(10);
    }
    for (int i = 0; i < 100000; i++)
    {
        int index1 = rand() % red_blobs.size();
        int index2 = rand() % red_blobs.size();
        swap(red_blobs[index1], red_blobs[index2]);
    }

    for (int i = 0; i < red_blobs.size(); i++)
    {
        red_blobs[i]->turn();
    }
    vector<Tile*> green_blobs;
    for (int y = 0; y < BOARD_HEIGHT; y++)
    {
        for (int x = 0; x < BOARD_WIDTH; x++)
        {
            if (world[y][x]->who() == GREEN_BLOB)
            {
                green_blobs.push_back(world[y][x]);
            }
        }
    }
    for (int i = 0; i < 100000; i++)
    {

        if (!green_blobs.empty())
        {



            int index1 = rand() % green_blobs.size();
            int index2 = rand() % green_blobs.size();
            swap(green_blobs[index1], green_blobs[index2]);
        }
    }

    for (int i = 0; i < green_blobs.size(); i++)
    {
        green_blobs[i]->turn();
    }

   if (green_blobs.size() == 0 || red_blobs.size() == 0)
    {   
       cout << "Predetor / Prey Wiped";
        exit(11);

    }
}

vector<Tile*>& World::operator[](int index)
{
    if (index >= BOARD_HEIGHT || index < 0)
    {
        cout << "Out of bounds access on world vector" << endl;
        exit(1);
    }
    return world[index];
}
const vector<Tile*>& World::operator[](int index) const
{
    return world[index];
}

Tile::Tile(int x, int y, World& world)
    :world(world)
{
    coord.x = x;
    coord.y = y;
}

Red_blob::Red_blob(int x, int y, World& world)
    :Tile(x, y, world)
{

}
//move
void Red_blob::display(sf::RenderWindow& window)
{
   // cout << starvecounter<<endl;
    breedcounter++;
    starvecounter++;
    breeding();
    sf::CircleShape shape(9.f);
    shape.setFillColor(sf::Color::Red);
    sf::Vector2f display_position;

    display_position.x = coord.x * 20 + 10;
    display_position.y = coord.y * 20 + 10;
    shape.setPosition(display_position);
    window.draw(shape);
}
void Red_blob::turn()
{
    Direction D;
    bool flag = false;
    switch (rand() % NUMBER_OF_DIRECTIONS)
    {
    case UP :
         if (coord.y - 1 >= 0 && world[coord.y - 1][coord.x]->who() == GREEN_BLOB) { flag = 1; D = Direction::UP; }
        else if (coord.x + 1 < 20 && world[coord.y][coord.x + 1]->who() == GREEN_BLOB) { flag = 1; D = Direction::RIGHT; }
        else if (coord.y + 1 < 20 && world[coord.y + 1][coord.x]->who() == GREEN_BLOB) { flag = 1; D = Direction::DOWN; }
        else if (coord.x - 1 >= 0 && world[coord.y][coord.x - 1]->who() == GREEN_BLOB) { flag = 1; D = Direction::LEFT; }
        else { flag = 0; }
         break;
    case DOWN:
        if (coord.y + 1 < 20 && world[coord.y + 1][coord.x]->who() == GREEN_BLOB) { flag = 1; D = Direction::DOWN; }
        else if (coord.x - 1 >= 0 && world[coord.y][coord.x - 1]->who() == GREEN_BLOB) { flag = 1; D = Direction::LEFT; }
        else if (coord.x + 1 < 20 && world[coord.y][coord.x + 1]->who() == GREEN_BLOB) { flag = 1; D = Direction::RIGHT; }
        else if (coord.y - 1 >= 0 && world[coord.y - 1][coord.x]->who() == GREEN_BLOB) { flag = 1; D = Direction::UP; }
        else { flag = 0; }
        break; 
    case RIGHT : 
         if (coord.x + 1 < 20 && world[coord.y][coord.x + 1]->who() == GREEN_BLOB) { flag = 1; D = Direction::RIGHT; }
        else if (coord.y - 1 >= 0 && world[coord.y - 1][coord.x]->who() == GREEN_BLOB) { flag = 1; D = Direction::UP; }
        else if (coord.y + 1 < 20 && world[coord.y + 1][coord.x]->who() == GREEN_BLOB) { flag = 1; D = Direction::DOWN; }
        else if (coord.x - 1 >= 0 && world[coord.y][coord.x - 1]->who() == GREEN_BLOB) { flag = 1; D = Direction::LEFT; }
        else { flag = 0; }
         break;
    case LEFT : 
        if (coord.x - 1 >= 0 && world[coord.y][coord.x - 1]->who() == GREEN_BLOB) { flag = 1; D = Direction::LEFT; }
        else if (coord.x + 1 < 20 && world[coord.y][coord.x + 1]->who() == GREEN_BLOB) { flag = 1; D = Direction::RIGHT; }
        else if (coord.y - 1 >= 0 && world[coord.y - 1][coord.x]->who() == GREEN_BLOB) { flag = 1; D = Direction::UP; }
        else if (coord.y + 1 < 20 && world[coord.y + 1][coord.x]->who() == GREEN_BLOB) { flag = 1; D = Direction::DOWN; }
        else { flag = 0; }
        break;
    }
    
    
   

        if (flag)
        {
            if (D == Direction::LEFT)
            {
                delete  world[coord.y][coord.x - 1];
                world[coord.y][coord.x - 1] = new Tile(coord.x-1, coord.y, world);
                swap(world[coord.y][coord.x - 1], world[coord.y][coord.x]);
                coord.x--;
                starvecounter = 0;
            }
            if (D == Direction::RIGHT)
            {
                delete  world[coord.y][coord.x + 1];
                world[coord.y][coord.x + 1] = new Tile(coord.x+1, coord.y, world);
                swap(world[coord.y][coord.x +1], world[coord.y][coord.x]);
                coord.x++;
                starvecounter = 0;

            }
            if (D == Direction::UP)
            {
                delete  world[coord.y-1][coord.x ];
                world[coord.y-1][coord.x ] = new Tile(coord.x, coord.y-1, world);
                swap(world[coord.y-1][coord.x ], world[coord.y][coord.x]);
                starvecounter = 0;
                coord.y--;
            }
            if (D == Direction::DOWN)
            {
                delete  world[coord.y + 1][coord.x];
                world[coord.y + 1][coord.x] = new Tile(coord.x, coord.y + 1, world);
                swap(world[coord.y + 1][coord.x], world[coord.y][coord.x]);
                coord.y++;
                starvecounter = 0;

            }
        }
        else
        {

        switch (rand() % NUMBER_OF_DIRECTIONS)
        {
        case UP:
            if (coord.y > 0)
            {
                if (world[coord.y - 1][coord.x]->who() == EMPTY)
                {
                    swap(world[coord.y - 1][coord.x], world[coord.y][coord.x]);
                    coord.y--;
                }
                else if (world[coord.y - 1][coord.x]->who() == GREEN_BLOB)
                {   
                
                   delete world[coord.y - 1][coord.x]; 
                   world[coord.y - 1][coord.x] = new Tile(coord.x, coord.y - 1, world);
                   swap(world[coord.y - 1][coord.x], world[coord.y][coord.x]);
                   coord.y--;
                   starvecounter = 0;

                }
            }
            break;
        case RIGHT:
            if (coord.x < BOARD_WIDTH - 1)
            {
                if (world[coord.y][coord.x + 1]->who() == EMPTY)
                {
                    swap(world[coord.y][coord.x + 1], world[coord.y][coord.x]);
                    coord.x++;
                }
                else if (world[coord.y][coord.x + 1]->who() == GREEN_BLOB)
                {
             
                    delete world[coord.y][coord.x + 1]; 
                    world[coord.y][coord.x + 1] = new Tile(coord.x+1, coord.y , world);
                    swap(world[coord.y ][coord.x+1], world[coord.y][coord.x]);
                    coord.x++;
                    starvecounter = 0;

                }
            }
            break;
        case DOWN:
            if (coord.y < BOARD_HEIGHT - 1)
            {
                if (world[coord.y + 1][coord.x]->who() == EMPTY)
                {
                    swap(world[coord.y + 1][coord.x], world[coord.y][coord.x]);
                    coord.y++;
                }
                else if (world[coord.y + 1][coord.x]->who() == GREEN_BLOB)
                {
                    delete world[coord.y + 1][coord.x];
                    world[coord.y + 1][coord.x] = new Tile(coord.x, coord.y + 1, world);
                    swap(world[coord.y+1][coord.x ], world[coord.y][coord.x]);
                    coord.y++;
                    starvecounter = 0;

                }
            }
            break;
        case LEFT:
            if (coord.x > 0)
            {
                if (world[coord.y][coord.x - 1]->who() == EMPTY)
                {
                    swap(world[coord.y][coord.x - 1], world[coord.y][coord.x]);
                    coord.x--;
                }
                else if (world[coord.y][coord.x - 1]->who() == GREEN_BLOB)
                {
                    delete world[coord.y][coord.x-1];
                    world[coord.y][coord.x-1] = new Tile(coord.x-1, coord.y , world);
                    swap(world[coord.y][coord.x-1], world[coord.y][coord.x]);
                    coord.x--;
                    starvecounter = 0;

                }
            }
            break;
        }
       }


       if (starvecounter > 3)
       {

             world[coord.y][coord.x] = new Tile(coord.x,coord.y, world);
       }
}
