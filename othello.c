int TREEDEPTH;
#define TRAINING 0


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "GL/glut.h"
#include "SOIL/SOIL.h"
#include "Board.h"
#include "Computer.h"
#include <windows.h>

#define PI 3.14159265
#define GL_CLAMP_TO_EDGE 0x812F
#define boardboundoff 10

typedef struct tagRect{
    float x;
    float y;
    float width;
    float height;
} Rect;



GLuint boardtex;
GLuint boardtex2;
GLuint whitetex;
GLuint blacktex;
Rect boardBounds;
Rect screenBounds;

GLuint newGame;
Rect newButton;

GLuint quit;
Rect quiButton;

GLuint toggletex;
Rect toggle;
short displayMoves;

Computer* compOpp;
short playerColor;
short compPlayer;
short activePlayer;
Board* b;


extern short otherPlayer(short player);

short contains(Rect* rect, int x, int y){
    if(x < rect->x ||
        x > rect->x + rect->width ||
        y < rect->y ||
        y > rect->y + rect->height){
            return 0;
        } else {
            return 1;
        }
}

void switchPlayer(){
    if(activePlayer== BLACK){
        activePlayer = WHITE;
    } else {
        activePlayer = BLACK;
    }
}
void renderScene();
Computer* train1;
Computer* train2;
void runTrainingGame(Computer* train1, Computer* train2 , int* ended){
    Turn* t;
            t = makeMove(train2, b);
            //SleepEx(1000, FALSE);
            flipCaptured(b, t, activePlayer);
            free(t);
            renderScene();
            switchPlayer();

            while(countMovePossible(b, activePlayer) == 0){

                switchPlayer();
                if(countMovePossible(b, activePlayer) == 0){
                    *ended = 1;
                    return;
                } else {
                    fillOpenMoves(b, activePlayer);
                    t = makeMove(train2, b);
                    SleepEx(1000, FALSE);
                    flipCaptured(b, t, activePlayer);
                    free(t);
                    renderScene();
                    switchPlayer();
                }
            }
            fillOpenMoves(b, activePlayer);

            t = makeMove(train1, b);
            //SleepEx(1000, FALSE);
            flipCaptured(b, t, activePlayer);
            free(t);
            renderScene();
            switchPlayer();

            while(countMovePossible(b, activePlayer) == 0){

                switchPlayer();
                if(countMovePossible(b, activePlayer) == 0){
                    *ended = 1;
                    return;
                } else {
                    fillOpenMoves(b, activePlayer);
                    t = makeMove(train1, b);
                    SleepEx(1000, FALSE);
                    flipCaptured(b, t, activePlayer);
                    free(t);
                     renderScene();
                    switchPlayer();

                }
            }
            fillOpenMoves(b, activePlayer);

}
void startTraining(){
    srand(time(NULL));
    int counter, ended, wins, winpos, wincount, longstab, shortstab, white, black;
    FILE* heuristic_log;
    struct tm *tm;
    char str_time[100];
    time_t t;
    t = time(NULL);
    tm = localtime(&t);
    strftime(str_time, sizeof(str_time), "%H %M %S %d %m %y", tm);
    heuristic_log = fopen(str_time, "w");
    wins = 0;
    winpos = 22;
    wincount = 53;
    longstab = 35;
    shortstab = 79;
    counter = 0;
    while(wins < 50 && counter < 100){
        activePlayer = BLACK;
        b = createBoard();
        ended = 0;
        fillOpenMoves(b, activePlayer);
        //train1 = createComputer(b, WHITE, BLACK, 0, 100, 0, 0);
        //train2 = createComputer(b, BLACK, BLACK, 0, 0, 100, 0);
        train1 = createComputer(b, WHITE, BLACK, rand() % 101, rand() % 101, rand() % 101, rand() % 101);
        train2 = createComputer(b, BLACK, BLACK, winpos, wincount, shortstab, longstab);
        //createComputer(Board* b, short compplayer, short activeboardplayer, int positional, int countt, int longstable, int shortstable)
        while(!ended){
           runTrainingGame(train1, train2, &ended);
        }
        white = countPieces(b, WHITE);
        black = countPieces(b, BLACK);
        if(white > black){
             if(wins > 5){
               fprintf(heuristic_log, "good heuristic logged with pos: %i, count: %i, long: %i, short: %i and %i wins\n",winpos,wincount,longstab,shortstab, wins);
           }
            winpos = train1->positional;
            wincount = train1->countt;
            longstab = train1->longstable;
           shortstab = train1->shortstable;
            printf("new winner with pos: %i, count: %i, long: %i, short: %i\n",winpos,wincount,longstab,shortstab);
            wins = 1;

        } else {
                  wins++;
            printf("same winner with pos: %i, count: %i, long: %i, short: %i and %i wins\n",winpos,wincount,longstab,shortstab,wins);
        }
        destroyBoard(b);
        destroyComputer(train1);
        destroyComputer(train2);
        counter++;
    }
    fclose(heuristic_log);
}

