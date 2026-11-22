#include "game.h"

/* Puts a new snake of 3 segments in the middle, moving right.
   Index 0 is the head. The body goes to the left of the head. */
void initSnake(void)
{
    int i;

    length = START_LENGTH;
    direction = DIR_RIGHT;
    nextDirection = DIR_RIGHT;
    for (i = 0; i < length; i++)
    {
        snakeX[i] = WIDTH / 2 - i;
        snakeY[i] = HEIGHT / 2;
    }
}

/* Moves the snake one cell. Every segment takes the place of the one in
   front of it (counting from the back), then the head steps forward.
   The loop starts at index 'length', one place BEHIND the tail. So the
   old tail position is always kept in that spare place. */
void moveSnake(void)
{
    int i;

    for (i = length; i > 0; i--)
    {
        snakeX[i] = snakeX[i - 1];
        snakeY[i] = snakeY[i - 1];
    }

    if (direction == DIR_UP)
        snakeY[0]--;
    else if (direction == DIR_DOWN)
        snakeY[0]++;
    else if (direction == DIR_LEFT)
        snakeX[0]--;
    else if (direction == DIR_RIGHT)
        snakeX[0]++;
}

/* Makes the snake one segment longer. The spare place behind the tail
   (filled by moveSnake) simply becomes the new tail. */
void growSnake(void)
{
    if (length < MAX_LENGTH - 1)
        length++;
}

/* 1 if the head is on a wall cell (the outer ring of the grid), else 0. */
int checkWallCollision(void)
{
    if (snakeX[0] <= 0 || snakeX[0] >= WIDTH - 1)
        return 1;
    if (snakeY[0] <= 0 || snakeY[0] >= HEIGHT - 1)
        return 1;
    return 0;
}

/* 1 if the head is on any other part of the snake, else 0.
   The loop starts at 1 because index 0 is the head itself. */
int checkSelfCollision(void)
{
    int i;

    for (i = 1; i < length; i++)
        if (snakeX[0] == snakeX[i] && snakeY[0] == snakeY[i])
            return 1;
    return 0;
}

/* Gets everything ready for a new game.
   The snake must exist before the first apple is placed. */
void resetGame(void)
{
    initSnake();
    resetScore();
    gameOver = 0;
    paused = 0;
    generateFood();
}

/* Starts a new game and the timer. Windows will now send a WM_TIMER
   message every 'speed' milliseconds. Each message calls gameTick(). */
void startGame(void)
{
    resetGame();
    gameState = STATE_PLAYING;
    SetTimer(hwndMain, GAME_TIMER_ID, (UINT)speed, NULL);
}

/* One step of the game. Called by the timer in main.c. */
void gameTick(void)
{
    direction = nextDirection;
    moveSnake();

    if (checkWallCollision() || checkSelfCollision())
        endGame();
    else if (checkFoodEaten())
    {
        growSnake();
        updateScore();
        generateFood();
        if (gameOver)
            endGame();
    }
}

/* The snake died (or the board is full): stop the timer, save the
   high score if it was beaten, show the game over screen. */
void endGame(void)
{
    KillTimer(hwndMain, GAME_TIMER_ID);
    gameOver = 1;
    updateHighScore();
    gameState = STATE_GAMEOVER;
}

/* The player leaves the game (ESC or Main Menu button). */
void quitToMenu(void)
{
    KillTimer(hwndMain, GAME_TIMER_ID);
    updateHighScore();
    paused = 0;
    gameState = STATE_MENU;
}
