#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>

int obstacles[4][5][2] = {{{0, 0}, {0, 1}, {0, 2}, {-1, -1}}, {{0, 0}, {0, 1}, {-1, -1}}, {{0, 0}, {1, 0}, {2, 0}, {-1, -1}}, {{0, 0}, {-1, -1}}};
int speed = 100;

typedef struct Player {
    int ypos;
    int xpos;
    int **points;
    int size;
    int numpoints;
    int nextmove;
    int dir;
    int jumpheight;
} Player;

enum ObsType {
    none = -1,
    tall = 0, 
    small = 1, 
    wide = 2, 
    tiny = 3, 
};

typedef struct Obstacle {
    enum ObsType obstype;
    int xpos;
    int nextmove;
} Obstacle;

typedef struct Dimensions {
    int x;
    int y;
} Dimension;

typedef struct Engine {
    Player player;
    Obstacle obstacles[3];
    Dimension dimensions;
    char** grid;
    int numobstacles;
    int movetime;
    int score;
} Engine;

typedef struct HashMap {
    int *arr[100];
    int len[100];
} HashMap;

HashMap newHashMap(){
    HashMap hm;
    for(int i = 0; i < 100; i++){
        hm.arr[i] = (int*) malloc(4 * 100);
        hm.len[i] = 0;
    }
    return hm;
}

void freehm(HashMap* hm){
    for(int i = 0; i < 100; i++){
        free((*hm).arr[i]);
    }
}

int hmcheck(HashMap hm, int num){
    int new = num % 100;
    for(int i = 0; i < (hm).len[new]; i++){
        if ((hm).arr[new][i] == num){
            return 1;
        }
    }
    return 0;
}

void hmadd(HashMap* hm, int num){
    int new = num % 100;
    if (hmcheck(*hm, new) == 0){
        int len = (*hm).len[new];
        (*hm).arr[new][len] = num;
        (*hm).len[new] ++;
    }
}

Player newPlayer(int points[5][2], int size){
    Player player;
    player.ypos = 0;
    player.xpos = 4;
    player.points = (int**) malloc(size * 4);
    player.numpoints = size;
    player.nextmove = -1;
    player.dir = 0;
    player.jumpheight = 5;
    for(int i = 0; i < size; i++){
        player.points[i] = (int*) malloc(2 * 4);
        player.points[i][0] = points[i][0];
        player.points[i][1] = points[i][1];
    }
    player.size = size;
    return player;
}

char** newgrid(int x, int y){
    int num = x * y;
    char **board = (char**) malloc(num);
    for (int i = 0; i < y; i++){
        board[i] = (char*) malloc(x);
    }
    for(int i = 0; i < y; i++){
        for (int j = 0; j < x; j++){
            board[i][j] = ' ';
        }
    }
    for(int i = 0; i < x; i++){
        board[0][i] = '#';
        board[y - 1][i] = '#';
    }
    for (int i = 0; i < y; i++){
        board[i][0] = '#';
        board[i][x - 1] = '#';
    }
    return board;
}

Dimension newdim(int x, int y){
    Dimension new;
    new.x = x;
    new.y = y;
    return new;
}

Obstacle newObstacle(int type, int xdim, int nextmove){
    Obstacle obs;
    switch (type){
        case -1: 
            obs.obstype = none;
            break;
        case 0:
            obs.obstype = tall;
            break;
        case 1:
            obs.obstype = small;
            break;
        case 2:
            obs.obstype = wide;
            break;
        case 3:
            obs.obstype = tiny;
            break;
    }
    obs.xpos = xdim;
    obs.nextmove = nextmove;
    return obs;
}

int lastObstacle(Engine engine){
    int ind = engine.numobstacles;
    int max = 0;
    for(int i = 0; i < 3; i++){
        if (engine.obstacles[i].obstype != -1){
            if (engine.obstacles[i].xpos > max){
                max = engine.obstacles[i].xpos;
            }
        }
    }
    return engine.dimensions.x - 2 - max;
}

void createObstacle(Engine *engine){
    int x = engine->dimensions.x - 3;
    int type = rand() % 4;
    int time = clock();
    for(int i = 0; i < 3; i++){
        if (engine->obstacles[i].obstype == -1){
            (*engine).obstacles[i] = newObstacle(type, x, clock() + engine->movetime);
            break;
        }
    }
    (*engine).numobstacles ++;
    if (engine->movetime >= speed / 2){
        (*engine).movetime = engine->movetime * 97 / 100;
    }
}

Engine newEngine(int x, int y){
    Engine engine;
    x += 2;
    y += 2;
    int arr[8][2] = {{0, 0}, {3, 2}, {1, 1}, {2, 1}, {2, 2}, {-1, 2}, {2, 0}, {0, 1}};
    int size = 8;
    engine.player = newPlayer(arr, size);
    engine.dimensions = newdim(x, y);
    engine.grid = newgrid(x, y);
    engine.numobstacles = 0;
    engine.movetime = speed;
    engine.score = 0;
    for(int i = 0; i < 3; i++){
        engine.obstacles[i] = newObstacle(-1, -1, 0);
    }
    createObstacle(&engine);
    return engine;
}

