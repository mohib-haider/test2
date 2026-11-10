/*
 * snake.c - Snake logic and game flow
 *
 * Owner: Mohib
 *
 * Functions in this file:
 *   initSnake(), moveSnake(), growSnake(), checkWallCollision(),
 *   checkSelfCollision(), resetGame(), startGame(), gameTick(),
 *   endGame(), quitToMenu()
 *
 * HOW THE GAME RUNS (GUI version):
 *   There is no "while" loop with Sleep() any more. Instead, Windows
 *   sends a timer message every 'speed' milliseconds. Each message
 *   calls gameTick(), which performs ONE step of the game.
 *
 * NOTE: Global variables (snakeX, snakeY, length, score, etc.) are
 * defined in main.c. Here we only use them. Do NOT define them again.
 *
 * Snake storage: index 0 is the HEAD, index (length - 1) is the TAIL.
 * Positions are in grid cells, not pixels.
 */

#include "game.h"

/* ---------------------------------------------------------------
 * initSnake()
 * Puts the snake in the middle of the grid, moving right.
 * The head is at index 0 and the body extends to the left.
 * --------------------------------------------------------------- */
void initSnake(void)
{
    int i;

    length = START_LENGTH;
    direction = DIR_RIGHT;
    nextDirection = DIR_RIGHT;

    for (i = 0; i < length; i++)
    {
        snakeX[i] = (WIDTH / 2) - i;    /* body goes to the left of the head */
        snakeY[i] = HEIGHT / 2;
    }

    oldTailX = snakeX[length - 1];
    oldTailY = snakeY[length - 1];
}

/* ---------------------------------------------------------------
 * moveSnake()
 * Moves the snake one step in the current direction.
 * Every segment takes the place of the segment in front of it,
 * and the head moves to a new cell.
 * The old tail position is saved so growSnake() can use it.
 * --------------------------------------------------------------- */
void moveSnake(void)
{
    int i;

    /* save the tail position BEFORE it is overwritten */
    oldTailX = snakeX[length - 1];
    oldTailY = snakeY[length - 1];

    /* shift the body: start from the tail and go towards the head */
    for (i = length - 1; i > 0; i--)
    {
        snakeX[i] = snakeX[i - 1];
        snakeY[i] = snakeY[i - 1];
    }

    /* move the head */
    switch (direction)
    {
        case DIR_UP:
            snakeY[0]--;
            break;

        case DIR_DOWN:
            snakeY[0]++;
            break;

        case DIR_LEFT:
            snakeX[0]--;
            break;

        case DIR_RIGHT:
            snakeX[0]++;
            break;

        default:
            break;
    }
}

/* ---------------------------------------------------------------
 * growSnake()
 * Makes the snake one segment longer after eating an apple.
 * The new segment is placed where the tail was before the last move
 * (saved in oldTailX / oldTailY by moveSnake()).
 * Call this AFTER moveSnake().
 * --------------------------------------------------------------- */
void growSnake(void)
{
    if (length < MAX_LENGTH)
    {
        snakeX[length] = oldTailX;
        snakeY[length] = oldTailY;
        length++;
    }
}

/* ---------------------------------------------------------------
 * checkWallCollision()
 * Returns 1 if the head is on a wall, otherwise 0.
 * The walls are at x = 0, x = WIDTH-1, y = 0 and y = HEIGHT-1.
 * --------------------------------------------------------------- */
int checkWallCollision(void)
{
    if (snakeX[0] <= 0 || snakeX[0] >= WIDTH - 1 ||
        snakeY[0] <= 0 || snakeY[0] >= HEIGHT - 1)
    {
        return 1;
    }
    return 0;
}

/* ---------------------------------------------------------------
 * checkSelfCollision()
 * Returns 1 if the head is on any part of the snake's own body,
 * otherwise 0. The head (index 0) is not compared with itself.
 * --------------------------------------------------------------- */
int checkSelfCollision(void)
{
    int i;

    for (i = 1; i < length; i++)
    {
        if (snakeX[0] == snakeX[i] && snakeY[0] == snakeY[i])
        {
            return 1;
        }
    }
    return 0;
}

/* ---------------------------------------------------------------
 * resetGame()
 * Gets everything ready for a new game: new snake, score 0,
 * a fresh apple. The difficulty (speed) is NOT changed.
 * --------------------------------------------------------------- */
void resetGame(void)
{
    initSnake();            /* must come first: generateFood() needs the snake */
    resetScore();
    gameOver = 0;
    paused = 0;
    generateFood();
}

/* ---------------------------------------------------------------
 * startGame()
 * Starts a new game: resets everything, switches to the playing
 * screen and starts the timer that moves the snake.
 * --------------------------------------------------------------- */
void startGame(void)
{
    resetGame();
    gameState = STATE_PLAYING;

    /* Windows will now send a WM_TIMER message every 'speed' ms.
     * Using the same timer ID again simply replaces the old timer. */
    SetTimer(hwndMain, GAME_TIMER_ID, (UINT)speed, NULL);
}

/* ---------------------------------------------------------------
 * gameTick()
 * ONE step of the game. Called by the timer in main.c while the
 * game is being played.
 * Order: use the chosen direction, move, check collisions,
 * check food.
 * --------------------------------------------------------------- */
void gameTick(void)
{
    /* 1. take the direction the player chose with the arrow keys */
    direction = nextDirection;

    /* 2. move the snake */
    moveSnake();

    /* 3. check for collisions */
    if (checkWallCollision() || checkSelfCollision())
    {
        endGame();
        return;
    }

    /* 4. check if the snake ate the apple */
    if (checkFoodEaten())
    {
        growSnake();
        updateScore();
        generateFood();     /* sets gameOver if the board is completely full */

        if (gameOver)
        {
            endGame();
        }
    }
}

/* ---------------------------------------------------------------
 * endGame()
 * Stops the timer, saves the high score if it was beaten and
 * shows the game over screen.
 * --------------------------------------------------------------- */
void endGame(void)
{
    KillTimer(hwndMain, GAME_TIMER_ID);
    gameOver = 1;
    updateHighScore();
    gameState = STATE_GAMEOVER;
}

/* ---------------------------------------------------------------
 * quitToMenu()
 * Stops whatever is running and goes back to the main menu.
 * Used by ESC and by the "Main Menu" / "Back" buttons.
 * --------------------------------------------------------------- */
void quitToMenu(void)
{
    KillTimer(hwndMain, GAME_TIMER_ID);
    updateHighScore();
    paused = 0;
    gameState = STATE_MENU;
}
