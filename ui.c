/*
 * ui.c - Graphics (GDI), menus, buttons, keyboard and mouse input
 *
 * Owner: Aarish
 *
 * Functions in this file:
 *   drawFrame(), drawBoard(), drawSnake(), drawFood(), drawScore(),
 *   drawMainMenu(), drawDifficultyScreen(), drawHighScoreScreen(),
 *   drawPauseScreen(), drawGameOverScreen(),
 *   handleKeyDown(), handleMouseClick(), handleMouseMove()
 *
 * HOW DRAWING WORKS:
 *   Windows asks us to draw the window (WM_PAINT in main.c). main.c
 *   calls drawFrame(hdc), which draws the screen that matches the
 *   current gameState. "hdc" is the surface we draw on.
 *   Whenever something changes we call requestRedraw() so that
 *   Windows asks us to draw again.
 *
 * NOTE: Global variables (snakeX, score, direction, etc.) are defined in
 * main.c. Here we only use them. Do NOT define them again.
 *
 * Positions: grid cell (x, y) is drawn at pixel
 *            left = x * CELL,  top = HUD_HEIGHT + y * CELL
 */

#include "game.h"

/* ---------------------------------------------------------------
 * COLOURS  (RGB = red, green, blue, each from 0 to 255)
 * --------------------------------------------------------------- */
#define CLR_GRASS_A     RGB(170, 215, 81)
#define CLR_GRASS_B     RGB(162, 209, 73)
#define CLR_WALL        RGB(74, 117, 44)
#define CLR_WALL_LIGHT  RGB(87, 138, 52)
#define CLR_HUD         RGB(55, 90, 35)
#define CLR_SNAKE_BODY  RGB(78, 124, 246)
#define CLR_SNAKE_HEAD  RGB(52, 95, 214)
#define CLR_FOOD        RGB(231, 71, 29)
#define CLR_LEAF        RGB(60, 160, 60)
#define CLR_MENU_BG     RGB(28, 45, 32)
#define CLR_WHITE       RGB(255, 255, 255)
#define CLR_BLACK       RGB(0, 0, 0)
#define CLR_YELLOW      RGB(255, 206, 84)
#define CLR_GRAY        RGB(170, 185, 170)
#define CLR_BTN         RGB(67, 160, 71)
#define CLR_BTN_HOVER   RGB(102, 187, 106)
#define CLR_BTN_BORDER  RGB(30, 90, 35)
#define CLR_BTN_ACTIVE  RGB(255, 193, 7)
#define CLR_DARK_TEXT   RGB(30, 30, 30)
#define CLR_RED_TEXT    RGB(255, 99, 71)

/* ---------------------------------------------------------------
 * BUTTONS
 * --------------------------------------------------------------- */
#define BTN_W   260
#define BTN_H   50

/* button IDs */
#define BTN_NONE        0
#define BTN_START       1
#define BTN_DIFFICULTY  2
#define BTN_HIGHSCORE   3
#define BTN_EXIT        4
#define BTN_EASY        5
#define BTN_MEDIUM      6
#define BTN_HARD        7
#define BTN_BACK        8
#define BTN_PLAY_AGAIN  9
#define BTN_MENU        10
#define BTN_RESUME      11

typedef struct
{
    int x, y, w, h;         /* position and size in pixels            */
    const char *label;      /* text on the button                     */
    int id;                 /* BTN_... value                          */
    int highlight;          /* 1 = drawn in yellow (selected option)  */
} Button;

/* The button the mouse is currently over (BTN_NONE if none) */
static int hoverId = BTN_NONE;

/* ---------------------------------------------------------------
 * SMALL HELPER FUNCTIONS (only used inside this file)
 * --------------------------------------------------------------- */

/* Asks Windows to redraw the whole window */
static void requestRedraw(void)
{
    InvalidateRect(hwndMain, NULL, FALSE);
}

/* Makes a RECT (rectangle) from four numbers */
static RECT makeRect(int l, int t, int r, int b)
{
    RECT rc;

    rc.left = l;
    rc.top = t;
    rc.right = r;
    rc.bottom = b;
    return rc;
}

/* Fills a rectangle with one colour */
static void fillRect(HDC hdc, int l, int t, int r, int b, COLORREF color)
{
    HBRUSH brush = CreateSolidBrush(color);
    RECT rc = makeRect(l, t, r, b);

    FillRect(hdc, &rc, brush);
    DeleteObject(brush);
}