int checkEnd(Engine engine){
    int x = engine.dimensions.x;
    int y = engine.dimensions.y;
    HashMap hm = newHashMap();
    for(int i = 0; i < engine.player.size; i++){ 
        int y2 = engine.player.ypos + engine.player.points[i][1];
        int x2 = engine.player.xpos + engine.player.points[i][0];
        int n = x * y2 + x2;
        hmadd(&hm, n);
        if (y2 - 1 >= 0){
            hmadd(&hm, x * (y2 - 1) + x2);
        }
        hmadd(&hm, x * y2 + (x2 + 1));
    }  
    for (int i = 0; i < 3; i++){
        if ((engine).obstacles[i].obstype == none){
            continue;
        }
        int j = 0;
        while (obstacles[(engine).obstacles[i].obstype][j][0] != -1){
            int tx = obstacles[(engine).obstacles[i].obstype][j][0];
            int ty = obstacles[(engine).obstacles[i].obstype][j][1];
            int x2 = (engine).obstacles[i].xpos + tx;
            int y2 = ty;
            int n = (x * y2 + x2);
            if (hmcheck(hm, n) == 1){
                freehm(&hm);
                return 1;
            }
            j++;
        }
    }
    freehm(&hm);
    return 0;
}

int nextmove(Player* player){
    if (player->ypos == 0 && player->dir == -1){
        (*player).dir = 0;
        return 0;
    }
    if (player->ypos >= player->jumpheight){
        (*player).dir = -1;
        return -125;
    }
    if (player->dir == 1){
        return 125;
    }
    return -125;
}

void updateEngine(Engine* engine){
    int time = clock();
    int pmove = engine->player.nextmove;
    if (pmove != -1){
        if (time > pmove){
            int t = nextmove(&((*engine).player));
            if (t > 0){
                (*engine).player.ypos ++;
            }
            else if (t < 0 && engine->player.ypos > 0){
                (*engine).player.ypos --;
            }
            (*engine).player.nextmove = time + abs(t);
        }
    }
    for(int i = 0; i < 3; i++){
        if (engine->obstacles[i].obstype != -1){
            if (engine->obstacles[i].xpos == 0){
                (*engine).obstacles[i].obstype = -1;
                (*engine).numobstacles --;
                (*engine).score ++;
            }
            int move = engine->obstacles[i].nextmove;
            if (time > move){
                (*engine).obstacles[i].xpos--;
                (*engine).obstacles[i].nextmove = move + engine->movetime;
            }
        }
    }
    int r = rand() % 10;
    if (engine->numobstacles < 3 && lastObstacle(*engine) > 30 && r == 0){
        createObstacle(engine);
    }
}

void jump(Engine *engine){
    if (engine->player.ypos != 0){
        return;
    }
    (*engine).player.nextmove = clock() + 10;
    (*engine).player.dir = 1;
}

void matchgrid(Engine* engine){
    int x = engine->dimensions.x;
    int y = engine->dimensions.y;
    int playery = engine->player.ypos;
    for(int i = 1; i < y - 1; i++){
        for (int j = 1; j < x - 1; j++){
            (*engine).grid[i][j] = ' ';
        }
    }
    for (int i = 0; i < engine->player.numpoints; i++){
        (*engine).grid[y - 2 - playery - engine->player.points[i][1]][engine->player.xpos + engine->player.points[i][0] + 1] = 'O';
    }
    for (int i = 0; i < 3; i++){
        if (engine->obstacles[i].obstype == none){
            continue;
        }
        int j = 0;
        while (obstacles[engine->obstacles[i].obstype][j][0] != -1){
            int tx = obstacles[engine->obstacles[i].obstype][j][0];
            int ty = obstacles[engine->obstacles[i].obstype][j][1];
            int newy = y - 2 - ty;
            int newx = engine->obstacles[i].xpos + tx + 1;
            if (newx < engine->dimensions.x - 1){
                (*engine).grid[y - 2 - ty][engine->obstacles[i].xpos + tx + 1] = 'X';
            }
            j++;
        }
    }
}

void printscreen(Engine engine){
    for (int i = 0; i < engine.dimensions.y; i++){
        for(int j = 0; j < engine.dimensions.x; j++){
            printf("%c", engine.grid[i][j]);
        }
        printf("\n");
    }
    printf("Score: %i\n", engine.score);
}

int main(){
    srand(time(NULL));
    printf("\e[?25l");
    int cont = 0;
    while(1){
        Engine engine = newEngine(60, 12);
        int lasttick = clock();
        char ch;
        while (1){
            if (kbhit()){
                ch = getch();
                if(ch == ' '){
                    jump(&engine);
                }
            }
            int time = clock();
            updateEngine(&engine);
            if (time > lasttick + 150){
                lasttick = time;
                matchgrid(&engine);
                printf("\e[1;1H\e[2J");
                printscreen(engine);
                if(checkEnd(engine)){
                    break;
                }
            }
        }
        printf("Press a to restart, press q to quit");
        while (1){
            if (kbhit()){
                ch = getch();
                if(ch == 'a'){
                    cont = 1;
                    break;
                }
                else if (ch == 'q'){
                    cont = 0;
                    break;
                }
            }
        }
        if (cont == 0){
            break;
        }
    }
    
}