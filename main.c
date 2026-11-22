#include "game.h"

/* ---------- the shared variables (created here, only declared in game.h) ---------- */
int snakeX[MAX_LENGTH];
int snakeY[MAX_LENGTH];
int length = START_LENGTH;
int direction = DIR_RIGHT;
int nextDirection = DIR_RIGHT;

int foodX = 0;
int foodY = 0;

int score = 0;
int highScore = 0;

int difficulty = MEDIUM;
int speed = SPEED_MEDIUM;
int gameOver = 0;
int paused = 0;
int gameState = STATE_MENU;

HWND hwndMain = NULL;

/* Windows calls this function for every event of our window.
   msg tells which event it is. */
static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_PAINT)                      /* the window must be drawn */
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        /* draw on a hidden picture first, then show it in one step (no flicker) */
        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP bitmap = CreateCompatibleBitmap(hdc, WINDOW_W, WINDOW_H);
        HBITMAP old = (HBITMAP)SelectObject(memDC, bitmap);

        drawFrame(memDC);
        BitBlt(hdc, 0, 0, WINDOW_W, WINDOW_H, memDC, 0, 0, SRCCOPY);

        SelectObject(memDC, old);
        DeleteObject(bitmap);
        DeleteDC(memDC);
        EndPaint(hwnd, &ps);
        return 0;
    }
    if (msg == WM_ERASEBKGND)                 /* we paint everything ourselves */
        return 1;
    if (msg == WM_TIMER && gameState == STATE_PLAYING)   /* time for one snake step */
    {
        gameTick();
        InvalidateRect(hwnd, NULL, FALSE);    /* ask for a redraw */
        return 0;
    }
    if (msg == WM_KEYDOWN)                    /* a key was pressed */
    {
        handleKeyDown((int)wParam);
        return 0;
    }
    if (msg == WM_LBUTTONDOWN)                /* left mouse button clicked */
    {
        handleMouseClick(LOWORD(lParam), HIWORD(lParam));
        return 0;
    }
    if (msg == WM_MOUSEMOVE)                  /* the mouse moved */
    {
        handleMouseMove(LOWORD(lParam), HIWORD(lParam));
        return 0;
    }
    if (msg == WM_DESTROY)                    /* the window was closed */
    {
        updateHighScore();
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);   /* Windows does the rest */
}

/* The program starts here: create the window, then handle events until it is closed. */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR cmdLine, int nCmdShow)
{
    WNDCLASSA wc;
    MSG msg;
    DWORD style = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    RECT size = {0, 0, WINDOW_W, WINDOW_H};

    /* describe our kind of window and register it */
    memset(&wc, 0, sizeof(wc));
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.lpszClassName = "SnakeGame";
    RegisterClassA(&wc);

    /* make the window big enough so that the drawing area is WINDOW_W x WINDOW_H */
    AdjustWindowRect(&size, style, FALSE);
    hwndMain = CreateWindowA("SnakeGame", "Snake Game", style,
                             CW_USEDEFAULT, CW_USEDEFAULT,
                             size.right - size.left, size.bottom - size.top,
                             NULL, NULL, hInstance, NULL);
    if (hwndMain == NULL)
        return 1;

    loadHighScore();
    ShowWindow(hwndMain, nCmdShow);

    /* wait for an event, give it to WndProc, repeat until the window closes */
    while (GetMessageA(&msg, NULL, 0, 0))
        DispatchMessageA(&msg);

    return 0;
}