/* Draws a rectangle with rounded corners, a fill colour and a border */
static void fillRounded(HDC hdc, int l, int t, int r, int b, int radius,
                        COLORREF fill, COLORREF border, int borderWidth)
{
    HBRUSH brush = CreateSolidBrush(fill);
    HPEN pen = CreatePen(PS_SOLID, borderWidth, border);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);

    RoundRect(hdc, l, t, r, b, radius, radius);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(brush);
    DeleteObject(pen);
}

/* Draws a filled circle with centre (cx, cy) and radius r */
static void fillCircle(HDC hdc, int cx, int cy, int r, COLORREF color)
{
    HBRUSH brush = CreateSolidBrush(color);
    HPEN pen = CreatePen(PS_SOLID, 1, color);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);

    Ellipse(hdc, cx - r, cy - r, cx + r, cy + r);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(brush);
    DeleteObject(pen);
}

/* Draws text inside a rectangle.
 * 'format' says how: DT_CENTER, DT_LEFT, DT_RIGHT, DT_VCENTER ... */
static void drawTextIn(HDC hdc, const char *text, RECT *rect, int size,
                       int bold, COLORREF color, UINT format)
{
    HFONT font = CreateFontA(-size, 0, 0, 0, bold ? FW_BOLD : FW_NORMAL,
                             FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                             OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_SWISS,
                             "Segoe UI");
    HFONT oldFont = (HFONT)SelectObject(hdc, font);

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, color);
    DrawTextA(hdc, text, -1, rect, format);

    SelectObject(hdc, oldFont);
    DeleteObject(font);
}

