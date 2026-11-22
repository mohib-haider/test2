#include "game.h"

/* ---------- colours ---------- */
#define CLR_GRASS1  RGB(170, 215, 81)
#define CLR_GRASS2  RGB(162, 209, 73)
#define CLR_WALL    RGB(74, 117, 44)
#define CLR_BAR     RGB(55, 90, 35)
#define CLR_BODY    RGB(78, 124, 246)
#define CLR_HEAD    RGB(52, 95, 214)
#define CLR_FOOD    RGB(231, 71, 29)
#define CLR_DARK    RGB(28, 45, 32)
#define CLR_WHITE   RGB(255, 255, 255)
#define CLR_YELLOW  RGB(255, 206, 84)
#define CLR_GRAY    RGB(170, 185, 170)
#define CLR_RED     RGB(255, 99, 71)
#define CLR_GREEN   RGB(67, 160, 71)
#define CLR_LIGHT   RGB(102, 187, 106)

/* ---------- button layout: 4 slots, one under the other ---------- */
#define MID         (WINDOW_W / 2)
#define BTN_LEFT    230
#define BTN_RIGHT   490
#define BTN_HEIGHT  50

static int hover = -1;                 /* slot under the mouse, -1 = none */
static const char *levelName[] = {"", "Easy", "Medium", "Hard"};

/* ---------- small drawing helpers ---------- */

/* Fills a rectangle with one colour. */
static void box(HDC hdc, int left, int top, int right, int bottom, COLORREF color)
{
    RECT r = {left, top, right, bottom};
    HBRUSH brush = CreateSolidBrush(color);
    FillRect(hdc, &r, brush);
    DeleteObject(brush);
}

/* Fills one grid cell (cell numbers -> pixels). margin = gap inside the cell. */
static void cell(HDC hdc, int x, int y, int margin, COLORREF color)
{
    int left = x * CELL;
    int top = HUD_HEIGHT + y * CELL;
    box(hdc, left + margin, top + margin, left + CELL - margin, top + CELL - margin, color);
}

/* Writes text at (x, y). align is TA_LEFT, TA_CENTER or TA_RIGHT. */
static void drawText(HDC hdc, const char *s, int x, int y, int size, COLORREF color, int align)
{
    HFONT font = CreateFontA(size, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, "Segoe UI");
    HFONT old = (HFONT)SelectObject(hdc, font);
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, color);
    SetTextAlign(hdc, align);
    TextOutA(hdc, x, y, s, (int)strlen(s));
    SelectObject(hdc, old);
    DeleteObject(font);
}

/* Top edge of button slot 0..3 (slots are 65 pixels apart). */
static int buttonTop(int slot)
{
    return 190 + slot * 65;
}

/* Draws one button in a slot; lighter green if the mouse is over it. */
static void drawButton(HDC hdc, const char *label, int slot)
{
    int top = buttonTop(slot);
    box(hdc, BTN_LEFT, top, BTN_RIGHT, top + BTN_HEIGHT, slot == hover ? CLR_LIGHT : CLR_GREEN);
    drawText(hdc, label, MID, top + 10, 26, CLR_WHITE, TA_CENTER);
}

/* Dark background with a big title (used by the three menu screens). */
static void drawBackground(HDC hdc, const char *title)
{
    box(hdc, 0, 0, WINDOW_W, WINDOW_H, CLR_DARK);
    drawText(hdc, title, MID, 40, 64, CLR_YELLOW, TA_CENTER);
}

/* Yellow-framed dark box (used by the pause and game over screens). */
static void drawPanel(HDC hdc)
{
    box(hdc, 150, 100, 570, 450, CLR_YELLOW);
    box(hdc, 153, 103, 567, 447, CLR_DARK);
}

/* ---------- buttons: which slot is the mouse on, and what does a slot do ---------- */

/* The first slot that has a button on the current screen (4 = no buttons). */
static int firstSlot(void)
{
    if (gameState == STATE_MENU || gameState == STATE_DIFFICULTY)
        return 0;
    if (gameState == STATE_HIGHSCORE)
        return 3;
    if (gameState == STATE_PAUSED || gameState == STATE_GAMEOVER)
        return 2;
    return 4;
}

