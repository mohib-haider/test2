/*
 * main.c - Program entry point, window and global variables
 *
 * Owner: Mohib
 *
 * This file does three things:
 *   1. DEFINES the global variables shared by all files
 *      (game.h only declares them with "extern").
 *   2. WndProc(): receives every event of the window
 *      (paint, timer tick, key press, mouse click).
 *   3. WinMain(): creates the window and runs the message loop.
 *
 * Compile all files together (see README.md):
 *   gcc main.c snake.c ui.c food_score.c -o snake.exe -mwindows -lgdi32
 */

#include "game.h"

/* ---------------------------------------------------------------
 * GLOBAL VARIABLES (the only place where they are defined)
 * --------------------------------------------------------------- */

/* Snake: index 0 is the head, index (length - 1) is the tail */
int snakeX[MAX_LENGTH];
int snakeY[MAX_LENGTH];
int length = START_LENGTH;
int direction = DIR_RIGHT;
int nextDirection = DIR_RIGHT;

/* Food position */
int foodX = 0;
int foodY = 0;

/* Score */
int score = 0;
int highScore = 0;

/* Game settings and state */
int difficulty = MEDIUM;
int speed = SPEED_MEDIUM;
int gameOver = 0;
int paused = 0;
int gameState = STATE_MENU;

/* Tail position left by the last move */
int oldTailX = 0;
int oldTailY = 0;

/* The game window */
HWND hwndMain = NULL;

/* ---------------------------------------------------------------
 * WndProc()
 * Windows calls this function every time something happens to
 * our window. We decide what to do for each type of message.
 * --------------------------------------------------------------- */
static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_PAINT:
        {
            /* Draw everything on a hidden bitmap first, then copy it to
             * the window in one go. This stops the screen from flickering. */
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rc;
            int w, h;
            HDC memDC;
            HBITMAP bitmap;
            HBITMAP oldBitmap;

            GetClientRect(hwnd, &rc);
            w = rc.right;
            h = rc.bottom;

            memDC = CreateCompatibleDC(hdc);
            bitmap = CreateCompatibleBitmap(hdc, w, h);
            oldBitmap = (HBITMAP)SelectObject(memDC, bitmap);

            drawFrame(memDC);                           /* ui.c */

            BitBlt(hdc, 0, 0, w, h, memDC, 0, 0, SRCCOPY);

            SelectObject(memDC, oldBitmap);
            DeleteObject(bitmap);
            DeleteDC(memDC);
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_ERASEBKGND:
            return 1;           /* we paint the whole window ourselves */

        case WM_TIMER:
            /* The timer ticks once per snake move */
            if (wParam == GAME_TIMER_ID && gameState == STATE_PLAYING)
            {
                gameTick();                             /* snake.c */
                InvalidateRect(hwnd, NULL, FALSE);      /* ask for a redraw */
            }
            return 0;

        case WM_KEYDOWN:
            handleKeyDown((int)wParam);                 /* ui.c */
            return 0;

        case WM_LBUTTONDOWN:
            handleMouseClick((short)LOWORD(lParam), (short)HIWORD(lParam));
            return 0;

        case WM_MOUSEMOVE:
            handleMouseMove((short)LOWORD(lParam), (short)HIWORD(lParam));
            return 0;

        case WM_DESTROY:
            KillTimer(hwnd, GAME_TIMER_ID);
            PostQuitMessage(0);
            return 0;

        default:
            break;
    }

    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

/* ---------------------------------------------------------------
 * WinMain()
 * The starting point of a Windows GUI program.
 * Creates the window, then keeps processing messages until
 * the window is closed.
 * --------------------------------------------------------------- */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSA wc;
    MSG msg;
    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    RECT rect;

    (void)hPrevInstance;    /* not used */
    (void)lpCmdLine;        /* not used */

    /* describe our window type */
    memset(&wc, 0, sizeof(wc));
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszClassName = "SnakeGameWindow";

    if (!RegisterClassA(&wc))
    {
        MessageBoxA(NULL, "Could not register the window.", "Error", MB_OK);
        return 1;
    }

    /* We want a drawing area of exactly WINDOW_W x WINDOW_H pixels.
     * AdjustWindowRect adds the space for the title bar and borders. */
    rect.left = 0;
    rect.top = 0;
    rect.right = WINDOW_W;
    rect.bottom = WINDOW_H;
    AdjustWindowRect(&rect, style, FALSE);

    hwndMain = CreateWindowA("SnakeGameWindow", "Snake Game", style,
                             CW_USEDEFAULT, CW_USEDEFAULT,
                             rect.right - rect.left, rect.bottom - rect.top,
                             NULL, NULL, hInstance, NULL);
    if (hwndMain == NULL)
    {
        MessageBoxA(NULL, "Could not create the window.", "Error", MB_OK);
        return 1;
    }

    loadHighScore();                /* read the best score from the file */

    ShowWindow(hwndMain, nCmdShow);
    UpdateWindow(hwndMain);

    /* message loop: runs until the window is closed */
    while (GetMessageA(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    return (int)msg.wParam;
}
