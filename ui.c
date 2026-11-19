#include "game.h"

#define CLR_GRASS_A     RGB(170, 215, 81)
#define CLR_GRASS_B     RGB(162, 209, 73)
#define CLR_WALL        RGB(74, 117, 44)
#define CLR_HUD         RGB(55, 90, 35)
#define CLR_BODY        RGB(78, 124, 246)
#define CLR_HEAD        RGB(52, 95, 214)
#define CLR_FOOD        RGB(231, 71, 29)
#define CLR_DARK        RGB(28, 45, 32)
#define CLR_WHITE       RGB(255, 255, 255)
#define CLR_YELLOW      RGB(255, 206, 84)
#define CLR_GRAY        RGB(170, 185, 170)
#define CLR_RED         RGB(255, 99, 71)
#define CLR_BTN         RGB(67, 160, 71)
#define CLR_BTN_HOT     RGB(102, 187, 106)

#define SHAPE_BOX       0
#define SHAPE_ROUND     1
#define SHAPE_CIRCLE    2

#define ALIGN_CENTER    (DT_CENTER | DT_VCENTER | DT_SINGLELINE)
#define REDRAW()        InvalidateRect(hwndMain, NULL, FALSE)

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

#define BTN_W           260
#define BTN_H           50
#define BTN_X           ((WINDOW_W - BTN_W) / 2)

typedef struct{
    int state,id,y;
    const char *label;
} Button;

static const Button buttons[] ={
    {STATE_MENU,       190, "Start Game", BTN_START},
    {STATE_MENU,       255, "Difficulty", BTN_DIFFICULTY},
    {STATE_MENU,       320, "High Score", BTN_HIGHSCORE},
    {STATE_MENU,       385, "Exit",       BTN_EXIT},
    {STATE_DIFFICULTY, 190, "Easy",       BTN_EASY},
    {STATE_DIFFICULTY, 255, "Medium",     BTN_MEDIUM},
    {STATE_DIFFICULTY, 320, "Hard",       BTN_HARD},
    {STATE_DIFFICULTY, 385, "Back",       BTN_BACK},
    {STATE_HIGHSCORE,  385, "Back",       BTN_BACK},
    {STATE_PAUSED,     320, "Resume",     BTN_RESUME},
    {STATE_PAUSED,     385, "Main Menu",  BTN_MENU},
    {STATE_GAMEOVER,   320, "Play Again", BTN_PLAY_AGAIN},
    {STATE_GAMEOVER,   385, "Main Menu",  BTN_MENU}
};

#define BUTTON_COUNT    ((int)(sizeof(buttons) / sizeof(buttons[0])))

static const char *levelName[] = {"", "Easy", "Medium", "Hard"};
static int hoverId = BTN_NONE;
static void shape(HDC hdc, int kind, int l, int t, int r, int b, COLORREF color){
    HBRUSH brush = CreateSolidBrush(color);
    HPEN pen = CreatePen(PS_SOLID, 1, color);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);
    if (kind == SHAPE_ROUND)
        RoundRect(hdc, l, t, r, b, 14, 14);
    else if (kind == SHAPE_CIRCLE)
        Ellipse(hdc, l, t, r, b);
    else
        Rectangle(hdc, l, t, r, b);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(brush);
    DeleteObject(pen);
}

static void drawText(HDC hdc, const char *text, int l, int t, int r, int b,
                     int size, COLORREF color, UINT format){
    RECT rc;
    HFONT font = CreateFontA(-size, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                             CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
                             DEFAULT_PITCH | FF_SWISS, "Segoe UI");
    HFONT oldFont = (HFONT)SelectObject(hdc, font);
    rc.left = l;
    rc.top = t;
    rc.right = r;
    rc.bottom = b;
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, color);
    DrawTextA(hdc, text, -1, &rc, format);
    SelectObject(hdc, oldFont);
    DeleteObject(font);
}

