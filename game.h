#ifndef GAME_H
#define GAME_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>

#define WIDTH           30
#define HEIGHT          20

#define CELL            24

#define HUD_HEIGHT      50

#define WINDOW_W        (WIDTH * CELL)
#define WINDOW_H        (HEIGHT * CELL + HUD_HEIGHT)

#define MAX_LENGTH      (WIDTH * HEIGHT)

#define START_LENGTH    3

#define POINTS_PER_APPLE 10

#define DIR_UP      1
#define DIR_DOWN    2
#define DIR_LEFT    3
#define DIR_RIGHT   4

#define EASY    1
#define MEDIUM  2
#define HARD    3

#define SPEED_EASY      150
#define SPEED_MEDIUM    100
#define SPEED_HARD      60

#define HIGHSCORE_FILE  "highscore.txt"

#define GAME_TIMER_ID   1

#define STATE_MENU          0
#define STATE_DIFFICULTY    1
#define STATE_PLAYING       2
#define STATE_PAUSED        3
#define STATE_GAMEOVER      4
#define STATE_HIGHSCORE     5

extern int snakeX[MAX_LENGTH];
extern int snakeY[MAX_LENGTH];
extern int length;
extern int direction;
extern int nextDirection;

extern int foodX;
extern int foodY;

extern int score;
extern int highScore;

extern int difficulty;
extern int speed;
extern int gameOver;
extern int paused;
extern int gameState;


extern HWND hwndMain;

void initSnake(void);
void moveSnake(void);
void growSnake(void);
int  checkWallCollision(void);
int  checkSelfCollision(void);
void resetGame(void);
void startGame(void);
void gameTick(void);
void endGame(void);
void quitToMenu(void);

void drawFrame(HDC hdc);
void drawBoard(HDC hdc);
void drawSnake(HDC hdc);
void drawFood(HDC hdc);
void drawScore(HDC hdc);
void drawMainMenu(HDC hdc);
void drawDifficultyScreen(HDC hdc);
void drawHighScoreScreen(HDC hdc);
void drawPauseScreen(HDC hdc);
void drawGameOverScreen(HDC hdc);

void handleKeyDown(int key);
void handleMouseClick(int x, int y);
void handleMouseMove(int x, int y);

void generateFood(void);
int  checkFoodEaten(void);
void updateScore(void);
void resetScore(void);
void loadHighScore(void);
void saveHighScore(void);
void updateHighScore(void);
void setDifficulty(int level);
void pauseGame(void);

#endif