/* Draws text centred (left-right and up-down) inside a rectangle */
static void drawTextBox(HDC hdc, const char *text, int l, int t, int r, int b,
                        int size, int bold, COLORREF color)
{
    RECT rc = makeRect(l, t, r, b);

    drawTextIn(hdc, text, &rc, size, bold, color,
               DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

/* Returns the name of the current difficulty as text */
static const char *getDifficultyName(void)
{
    if (difficulty == EASY)
    {
        return "Easy";
    }
    if (difficulty == HARD)
    {
        return "Hard";
    }
    return "Medium";
}

/* Fills the whole window with the dark menu colour */
static void drawMenuBackground(HDC hdc)
{
    fillRect(hdc, 0, 0, WINDOW_W, WINDOW_H, CLR_MENU_BG);
}

/* Draws a big title and a small line under it */
static void drawTitle(HDC hdc, const char *title, const char *subtitle)
{
    drawTextBox(hdc, title, 0, 35, WINDOW_W, 115, 60, 1, CLR_YELLOW);
    drawTextBox(hdc, subtitle, 0, 115, WINDOW_W, 150, 18, 0, CLR_GRAY);
}

/* Draws a small decoration: a snake running towards an apple */
static void drawDecoration(HDC hdc)
{
    int i;
    int y = 455;
    int startX = (WINDOW_W - 10 * CELL) / 2;
    int headX = startX + 7 * CELL;
    int appleX = startX + 9 * CELL + CELL / 2;

    for (i = 0; i < 7; i++)
    {
        fillRounded(hdc, startX + i * CELL + 2, y + 2,
                    startX + (i + 1) * CELL - 2, y + CELL - 2,
                    10, CLR_SNAKE_BODY, CLR_SNAKE_BODY, 1);
    }

    /* head with two eyes, looking right */
    fillRounded(hdc, headX + 1, y + 1, headX + CELL - 1, y + CELL - 1,
                12, CLR_SNAKE_HEAD, CLR_SNAKE_HEAD, 1);
    fillCircle(hdc, headX + 16, y + 7, 3, CLR_WHITE);
    fillCircle(hdc, headX + 16, y + 17, 3, CLR_WHITE);
    fillCircle(hdc, headX + 17, y + 7, 1, CLR_BLACK);
    fillCircle(hdc, headX + 17, y + 17, 1, CLR_BLACK);

    /* apple */
    fillCircle(hdc, appleX, y + CELL / 2 + 1, 9, CLR_FOOD);
    fillCircle(hdc, appleX + 3, y + 3, 3, CLR_LEAF);
}

/* Draws the dark box used by the pause and game over screens */
static void drawPanel(HDC hdc)
{
    fillRounded(hdc, 150, 100, 570, 450, 24, CLR_MENU_BG, CLR_YELLOW, 3);
}

/* ---------------------------------------------------------------
 * Button helpers
 * --------------------------------------------------------------- */

/* Creates a button centred horizontally at height y */
static Button makeButton(int y, const char *label, int id, int highlight)
{
    Button b;

    b.x = (WINDOW_W - BTN_W) / 2;
    b.y = y;
    b.w = BTN_W;
    b.h = BTN_H;
    b.label = label;
    b.id = id;
    b.highlight = highlight;
    return b;
}

/* Fills 'list' with the buttons of the current screen and returns
 * how many there are. 'list' must have room for 4 buttons. */
static int getButtons(Button *list)
{
    int n = 0;

    switch (gameState)
    {
        case STATE_MENU:
            list[n++] = makeButton(190, "Start Game", BTN_START, 0);
            list[n++] = makeButton(255, "Difficulty", BTN_DIFFICULTY, 0);
            list[n++] = makeButton(320, "High Score", BTN_HIGHSCORE, 0);
            list[n++] = makeButton(385, "Exit", BTN_EXIT, 0);
            break;

        case STATE_DIFFICULTY:
            list[n++] = makeButton(190, "Easy", BTN_EASY, difficulty == EASY);
            list[n++] = makeButton(255, "Medium", BTN_MEDIUM, difficulty == MEDIUM);
            list[n++] = makeButton(320, "Hard", BTN_HARD, difficulty == HARD);
            list[n++] = makeButton(385, "Back", BTN_BACK, 0);
            break;

        case STATE_HIGHSCORE:
            list[n++] = makeButton(385, "Back", BTN_BACK, 0);
            break;

        case STATE_PAUSED:
            list[n++] = makeButton(320, "Resume", BTN_RESUME, 0);
            list[n++] = makeButton(385, "Main Menu", BTN_MENU, 0);
            break;

        case STATE_GAMEOVER:
            list[n++] = makeButton(320, "Play Again", BTN_PLAY_AGAIN, 0);
            list[n++] = makeButton(385, "Main Menu", BTN_MENU, 0);
            break;

        default:
            break;      /* no buttons while playing */
    }

    return n;
}

/* Draws one button (highlighted if the mouse is over it) */
static void drawButton(HDC hdc, const Button *b)
{
    COLORREF fill = CLR_BTN;
    COLORREF textColor = CLR_WHITE;

    if (b->highlight)
    {
        fill = CLR_BTN_ACTIVE;
        textColor = CLR_DARK_TEXT;
    }
    else if (b->id == hoverId)
    {
        fill = CLR_BTN_HOVER;
    }

    fillRounded(hdc, b->x, b->y, b->x + b->w, b->y + b->h, 16,
                fill, CLR_BTN_BORDER, 2);
    drawTextBox(hdc, b->label, b->x, b->y, b->x + b->w, b->y + b->h,
                22, 1, textColor);
}

/* Draws all the buttons of the current screen */
static void drawButtons(HDC hdc)
{
    Button list[4];
    int count = getButtons(list);
    int i;

    for (i = 0; i < count; i++)
    {
        drawButton(hdc, &list[i]);
    }
}

/* Returns the ID of the button at pixel (x, y), or BTN_NONE */
static int buttonAt(int x, int y)
{
    Button list[4];
    int count = getButtons(list);
    int i;

    for (i = 0; i < count; i++)
    {
        if (x >= list[i].x && x < list[i].x + list[i].w &&
            y >= list[i].y && y < list[i].y + list[i].h)
        {
            return list[i].id;
        }
    }
    return BTN_NONE;
}

/* Does what a button says. Used by both mouse clicks and keyboard
 * shortcuts, so a click and a key press always behave the same. */
static void doAction(int id)
{
    switch (id)
    {
        case BTN_START:
        case BTN_PLAY_AGAIN:
            startGame();
            break;

        case BTN_DIFFICULTY:
            gameState = STATE_DIFFICULTY;
            break;

        case BTN_HIGHSCORE:
            gameState = STATE_HIGHSCORE;
            break;

        case BTN_EXIT:
            DestroyWindow(hwndMain);
            break;

        case BTN_EASY:
            setDifficulty(EASY);
            gameState = STATE_MENU;
            break;

        case BTN_MEDIUM:
            setDifficulty(MEDIUM);
            gameState = STATE_MENU;
            break;

        case BTN_HARD:
            setDifficulty(HARD);
            gameState = STATE_MENU;
            break;

        case BTN_BACK:
        case BTN_MENU:
            quitToMenu();
            break;

        case BTN_RESUME:
            pauseGame();
            break;

        default:
            break;
    }

    hoverId = BTN_NONE;     /* the screen changed, forget the old hover */
    requestRedraw();
}

/* ---------------------------------------------------------------
 * drawBoard()
 * Draws the grass (two shades like a chess board) and the walls.
 * --------------------------------------------------------------- */
void drawBoard(HDC hdc)
{
    int x, y;
    int left, top;

    for (y = 0; y < HEIGHT; y++)
    {
        for (x = 0; x < WIDTH; x++)
        {
            left = x * CELL;
            top = HUD_HEIGHT + y * CELL;

            if (x == 0 || y == 0 || x == WIDTH - 1 || y == HEIGHT - 1)
            {
                /* wall cell: dark block with a lighter square inside */
                fillRect(hdc, left, top, left + CELL, top + CELL, CLR_WALL);
                fillRect(hdc, left + 3, top + 3, left + CELL - 3, top + CELL - 3,
                         CLR_WALL_LIGHT);
            }
            else if ((x + y) % 2 == 0)
            {
                fillRect(hdc, left, top, left + CELL, top + CELL, CLR_GRASS_A);
            }
            else
            {
                fillRect(hdc, left, top, left + CELL, top + CELL, CLR_GRASS_B);
            }
        }
    }
}

/* ---------------------------------------------------------------
 * drawSnake()
 * Draws the body as rounded blue blocks and the head with two eyes
 * that look in the direction the snake is moving.
 * --------------------------------------------------------------- */
void drawSnake(HDC hdc)
{
    int i;
    int left, top;
    int cx, cy;
    int ex1, ey1, ex2, ey2;     /* eye positions */

    /* body (from the tail up to, but not including, the head) */
    for (i = length - 1; i >= 1; i--)
    {
        left = snakeX[i] * CELL;
        top = HUD_HEIGHT + snakeY[i] * CELL;
        fillRounded(hdc, left + 2, top + 2, left + CELL - 2, top + CELL - 2,
                    10, CLR_SNAKE_BODY, CLR_SNAKE_BODY, 1);
    }

    /* head */
    left = snakeX[0] * CELL;
    top = HUD_HEIGHT + snakeY[0] * CELL;
    fillRounded(hdc, left + 1, top + 1, left + CELL - 1, top + CELL - 1,
                12, CLR_SNAKE_HEAD, CLR_SNAKE_HEAD, 1);

    /* eyes: placed on the side the snake is facing */
    cx = left + CELL / 2;
    cy = top + CELL / 2;

    switch (direction)
    {
        case DIR_UP:
            ex1 = cx - 5; ey1 = cy - 4;
            ex2 = cx + 5; ey2 = cy - 4;
            break;

        case DIR_DOWN:
            ex1 = cx - 5; ey1 = cy + 4;
            ex2 = cx + 5; ey2 = cy + 4;
            break;

        case DIR_LEFT:
            ex1 = cx - 4; ey1 = cy - 5;
            ex2 = cx - 4; ey2 = cy + 5;
            break;

        default:    /* DIR_RIGHT */
            ex1 = cx + 4; ey1 = cy - 5;
            ex2 = cx + 4; ey2 = cy + 5;
            break;
    }

    fillCircle(hdc, ex1, ey1, 4, CLR_WHITE);
    fillCircle(hdc, ex2, ey2, 4, CLR_WHITE);
    fillCircle(hdc, ex1, ey1, 2, CLR_BLACK);
    fillCircle(hdc, ex2, ey2, 2, CLR_BLACK);
}

/* ---------------------------------------------------------------
 * drawFood()
 * Draws the apple: a red circle with a small green leaf.
 * --------------------------------------------------------------- */
void drawFood(HDC hdc)
{
    int left = foodX * CELL;
    int top = HUD_HEIGHT + foodY * CELL;
    int cx = left + CELL / 2;
    int cy = top + CELL / 2 + 1;

    fillCircle(hdc, cx, cy, CELL / 2 - 3, CLR_FOOD);
    fillCircle(hdc, cx + 3, top + 5, 3, CLR_LEAF);
}

/* ---------------------------------------------------------------
 * drawScore()
 * Draws the bar at the top: score on the left, difficulty in the
 * middle, high score on the right.
 * --------------------------------------------------------------- */
void drawScore(HDC hdc)
{
    char text[64];
    RECT rc;

    fillRect(hdc, 0, 0, WINDOW_W, HUD_HEIGHT, CLR_HUD);

    sprintf(text, "Score: %d", score);
    rc = makeRect(20, 0, 260, HUD_HEIGHT);
    drawTextIn(hdc, text, &rc, 24, 1, CLR_WHITE,
               DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    sprintf(text, "Level: %s", getDifficultyName());
    rc = makeRect(260, 0, 460, HUD_HEIGHT);
    drawTextIn(hdc, text, &rc, 20, 0, CLR_GRAY,
               DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    sprintf(text, "High Score: %d", highScore);
    rc = makeRect(460, 0, WINDOW_W - 20, HUD_HEIGHT);
    drawTextIn(hdc, text, &rc, 24, 1, CLR_YELLOW,
               DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
}

/* ---------------------------------------------------------------
 * drawMainMenu()
 * Title, four buttons and a small decoration.
 * --------------------------------------------------------------- */
void drawMainMenu(HDC hdc)
{
    drawMenuBackground(hdc);
    drawTitle(hdc, "SNAKE GAME", "by Mohib, Aarish and Hasan");
    drawButtons(hdc);
    drawDecoration(hdc);
    drawTextBox(hdc, "Click a button or press 1 - 4", 0, 490, WINDOW_W, 520,
                16, 0, CLR_GRAY);
}

/* ---------------------------------------------------------------
 * drawDifficultyScreen()
 * Three level buttons (the current one is yellow) and a Back button.
 * --------------------------------------------------------------- */
void drawDifficultyScreen(HDC hdc)
{
    drawMenuBackground(hdc);
    drawTitle(hdc, "DIFFICULTY", "Choose how fast the snake moves");
    drawButtons(hdc);
    drawTextBox(hdc, "Press 1, 2 or 3  |  ESC to go back", 0, 490, WINDOW_W, 520,
                16, 0, CLR_GRAY);
}

/* ---------------------------------------------------------------
 * drawHighScoreScreen()
 * Shows the best score in big numbers.
 * --------------------------------------------------------------- */
void drawHighScoreScreen(HDC hdc)
{
    char text[32];

    drawMenuBackground(hdc);
    drawTitle(hdc, "HIGH SCORE", "Your best result so far");

    sprintf(text, "%d", highScore);
    drawTextBox(hdc, text, 0, 175, WINDOW_W, 320, 110, 1, CLR_WHITE);

    drawButtons(hdc);
    drawTextBox(hdc, "Press ESC to go back", 0, 490, WINDOW_W, 520,
                16, 0, CLR_GRAY);
}

/* ---------------------------------------------------------------
 * drawPauseScreen()
 * A box in the middle of the game with Resume and Main Menu.
 * --------------------------------------------------------------- */
void drawPauseScreen(HDC hdc)
{
    drawPanel(hdc);
    drawTextBox(hdc, "PAUSED", 150, 140, 570, 220, 56, 1, CLR_YELLOW);
    drawTextBox(hdc, "Press P to resume", 150, 230, 570, 270, 20, 0, CLR_GRAY);
    drawButtons(hdc);
}

/* ---------------------------------------------------------------
 * drawGameOverScreen()
 * A box with the final score, the high score and two buttons.
 * --------------------------------------------------------------- */
void drawGameOverScreen(HDC hdc)
{
    char text[64];

    drawPanel(hdc);
    drawTextBox(hdc, "GAME OVER", 150, 115, 570, 185, 52, 1, CLR_RED_TEXT);

    sprintf(text, "Your score: %d", score);
    drawTextBox(hdc, text, 150, 190, 570, 225, 24, 1, CLR_WHITE);

    sprintf(text, "High score: %d", highScore);
    drawTextBox(hdc, text, 150, 225, 570, 255, 20, 0, CLR_GRAY);

    if (score > 0 && score == highScore)
    {
        drawTextBox(hdc, "NEW HIGH SCORE!", 150, 260, 570, 295, 24, 1, CLR_YELLOW);
    }

    drawButtons(hdc);
}

/* ---------------------------------------------------------------
 * drawFrame()
 * Called by main.c every time the window must be drawn.
 * Draws the screen that belongs to the current gameState.
 * --------------------------------------------------------------- */
void drawFrame(HDC hdc)
{
    switch (gameState)
    {
        case STATE_MENU:
            drawMainMenu(hdc);
            break;

        case STATE_DIFFICULTY:
            drawDifficultyScreen(hdc);
            break;

        case STATE_HIGHSCORE:
            drawHighScoreScreen(hdc);
            break;

        default:
            /* playing, paused and game over all show the game itself */
            drawScore(hdc);
            drawBoard(hdc);
            drawFood(hdc);
            drawSnake(hdc);

            if (gameState == STATE_PAUSED)
            {
                drawPauseScreen(hdc);
            }
            else if (gameState == STATE_GAMEOVER)
            {
                drawGameOverScreen(hdc);
            }
            break;
    }
}

/* ---------------------------------------------------------------
 * handleKeyDown()
 * Called when a key is pressed. What a key does depends on the
 * current screen:
 *   Menu        : 1 Start, 2 Difficulty, 3 High Score, 4 or ESC Exit
 *   Difficulty  : 1 Easy, 2 Medium, 3 Hard, ESC back
 *   High score  : ESC / Enter back
 *   Playing     : arrow keys steer, P pauses, ESC goes to the menu
 *   Paused      : P resumes, ESC goes to the menu
 *   Game over   : Y / Enter play again, N / ESC menu
 * --------------------------------------------------------------- */
void handleKeyDown(int key)
{
    switch (gameState)
    {
        case STATE_MENU:
            if (key == '1' || key == VK_RETURN)
            {
                doAction(BTN_START);
            }
            else if (key == '2')
            {
                doAction(BTN_DIFFICULTY);
            }
            else if (key == '3')
            {
                doAction(BTN_HIGHSCORE);
            }
            else if (key == '4' || key == VK_ESCAPE)
            {
                doAction(BTN_EXIT);
            }
            break;

        case STATE_DIFFICULTY:
            if (key == '1')
            {
                doAction(BTN_EASY);
            }
            else if (key == '2')
            {
                doAction(BTN_MEDIUM);
            }
            else if (key == '3')
            {
                doAction(BTN_HARD);
            }
            else if (key == VK_ESCAPE)
            {
                doAction(BTN_BACK);
            }
            break;

        case STATE_HIGHSCORE:
            if (key == VK_ESCAPE || key == VK_RETURN || key == VK_SPACE)
            {
                doAction(BTN_BACK);
            }
            break;

        case STATE_PLAYING:
            /* The snake may not turn directly back on itself, so each
             * new direction is compared with 'direction' (the direction
             * of the last move). The new choice is stored in
             * nextDirection and used by the next gameTick(). */
            if (key == VK_UP && direction != DIR_DOWN)
            {
                nextDirection = DIR_UP;
            }
            else if (key == VK_DOWN && direction != DIR_UP)
            {
                nextDirection = DIR_DOWN;
            }
            else if (key == VK_LEFT && direction != DIR_RIGHT)
            {
                nextDirection = DIR_LEFT;
            }
            else if (key == VK_RIGHT && direction != DIR_LEFT)
            {
                nextDirection = DIR_RIGHT;
            }
            else if (key == 'P')
            {
                doAction(BTN_RESUME);       /* pauseGame() toggles */
            }
            else if (key == VK_ESCAPE)
            {
                doAction(BTN_MENU);
            }
            break;

        case STATE_PAUSED:
            if (key == 'P')
            {
                doAction(BTN_RESUME);
            }
            else if (key == VK_ESCAPE)
            {
                doAction(BTN_MENU);
            }
            break;

        case STATE_GAMEOVER:
            if (key == 'Y' || key == VK_RETURN)
            {
                doAction(BTN_PLAY_AGAIN);
            }
            else if (key == 'N' || key == VK_ESCAPE)
            {
                doAction(BTN_MENU);
            }
            break;

        default:
            break;
    }
}

/* ---------------------------------------------------------------
 * handleMouseClick()
 * Called when the left mouse button is pressed at pixel (x, y).
 * If a button is under the mouse, do its action.
 * --------------------------------------------------------------- */
void handleMouseClick(int x, int y)
{
    int id = buttonAt(x, y);

    if (id != BTN_NONE)
    {
        doAction(id);
    }
}

/* ---------------------------------------------------------------
 * handleMouseMove()
 * Called when the mouse moves. Remembers which button the mouse is
 * over so drawButton() can make it lighter.
 * --------------------------------------------------------------- */
void handleMouseMove(int x, int y)
{
    int id = buttonAt(x, y);

    if (id != hoverId)
    {
        hoverId = id;
        requestRedraw();
    }
}