static void drawButtons(HDC hdc){
    int i;
    for (i = 0; i < BUTTON_COUNT; i++){
        if (buttons[i].state != gameState)
            continue;
        shape(hdc, SHAPE_ROUND, BTN_X, buttons[i].y, BTN_X + BTN_W, buttons[i].y + BTN_H,
              buttons[i].id == hoverId ? CLR_BTN_HOT : CLR_BTN);
        drawText(hdc, buttons[i].label, BTN_X, buttons[i].y, BTN_X + BTN_W,
                 buttons[i].y + BTN_H, 22, CLR_WHITE, ALIGN_CENTER);
    }
}

static int buttonAt(int x, int y){
    int i;
    for (i = 0; i < BUTTON_COUNT; i++){
        if (buttons[i].state == gameState &&
            x >= BTN_X && x < BTN_X + BTN_W &&
            y >= buttons[i].y && y < buttons[i].y + BTN_H)
            return buttons[i].id;
    }
    return BTN_NONE;
}

static void doAction(int id){
    switch (id){
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
        case BTN_MEDIUM:
        case BTN_HARD:
            setDifficulty(id - BTN_EASY + 1);
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
    hoverId = BTN_NONE;
    REDRAW();
}

static void drawMenuScreen(HDC hdc, const char *title, const char *subtitle){
    shape(hdc, SHAPE_BOX, 0, 0, WINDOW_W, WINDOW_H, CLR_DARK);
    drawText(hdc, title, 0, 35, WINDOW_W, 115, 60, CLR_YELLOW, ALIGN_CENTER);
    drawText(hdc, subtitle, 0, 115, WINDOW_W, 150, 18, CLR_GRAY, ALIGN_CENTER);
    drawButtons(hdc);
}

static void drawBox(HDC hdc, const char *title, int size, COLORREF color){
    shape(hdc, SHAPE_ROUND, 150, 100, 570, 450, CLR_YELLOW);
    shape(hdc, SHAPE_ROUND, 153, 103, 567, 447, CLR_DARK);
    drawText(hdc, title, 150, 120, 570, 200, size, color, ALIGN_CENTER);
    drawButtons(hdc);
}

void drawBoard(HDC hdc){
    int x, y;
    COLORREF color;
    for (y = 0; y < HEIGHT; y++){
        for (x = 0; x < WIDTH; x++){
            color = ((x + y) % 2) ? CLR_GRASS_B : CLR_GRASS_A;
            if (x == 0 || y == 0 || x == WIDTH - 1 || y == HEIGHT - 1)
                color = CLR_WALL;
            shape(hdc, SHAPE_BOX, x * CELL, HUD_HEIGHT + y * CELL,
                  (x + 1) * CELL, HUD_HEIGHT + (y + 1) * CELL, color);
        }
    }
}

void drawSnake(HDC hdc){
    int i;
    for (i = length - 1; i >= 0; i--)
        shape(hdc, SHAPE_ROUND, snakeX[i] * CELL + 1, HUD_HEIGHT + snakeY[i] * CELL + 1,
              (snakeX[i] + 1) * CELL - 1, HUD_HEIGHT + (snakeY[i] + 1) * CELL - 1,
              i == 0 ? CLR_HEAD : CLR_BODY);
}

void drawFood(HDC hdc){
    shape(hdc, SHAPE_CIRCLE, foodX * CELL + 3, HUD_HEIGHT + foodY * CELL + 3,
          (foodX + 1) * CELL - 3, HUD_HEIGHT + (foodY + 1) * CELL - 3, CLR_FOOD);
}

void drawScore(HDC hdc){
    char text[64];
    shape(hdc, SHAPE_BOX, 0, 0, WINDOW_W, HUD_HEIGHT, CLR_HUD);
    sprintf(text, "Score: %d", score);
    drawText(hdc, text, 20, 0, 260, HUD_HEIGHT, 24, CLR_WHITE,
             DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    sprintf(text, "Level: %s", levelName[difficulty]);
    drawText(hdc, text, 260, 0, 460, HUD_HEIGHT, 20, CLR_GRAY, ALIGN_CENTER);
    sprintf(text, "High Score: %d", highScore);
    drawText(hdc, text, 460, 0, WINDOW_W - 20, HUD_HEIGHT, 24, CLR_YELLOW,
             DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
}

void drawMainMenu(HDC hdc){
    drawMenuScreen(hdc, "SNAKE GAME", "by Mohib, Aarish and Hasan");
}

void drawDifficultyScreen(HDC hdc){
    char text[32];
    sprintf(text, "Current level: %s", levelName[difficulty]);
    drawMenuScreen(hdc, "DIFFICULTY", text);
}

void drawHighScoreScreen(HDC hdc){
    char text[32];
    drawMenuScreen(hdc, "HIGH SCORE", "Your best result so far");
    sprintf(text, "%d", highScore);
    drawText(hdc, text, 0, 175, WINDOW_W, 320, 110, CLR_WHITE, ALIGN_CENTER);
}

void drawPauseScreen(HDC hdc){
    drawBox(hdc, "PAUSED", 56, CLR_YELLOW);
    drawText(hdc, "Press P to resume", 150, 230, 570, 270, 20, CLR_GRAY, ALIGN_CENTER);
}

void drawGameOverScreen(HDC hdc){
    char text[64];
    drawBox(hdc, "GAME OVER", 52, CLR_RED);
    sprintf(text, "Your score: %d", score);
    drawText(hdc, text, 150, 205, 570, 235, 24, CLR_WHITE, ALIGN_CENTER);
    sprintf(text, "High score: %d", highScore);
    drawText(hdc, text, 150, 235, 570, 265, 20, CLR_GRAY, ALIGN_CENTER);
    if (score > 0 && score == highScore)
        drawText(hdc, "NEW HIGH SCORE!", 150, 270, 570, 305, 24, CLR_YELLOW, ALIGN_CENTER);
}

void drawFrame(HDC hdc){
    if (gameState == STATE_MENU)
        drawMainMenu(hdc);
    else if (gameState == STATE_DIFFICULTY)
        drawDifficultyScreen(hdc);
    else if (gameState == STATE_HIGHSCORE)
        drawHighScoreScreen(hdc);
    else{
        drawScore(hdc);
        drawBoard(hdc);
        drawFood(hdc);
        drawSnake(hdc);
        if (gameState == STATE_PAUSED)
            drawPauseScreen(hdc);
        else if (gameState == STATE_GAMEOVER)
            drawGameOverScreen(hdc);
    }
}
void handleKeyDown(int key){
    switch (gameState){
        case STATE_MENU:
            if (key >= '1' && key <= '4')
                doAction(key - '0');
            else if (key == VK_ESCAPE)
                doAction(BTN_EXIT);
            break;

        case STATE_DIFFICULTY:
            if (key >= '1' && key <= '3')
                doAction(BTN_EASY + key - '1');
            else if (key == VK_ESCAPE)
                doAction(BTN_BACK);
            break;

        case STATE_HIGHSCORE:
            if (key == VK_ESCAPE || key == VK_RETURN)
                doAction(BTN_BACK);
            break;

        case STATE_PLAYING:
        case STATE_PAUSED:
            if (gameState == STATE_PLAYING){
                if (key == VK_UP && direction != DIR_DOWN)
                    nextDirection = DIR_UP;
                else if (key == VK_DOWN && direction != DIR_UP)
                    nextDirection = DIR_DOWN;
                else if (key == VK_LEFT && direction != DIR_RIGHT)
                    nextDirection = DIR_LEFT;
                else if (key == VK_RIGHT && direction != DIR_LEFT)
                    nextDirection = DIR_RIGHT;
            }
            if (key == 'P')
                doAction(BTN_RESUME);
            else if (key == VK_ESCAPE)
                doAction(BTN_MENU);
            break;

        case STATE_GAMEOVER:
            if (key == 'Y' || key == VK_RETURN)
                doAction(BTN_PLAY_AGAIN);
            else if (key == 'N' || key == VK_ESCAPE)
                doAction(BTN_MENU);
            break;

        default:
            break;
    }
}
void handleMouseClick(int x, int y){
    int id = buttonAt(x, y);
    if (id != BTN_NONE)
        doAction(id);
}
void handleMouseMove(int x, int y){
    int id = buttonAt(x, y);
    if (id != hoverId){
        hoverId = id;
        REDRAW();
    }
}