void startnewgame(short tempplayerColor){
    short result;
    Turn* t;
    b = createBoard();
    result = MessageBox(NULL, "Would you like to go first", "Would you like to go first", MB_YESNO);
    if(result == IDYES){
        playerColor = BLACK;
    } else {
        playerColor = WHITE;
    }
    compPlayer = otherPlayer(playerColor);
    activePlayer = BLACK;



    fillOpenMoves(b, activePlayer);

    compOpp = createComputer(b, compPlayer, activePlayer, 22, 53, 35, 79);
    if(activePlayer == compPlayer){
        t = makeMove(compOpp, b);
        flipCaptured(b, t, activePlayer);
        switchPlayer();
        fillOpenMoves(b, activePlayer);
    }
}
void endGame(){
    destroyBoard(b);
    destroyComputer(compOpp);
}

void renderScene(void) {
    int i, j, count;
    MoveNode* itr;
    if(b == NULL) return;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, screenBounds.width, 0, screenBounds.height, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glBindTexture(GL_TEXTURE_2D, boardtex);
	glBegin(GL_QUADS);
        glTexCoord2f(0, 1);glVertex3f(0, 0 + screenBounds.height, 0.0f);
        glTexCoord2f(1, 1);glVertex3f(0 + screenBounds.width, 0 + screenBounds.height, 0.0f);
        glTexCoord2f(1, 0);glVertex3f(0 + screenBounds.width, 0, 0.0f);
        glTexCoord2f(0, 0);glVertex3f(0, 0, 0.0f);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, boardtex2);
    glColor3f(1, 1, 1);
	glBegin(GL_QUADS);
        glTexCoord2f(0, 1);glVertex3f(boardBounds.x - boardboundoff, boardBounds.y - boardboundoff, 0.0f);
        glTexCoord2f(1, 1);glVertex3f(boardBounds.x + boardBounds.width + boardboundoff, boardBounds.y - boardboundoff, 0.0f);
        glTexCoord2f(1, 0);glVertex3f(boardBounds.x + boardBounds.width + boardboundoff, boardBounds.height + boardBounds.y + boardboundoff, 0.0f);
        glTexCoord2f(0, 0);glVertex3f(boardBounds.x - 10, boardBounds.height + boardBounds.y + boardboundoff, 0.0f);
	glEnd();
    glColor3f(1.0, 1.0, 1.0);
	for(i = 0; i < 8; i++){
        for(j = 0; j < 8; j++){
            if(getTile(b, i, j) != EMPTY){
                if(getTile(b, i, j) == BLACK){
                    glBindTexture(GL_TEXTURE_2D, blacktex);
                } else {
                    glBindTexture(GL_TEXTURE_2D, whitetex);
                }
                glTranslatef(boardBounds.x + boardBounds.width/8*(i), boardBounds.y + boardBounds.height/8*(8 - (j+1)), 0);
                glBegin(GL_QUADS);
                        glTexCoord2f(0, 1);glVertex3f(0, 0, 0.0f);
                        glTexCoord2f(1, 1);glVertex3f(boardBounds.width/8, 0, 0.0f);
                        glTexCoord2f(1, 0);glVertex3f(boardBounds.width/8, boardBounds.height/8, 0.0f);
                        glTexCoord2f(0, 0);glVertex3f(0, boardBounds.height/8, 0.0f);
                glEnd();

                glEnd();
                glLoadIdentity();
            }
        }
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	if(displayMoves){
        glColor3f(1.0, 0.0, 0.0);
        itr = b->openMoves->head;
        while(itr != NULL_PTR){
            glTranslatef(boardBounds.x + boardBounds.width/8*(itr->turn->x + 0.5), boardBounds.y + boardBounds.height/8*(8 - (itr->turn->y + 0.5)), 0);
            glBegin(GL_POLYGON);
            for(count = 0; count < 20; count ++){
                glVertex3f(sin(2*PI / 20 * count)*boardBounds.width/64, cos(2*PI / 20 * count)*boardBounds.height/64, 0.0f);
            }
            glEnd();
            glLoadIdentity();
            itr = itr->next;
        }
	}
    glColor3f(1.0, 1.0, 1.0);
    glBindTexture(GL_TEXTURE_2D, newGame);
    glBegin(GL_QUADS);
            glTexCoord2f(0, 1);glVertex3f(newButton.x, screenBounds.height - newButton.y, 0.0f);
            glTexCoord2f(1, 1);glVertex3f(newButton.x + newButton.width, screenBounds.height - newButton.y, 0.0f);
            glTexCoord2f(1, 0);glVertex3f(newButton.x + newButton.width, screenBounds.height - (newButton.y + newButton.height), 0.0f);
            glTexCoord2f(0, 0);glVertex3f(newButton.x, screenBounds.height - (newButton.y + newButton.height), 0.0f);
      glEnd();
      glBindTexture(GL_TEXTURE_2D, quit);

      glBegin(GL_QUADS);
            glTexCoord2f(0, 1);glVertex3f(quiButton.x, screenBounds.height - quiButton.y, 0.0f);
            glTexCoord2f(1, 1);glVertex3f(quiButton.x + quiButton.width, screenBounds.height - quiButton.y, 0.0f);
            glTexCoord2f(1, 0);glVertex3f(quiButton.x + newButton.width, screenBounds.height - (quiButton.y + quiButton.height), 0.0f);
            glTexCoord2f(0, 0);glVertex3f(quiButton.x, screenBounds.height - (quiButton.y + quiButton.height), 0.0f);
          glEnd();

      glBindTexture(GL_TEXTURE_2D, toggletex);

      glBegin(GL_QUADS);
            glTexCoord2f(0, 1);glVertex3f(toggle.x, screenBounds.height - toggle.y, 0.0f);
            glTexCoord2f(1, 1);glVertex3f(toggle.x + toggle.width, screenBounds.height - toggle.y, 0.0f);
            glTexCoord2f(1, 0);glVertex3f(toggle.x + toggle.width, screenBounds.height - (toggle.y + toggle.height), 0.0f);
            glTexCoord2f(0, 0);glVertex3f(toggle.x, screenBounds.height - (toggle.y + toggle.height), 0.0f);
          glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
    glutSwapBuffers();

}

