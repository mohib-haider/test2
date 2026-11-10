/*
 * game.h - Shared header for the Snake Game (C, Windows GUI)
 *
 * Team: Mohib, Aarish, Hasan
 *
 * This version uses the Windows API (Win32 + GDI) to draw a real
 * window with graphics, buttons and mouse support.
 * No extra libraries need to be installed.
 *
 * RULES:
 *  1. Every .c file must start with:  #include "game.h"
 *  2. Do NOT change anything in this file without telling the whole team.
 *  3. Do NOT rename functions or variables after Day 1.
 *  4. The global variables are DEFINED once in main.c (Mohib)
 *     and only DECLARED here with "extern".
 */

#ifndef GAME_H
#define GAME_H

/* ---------------------------------------------------------------
 * LIBRARIES
 * --------------------------------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>    /* Win32 window, timer, keyboard, mouse, GDI drawing */

/* ---------------------------------------------------------------
 * CONSTANTS
 * --------------------------------------------------------------- */

/* Game grid size in cells (the outer ring of cells is the wall).
 * Playable cells: x = 1 .. WIDTH-2,  y = 1 .. HEIGHT-2 */
#define WIDTH           30
#define HEIGHT          20

/* Size of one cell on the screen, in pixels */
#define CELL            24

/* Height of the score bar at the top of the window, in pixels */
#define HUD_HEIGHT      50

/* Size of the window's drawing area, in pixels */
#define WINDOW_W        (WIDTH * CELL)
#define WINDOW_H        (HEIGHT * CELL + HUD_HEIGHT)

/* Maximum snake length = number of cells (safe upper limit) */
#define MAX_LENGTH      (WIDTH * HEIGHT)

/* Snake starting length */
#define START_LENGTH    3

/* Points gained for each apple */
#define POINTS_PER_APPLE 10

/* Directions */
#define DIR_UP      1
#define DIR_DOWN    2
#define DIR_LEFT    3
#define DIR_RIGHT   4

/* Difficulty levels */
#define EASY    1
#define MEDIUM  2
#define HARD    3

/* Game speed = time between two snake moves, in milliseconds.
 * Smaller delay = faster snake. */
#define SPEED_EASY      150
#define SPEED_MEDIUM    100
#define SPEED_HARD      60

/* File where the high score is stored */
#define HIGHSCORE_FILE  "highscore.txt"

/* ID of the Windows timer that makes the snake move */
#define GAME_TIMER_ID   1

/* Screens / states of the program (stored in gameState) */
#define STATE_MENU          0   /* main menu                  */
#define STATE_DIFFICULTY    1   /* choose Easy / Medium / Hard */
#define STATE_PLAYING       2   /* game is running            */
#define STATE_PAUSED        3   /* game is paused             */
#define STATE_GAMEOVER      4   /* game over screen           */
#define STATE_HIGHSCORE     5   /* high score screen          */

/* ---------------------------------------------------------------
 * GLOBAL VARIABLES  (defined in main.c, shared by everyone)
 * --------------------------------------------------------------- */

/* Snake body: index 0 is the HEAD, index (length-1) is the TAIL */
extern int snakeX[MAX_LENGTH];
extern int snakeY[MAX_LENGTH];
extern int length;          /* current snake length                       */
extern int direction;       /* direction used for the LAST move           */
extern int nextDirection;   /* direction chosen by the player for the
                               NEXT move (set by the arrow keys)          */

/* Food position (in cells) */
extern int foodX;
extern int foodY;

/* Score */
extern int score;           /* current score                              */
extern int highScore;       /* best score (loaded from highscore.txt)     */

/* Game settings and state */
extern int difficulty;      /* EASY, MEDIUM or HARD                       */
extern int speed;           /* time between moves in ms                   */
extern int gameOver;        /* 1 = game finished, 0 = still playing       */
extern int paused;          /* 1 = game paused,   0 = running             */
extern int gameState;       /* which screen is showing (STATE_...)        */

/* The tail position that was left by the last moveSnake().
 * growSnake() puts the new segment there. */
extern int oldTailX;
extern int oldTailY;

/* Handle of the game window (needed for timer and redraw) */
extern HWND hwndMain;

/* ---------------------------------------------------------------
 * FUNCTIONS OWNED BY MOHIB   (main.c, snake.c)
 * --------------------------------------------------------------- */

/* main.c: WinMain() creates the window and runs the message loop,
 * WndProc() receives all events (paint, timer, keys, mouse).       */

void initSnake(void);           /* set start position, length, direction     */
void moveSnake(void);           /* move one step; save old tail position     */
void growSnake(void);           /* increase length by 1 after eating         */
int  checkWallCollision(void);  /* returns 1 if head hit a wall, else 0      */
int  checkSelfCollision(void);  /* returns 1 if head hit own body, else 0    */
void resetGame(void);           /* reset snake, score, food for a new game   */
void startGame(void);           /* reset, switch to playing, start the timer */
void gameTick(void);            /* ONE game step (called by the timer)       */
void endGame(void);             /* stop the timer, save score, game over     */
void quitToMenu(void);          /* stop the game and go back to the menu     */

/* ---------------------------------------------------------------
 * FUNCTIONS OWNED BY AARISH  (ui.c)
 * All drawing functions receive the device context (hdc) to draw on.
 * --------------------------------------------------------------- */

void drawFrame(HDC hdc);            /* draws the correct screen for gameState */
void drawBoard(HDC hdc);            /* grass and walls                        */
void drawSnake(HDC hdc);            /* snake body and head with eyes          */
void drawFood(HDC hdc);             /* the apple                              */
void drawScore(HDC hdc);            /* score bar at the top                   */
void drawMainMenu(HDC hdc);         /* main menu with buttons                 */
void drawDifficultyScreen(HDC hdc); /* Easy / Medium / Hard buttons           */
void drawHighScoreScreen(HDC hdc);  /* shows the best score                   */
void drawPauseScreen(HDC hdc);      /* "PAUSED" box over the game             */
void drawGameOverScreen(HDC hdc);   /* "GAME OVER" box over the game          */

void handleKeyDown(int key);        /* keyboard: arrows, P, ESC, 1-4, Y, N    */
void handleMouseClick(int x, int y);/* mouse click on a button                */
void handleMouseMove(int x, int y); /* mouse hover highlight on buttons       */

/* ---------------------------------------------------------------
 * FUNCTIONS OWNED BY HASAN  (food_score.c)
 * --------------------------------------------------------------- */

void generateFood(void);        /* random foodX, foodY, never on the snake   */
int  checkFoodEaten(void);      /* returns 1 if head is on the food, else 0  */
void updateScore(void);         /* score += POINTS_PER_APPLE                 */
void resetScore(void);          /* score = 0                                 */
void loadHighScore(void);       /* read highScore from HIGHSCORE_FILE        */
void saveHighScore(void);       /* write highScore to HIGHSCORE_FILE         */
void updateHighScore(void);     /* if score > highScore, update and save     */
void setDifficulty(int level);  /* set 'difficulty' and 'speed' (EASY...)    */
void pauseGame(void);           /* pause if playing, resume if paused        */

#endif /* GAME_H */