/* Returns the slot at pixel (x, y), or -1 if there is no button there. */
static int buttonAt(int x, int y)
{
    int slot;

    if (x < BTN_LEFT || x >= BTN_RIGHT)
        return -1;
    for (slot = firstSlot(); slot < 4; slot++)
        if (y >= buttonTop(slot) && y < buttonTop(slot) + BTN_HEIGHT)
            return slot;
    return -1;
}

/* What pressing a button does. Mouse clicks and number keys both use this. */
static void doButton(int slot)
{
    hover = -1;
    if (gameState == STATE_MENU)
    {
        if (slot == 0)
            startGame();
        else if (slot == 1)
            gameState = STATE_DIFFICULTY;
        else if (slot == 2)
            gameState = STATE_HIGHSCORE;
        else
            DestroyWindow(hwndMain);
    }
    else if (gameState == STATE_DIFFICULTY)
    {
        if (slot < 3)
            setDifficulty(slot + 1);
        gameState = STATE_MENU;
    }
    else if (gameState == STATE_HIGHSCORE)
        gameState = STATE_MENU;
    else if (gameState == STATE_PAUSED)
    {
        if (slot == 2)
            pauseGame();
        else
            quitToMenu();
    }
    else if (gameState == STATE_GAMEOVER)
    {
        if (slot == 2)
            startGame();
        else
            gameState = STATE_MENU;
    }
}

/* ---------- the game screen ---------- */

void drawBoard(HDC hdc)
{
    int x, y;

    for (y = 0; y < HEIGHT; y++)
        for (x = 0; x < WIDTH; x++)
        {
            if (x == 0 || y == 0 || x == WIDTH - 1 || y == HEIGHT - 1)
                cell(hdc, x, y, 0, CLR_WALL);
            else if ((x + y) % 2 == 0)
                cell(hdc, x, y, 0, CLR_GRASS1);
            else
                cell(hdc, x, y, 0, CLR_GRASS2);
        }
}

void drawSnake(HDC hdc)
{
    int i;

    cell(hdc, snakeX[0], snakeY[0], 1, CLR_HEAD);
    for (i = 1; i < length; i++)
        cell(hdc, snakeX[i], snakeY[i], 1, CLR_BODY);
}

void drawFood(HDC hdc)
{
    cell(hdc, foodX, foodY, 4, CLR_FOOD);
}

void drawScore(HDC hdc)
{
    char text[40];

    box(hdc, 0, 0, WINDOW_W, HUD_HEIGHT, CLR_BAR);
    sprintf(text, "Score: %d", score);
    drawText(hdc, text, 20, 10, 26, CLR_WHITE, TA_LEFT);
    sprintf(text, "Level: %s", levelName[difficulty]);
    drawText(hdc, text, MID, 14, 22, CLR_GRAY, TA_CENTER);
    sprintf(text, "High Score: %d", highScore);
    drawText(hdc, text, WINDOW_W - 20, 10, 26, CLR_YELLOW, TA_RIGHT);
}

/* ---------- the other screens ---------- */

void drawMainMenu(HDC hdc)
{
    drawBackground(hdc, "SNAKE GAME");
    drawText(hdc, "by Mohib, Aarish and Hasan", MID, 120, 22, CLR_GRAY, TA_CENTER);
    drawButton(hdc, "Start Game", 0);
    drawButton(hdc, "Difficulty", 1);
    drawButton(hdc, "High Score", 2);
    drawButton(hdc, "Exit", 3);
}

void drawDifficultyScreen(HDC hdc)
{
    char text[40];

    drawBackground(hdc, "DIFFICULTY");
    sprintf(text, "Current level: %s", levelName[difficulty]);
    drawText(hdc, text, MID, 120, 22, CLR_GRAY, TA_CENTER);
    drawButton(hdc, "Easy", 0);
    drawButton(hdc, "Medium", 1);
    drawButton(hdc, "Hard", 2);
    drawButton(hdc, "Back", 3);
}