void endPopup(){
        char winString[50];
        int blackcount, whitecount, result;
    renderScene();
    blackcount = countPieces(b, BLACK);
    whitecount = countPieces(b, WHITE);
    if(whitecount > blackcount){
        sprintf(winString, "White Wins with %i counters to Black's %i\nPlay Again?", whitecount, blackcount);
    }else if(whitecount < blackcount){
        sprintf(winString, "Black Wins with %i counters to Whites's %i\nPlay Again?", blackcount, whitecount);
    } else {
        sprintf(winString, "A draw with both players have %i counters\nPlay Again?", whitecount);
    }
    result = MessageBox(NULL, winString, "Game Over", MB_YESNO);
    if(result == IDYES){
        endGame();
        startnewgame(BLACK);
        return;
    } else {
        endGame();
        exit(0);
    }
}

void processKeyboard(int button, int state, int x, int y){
//    printf("Clicked\n");
    int boardx, boardy;
    short cellX, cellY, goodMove;
    MoveNode* itr;
    Turn* itrTurn;
    Turn* compTurn;

    goodMove = 0;
    if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
        if(contains(&newButton, x, y)){
//                printf("ended");
            endGame();
            startnewgame(BLACK);
        } else if(contains(&quiButton, x, y)){
            endGame();
            exit(0);
        } else if(contains(&toggle, x, y)){
            displayMoves = (displayMoves+1) % 2;
        } else if(contains(&boardBounds, x, y)){
            if(activePlayer == playerColor){
                boardx = x - boardBounds.x;
                boardy = y - boardBounds.y;
                cellX = (int) boardx / (boardBounds.width /8);
                cellY = (int) boardy / (boardBounds.height /8);
//                printf("click was made on cell %i %i\n", cellX, cellY);
                if(isMoveListEmpty(b->openMoves)){
//                    printf("Move List was empty\n");
                } else {
                    itr = b->openMoves->head;
                    while(itr != NULL_PTR){
                        itrTurn = itr->turn;
                        if(itrTurn->x == cellX && itrTurn->y == cellY){
                            goodMove = 1;
                            break;
                        }
                        itr = itr->next;
                    }
                    if(goodMove){
                        printf("move was good\n");
                        flipCaptured(b, itrTurn, activePlayer);
                        printf("pieces captured\n");
                        switchPlayer();
                        renderScene();
                        printf("computer data updated\n");
                        if(!countMovePossible(b, activePlayer)){
                            switchPlayer();
                            if(countMovePossible(b, activePlayer)){

                                fillOpenMoves(b, activePlayer);

                                return;
                            } else {
                                endPopup();
                            }
                        } else {
                            fillOpenMoves(b, activePlayer);

                        }
                        renderScene();
                        /*if(fillOpenMoves(b, activePlayer)){
                            deadMoves++;
                            if(deadMoves == 2){
                               endPopup();
                            }


                        } else {
                            deadMoves = 0;
                        }
                        */
                        SleepEx(500, TRUE);
                        compTurn = makeMove(compOpp, b);
                        printf("computer calculated move at %i, %i\n", compTurn->x, compTurn->y);

                        flipCaptured(b, compTurn, activePlayer);
                        switchPlayer();
                        printf("computer pieces captured\n");
                        //fillOpenMoves(b, activePlayer);
                        //commene
                        renderScene();
                        printf("moves filled\n");
                        printf("computer data updated again\n");
                        while(!countMovePossible(b, activePlayer)){
                            MessageBox(NULL, "You Cant GO", "You Cant GO", MB_OK);
                            switchPlayer();
                            if(!countMovePossible(b, activePlayer)){
                                endPopup();
                            } else {
                                fillOpenMoves(b, activePlayer);
                                compTurn = makeMove(compOpp, b);
                                printf("computer calculated move at %i, %i\n", compTurn->x, compTurn->y);
                                flipCaptured(b, compTurn, activePlayer);
                                switchPlayer();
                                printf("computer pieces captured\n");
                                //fillOpenMoves(b, activePlayer);
                                //commene
                                renderScene();
                                printf("moves filled\n");
                                printf("computer data updated again\n");
                            }

                        }
                        fillOpenMoves(b, activePlayer);
                        renderScene();
                        /*if(fillOpenMoves(b, activePlayer)){
                            deadMoves++;
                            if(deadMoves == 2){
                               endPopup();
                            }


                        } else {
                            deadMoves = 0;
                        }*/

                        destroyTurn(compTurn);
                        printf("turn destroyed\n");
                    }
                }
            }
        }
        printf("click code finished\n");
    }
}


