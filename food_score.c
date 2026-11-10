/*
 * food_score.c - Food, score, high score, difficulty and pause
 *
 * Owner: Hasan
 *
 * Functions in this file:
 *   generateFood(), checkFoodEaten(), updateScore(), resetScore(),
 *   loadHighScore(), saveHighScore(), updateHighScore(),
 *   setDifficulty(), pauseGame()
 *
 * NOTE: Global variables (snakeX, score, speed, etc.) are defined in
 * main.c. Here we only use them. Do NOT define them again.
 */

#include "game.h"

/* ---------------------------------------------------------------
 * generateFood()
 * Places the apple at a random cell inside the walls.
 * Makes sure the apple never appears on the snake's body.
 * --------------------------------------------------------------- */
void generateFood(void)
{
    static int seeded = 0;      /* seed the random generator only once */
    int i;
    int onSnake;

    if (!seeded)
    {
        srand((unsigned int)time(NULL));
        seeded = 1;
    }

    /* If the snake fills the entire play area there is no space left.
     * The player has won, so end the game to avoid an infinite loop. */
    if (length >= (WIDTH - 2) * (HEIGHT - 2))
    {
        gameOver = 1;
        return;
    }

    do
    {
        /* playable cells: x = 1..WIDTH-2, y = 1..HEIGHT-2 */
        foodX = (rand() % (WIDTH - 2)) + 1;
        foodY = (rand() % (HEIGHT - 2)) + 1;

        /* check that this spot is not on any part of the snake */
        onSnake = 0;
        for (i = 0; i < length; i++)
        {
            if (snakeX[i] == foodX && snakeY[i] == foodY)
            {
                onSnake = 1;
                break;
            }
        }
    } while (onSnake);
}

/* ---------------------------------------------------------------
 * checkFoodEaten()
 * Returns 1 if the snake's head is on the apple, otherwise 0.
 * --------------------------------------------------------------- */
int checkFoodEaten(void)
{
    if (snakeX[0] == foodX && snakeY[0] == foodY)
    {
        return 1;
    }
    return 0;
}

/* ---------------------------------------------------------------
 * updateScore()
 * Adds points to the score after eating an apple.
 * --------------------------------------------------------------- */
void updateScore(void)
{
    score += POINTS_PER_APPLE;
}

/* ---------------------------------------------------------------
 * resetScore()
 * Sets the score back to 0 for a new game.
 * --------------------------------------------------------------- */
void resetScore(void)
{
    score = 0;
}

/* ---------------------------------------------------------------
 * loadHighScore()
 * Reads the high score from highscore.txt.
 * If the file does not exist yet, the high score is 0.
 * --------------------------------------------------------------- */
void loadHighScore(void)
{
    FILE *fp = fopen(HIGHSCORE_FILE, "r");

    if (fp == NULL)
    {
        highScore = 0;      /* first time playing, no file yet */
        return;
    }

    if (fscanf(fp, "%d", &highScore) != 1)
    {
        highScore = 0;      /* file was empty or damaged */
    }

    if (highScore < 0)
    {
        highScore = 0;
    }

    fclose(fp);
}

/* ---------------------------------------------------------------
 * saveHighScore()
 * Writes the current high score to highscore.txt.
 * --------------------------------------------------------------- */
void saveHighScore(void)
{
    FILE *fp = fopen(HIGHSCORE_FILE, "w");

    if (fp == NULL)
    {
        return;             /* could not open the file, skip saving */
    }

    fprintf(fp, "%d", highScore);
    fclose(fp);
}

/* ---------------------------------------------------------------
 * updateHighScore()
 * If the current score is better than the high score,
 * replace it and save it to the file.
 * --------------------------------------------------------------- */
void updateHighScore(void)
{
    if (score > highScore)
    {
        highScore = score;
        saveHighScore();
    }
}

/* ---------------------------------------------------------------
 * setDifficulty()
 * Sets the difficulty level and the game speed (delay in ms).
 * The buttons on the difficulty screen (ui.c) call this function.
 * --------------------------------------------------------------- */
void setDifficulty(int level)
{
    switch (level)
    {
        case EASY:
            difficulty = EASY;
            speed = SPEED_EASY;
            break;

        case HARD:
            difficulty = HARD;
            speed = SPEED_HARD;
            break;

        case MEDIUM:
        default:
            difficulty = MEDIUM;
            speed = SPEED_MEDIUM;
            break;
    }
}

/* ---------------------------------------------------------------
 * pauseGame()
 * Pauses the game if it is running, or resumes it if it is paused.
 * Called when the player presses P or clicks the Resume button.
 * While gameState is STATE_PAUSED the timer messages are ignored
 * (see WM_TIMER in main.c), so the snake stops moving.
 * --------------------------------------------------------------- */
void pauseGame(void)
{
    if (gameState == STATE_PLAYING)
    {
        paused = 1;
        gameState = STATE_PAUSED;
    }
    else if (gameState == STATE_PAUSED)
    {
        paused = 0;
        gameState = STATE_PLAYING;
    }
}