void drawHighScoreScreen(HDC hdc)
{
    char text[40];

    drawBackground(hdc, "HIGH SCORE");
    sprintf(text, "%d", highScore);
    drawText(hdc, text, MID, 190, 110, CLR_WHITE, TA_CENTER);
    drawButton(hdc, "Back", 3);
}

void drawPauseScreen(HDC hdc)
{
    drawPanel(hdc);
    drawText(hdc, "PAUSED", MID, 125, 60, CLR_YELLOW, TA_CENTER);
    drawText(hdc, "Press P to resume", MID, 215, 22, CLR_GRAY, TA_CENTER);
    drawButton(hdc, "Resume", 2);
    drawButton(hdc, "Main Menu", 3);
}

void drawGameOverScreen(HDC hdc)
{
    char text[40];

    drawPanel(hdc);
    drawText(hdc, "GAME OVER", MID, 120, 56, CLR_RED, TA_CENTER);
    sprintf(text, "Your score: %d", score);
    drawText(hdc, text, MID, 205, 26, CLR_WHITE, TA_CENTER);
    sprintf(text, "High score: %d", highScore);
    drawText(hdc, text, MID, 240, 22, CLR_GRAY, TA_CENTER);
    if (score > 0 && score == highScore)
        drawText(hdc, "NEW HIGH SCORE!", MID, 272, 26, CLR_YELLOW, TA_CENTER);
    drawButton(hdc, "Play Again", 2);
    drawButton(hdc, "Main Menu", 3);
}

/* Draws the screen that matches gameState. main.c calls this. */
void drawFrame(HDC hdc)
{
    if (gameState == STATE_MENU)
        drawMainMenu(hdc);
    else if (gameState == STATE_DIFFICULTY)
        drawDifficultyScreen(hdc);
    else if (gameState == STATE_HIGHSCORE)
        drawHighScoreScreen(hdc);
    else
    {
        drawScore(hdc);
        drawBoard(hdc);
        drawFood(hdc);
        drawSnake(hdc);
        if (gameState == STATE_PAUSED)
            drawPauseScreen(hdc);
        if (gameState == STATE_GAMEOVER)
            drawGameOverScreen(hdc);
    }
}

/* ---------- keyboard and mouse ---------- */

void handleKeyDown(int key)
{
    if (gameState == STATE_MENU || gameState == STATE_DIFFICULTY)
    {
        if (key >= '1' && key <= '4')
            doButton(key - '1');
        else if (key == VK_ESCAPE)
            doButton(3);
    }
    else if (gameState == STATE_HIGHSCORE)
    {
        if (key == VK_ESCAPE || key == VK_RETURN)
            doButton(3);
    }
    else if (gameState == STATE_PLAYING)
    {
        if (key == VK_UP && direction != DIR_DOWN)
            nextDirection = DIR_UP;
        else if (key == VK_DOWN && direction != DIR_UP)
            nextDirection = DIR_DOWN;
        else if (key == VK_LEFT && direction != DIR_RIGHT)
            nextDirection = DIR_LEFT;
        else if (key == VK_RIGHT && direction != DIR_LEFT)
            nextDirection = DIR_RIGHT;
        else if (key == 'P')
            pauseGame();
        else if (key == VK_ESCAPE)
            quitToMenu();
    }
    else if (gameState == STATE_PAUSED)
    {
        if (key == 'P')
            pauseGame();
        else if (key == VK_ESCAPE)
            quitToMenu();
    }
    else if (gameState == STATE_GAMEOVER)
    {
        if (key == 'Y' || key == VK_RETURN)
            doButton(2);
        else if (key == 'N' || key == VK_ESCAPE)
            doButton(3);
    }
    InvalidateRect(hwndMain, NULL, FALSE);
}

void handleMouseClick(int x, int y)
{
    int slot = buttonAt(x, y);

    if (slot >= 0)
    {
        doButton(slot);
        InvalidateRect(hwndMain, NULL, FALSE);
    }
}

void handleMouseMove(int x, int y)
{
    int slot = buttonAt(x, y);

    if (slot != hover)
    {
        hover = slot;
        InvalidateRect(hwndMain, NULL, FALSE);
    }
}