void resize(int width, int height) {
    // we ignore the params and do:
    glutReshapeWindow( 1000, 600);
};

GLuint loadTexture(char* bname){
    int id;
   id = SOIL_load_OGL_texture
	(
		bname,
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);
    printf( "SOIL: '%s' , texid is %i\n", SOIL_last_result() , id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);
    return id;
}

int main(int argc, char **argv)
{
    b = NULL;
    displayMoves = 0;
    boardBounds.x = 300;
    boardBounds.y = 00;
    boardBounds.width = 700;
    boardBounds.height = 600;
    screenBounds.x = 100;
    screenBounds.y = 100;
    screenBounds.width = 1000;
    screenBounds.height = 600;
    quiButton.x = 10;
    quiButton.width = boardBounds.x - 30;
    quiButton.height = 80;
    quiButton.y = screenBounds.height - quiButton.height - 20;
    newButton.x = 10;
    newButton.width = boardBounds.x - 30;
    newButton.height = 80;
    newButton.y = screenBounds.height - quiButton.height - 20 - newButton.height - 20;;
    toggle.x = 10;
    toggle.width = boardBounds.x - 30;
    toggle.height = 80;
    toggle.y = 20;
	// init GLUT and create Window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(screenBounds.x, screenBounds.y);
	glutInitWindowSize(screenBounds.width, screenBounds.height);
	glutCreateWindow("Reversi");
    glutReshapeFunc(resize);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    newGame = loadTexture("new.png");
    quit = loadTexture("quit.png");
    boardtex = loadTexture("board2.png");
    whitetex = loadTexture("white.png");
    blacktex = loadTexture("black.png");
    boardtex2 = loadTexture("Boardback.png");
    toggletex = loadTexture("toggle.png");
	// register callbacks
	glutDisplayFunc(renderScene);

	glutMouseFunc(processKeyboard);

	// enter GLUT event processing cycle
	glClearColor(0.133, 0.545, 0.133, 1.0);
    glLineWidth(2.0f);


#if TRAINING == 1
startTraining();
#else
startnewgame(WHITE);
#endif




	glutMainLoop();

	return 0;

}
