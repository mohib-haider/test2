# Snake Game in C (Windows GUI)

A classic 2D Snake game with a native graphical user interface, written in C using the **Windows API (Win32 & GDI)**. No external third-party graphics libraries are required.

## Table of Contents
1. [Features](#features)
2. [How to Play & Controls](#how-to-play--controls)
3. [Project Architecture & File Breakdown](#project-architecture--file-breakdown)
   - [`game.h`](#1-gameh)
   - [`main.c`](#2-mainc)
   - [`snake.c`](#3-snakec)
   - [`ui.c`](#4-uic)
   - [`food_score.c`](#5-food_scorec)
4. [How It Works](#how-it-works)
5. [Compilation & Execution](#compilation--execution)
6. [Team Contributions](#team-contributions)

---

## Features
- **Native Windows GUI:** Smooth, flicker-free rendering using Win32 GDI double buffering (`CreateCompatibleDC` and `BitBlt`).
- **Multiple Game Screens:** Main Menu, Difficulty Selection, Active Gameplay, Pause Overlay, Game Over Overlay, and High Score Screen.
- **Full Mouse & Keyboard Support:** Interactive buttons with hover highlights as well as keyboard shortcuts.
- **Three Difficulty Levels:** Easy, Medium, and Hard, dynamically adjusting the snake's movement speed.
- **Persistent High Scores:** Automatically reads and saves the highest score to `highscore.txt`.

---

## How to Play & Controls

| Input | Action |
| :--- | :--- |
| **Arrow Keys** ($\uparrow, \downarrow, \leftarrow, \rightarrow$) | Steer the snake (prevents immediate 180-degree reversal) |
| **`P`** | Pause or resume the game |
| **`ESC`** | Return to the Main Menu (or exit from the Main Menu) |
| **Mouse Click** | Click any on-screen menu button |
| **`1` – `4`** | Quick-select buttons on the Main Menu and Difficulty Screen |
| **`Y` / `Enter`** | Play Again on the Game Over screen |
| **`N`** | Return to Main Menu on the Game Over screen |

### Scoring & Difficulty
- **Points per Apple:** `10` points
- **Difficulty Speeds (Timer Interval):**
  - **Easy:** `150 ms` per step
  - **Medium:** `100 ms` per step *(Default)*
  - **Hard:** `60 ms` per step

---

## Project Architecture & File Breakdown

The project is modularized into one shared header file and four C source files:

### 1. `game.h`
The central header file shared across all modules.
- **Macros & Constants:** Defines grid dimensions (`WIDTH = 30`, `HEIGHT = 20`), cell pixel size (`CELL = 24`), HUD height (`HUD_HEIGHT = 50`), window dimensions (`720 x 530`), directions (`DIR_*`), difficulty levels, timer speeds, and screen states (`STATE_*`).
- **Global Declarations (`extern`):** Declares shared game state variables (`snakeX`, `snakeY`, `length`, `score`, `highScore`, `gameState`, `hwndMain`, etc.) so all `.c` files can access them without duplicate definitions.
- **Function Prototypes:** Declares all public functions across `snake.c`, `ui.c`, and `food_score.c`.

### 2. `main.c`
Handles application entry, global variable definitions, window creation, and the Windows event loop.
- **Global Definitions:** Instantiates all shared game variables declared in `game.h`.
- **`WinMain()`:** Registers the `"SnakeGame"` window class, calculates exact client dimensions via `AdjustWindowRect()`, creates the main window, loads the saved high score, and runs the Win32 message loop (`GetMessageA` / `DispatchMessageA`).
- **`WndProc()`:** Processes Windows system messages:
  - `WM_PAINT`: Performs double-buffered rendering by drawing to an off-screen memory bitmap and copying it to the screen in a single `BitBlt` call.
  - `WM_ERASEBKGND`: Returns `1` to prevent background flicker.
  - `WM_TIMER`: Triggers `gameTick()` and requests a screen redraw (`InvalidateRect`) whenever `gameState == STATE_PLAYING`.
  - `WM_KEYDOWN`, `WM_LBUTTONDOWN`, `WM_MOUSEMOVE`: Forwards user inputs to `ui.c`.
  - `WM_DESTROY`: Saves any new high score and cleanly exits the application.

### 3. `snake.c`
Manages core snake mechanics, collision detection, and game lifecycle transitions.
- **`initSnake()`:** Places the initial 3-segment snake in the center of the board facing right.
- **`moveSnake()`:** Shifts body coordinates backward along `snakeX[]` and `snakeY[]` and advances the head (`index 0`) by one cell in the current `direction`.
- **`growSnake()`:** Increments `length` up to `MAX_LENGTH - 1` when an apple is eaten.
- **`checkWallCollision()`:** Returns `1` if the snake's head hits the outer border walls (`x <= 0`, `x >= WIDTH - 1`, `y <= 0`, or `y >= HEIGHT - 1`).
- **`checkSelfCollision()`:** Returns `1` if the snake's head coordinates match any of its body segments.
- **`resetGame()` & `startGame()`:** Resets snake position, score, and flags, spawns the initial apple, sets `gameState = STATE_PLAYING`, and starts the Windows timer (`SetTimer`).
- **`gameTick()`:** Executes one simulation step: updates direction, moves the snake, checks collisions, and processes apple consumption.
- **`endGame()` & `quitToMenu()`:** Stops the timer (`KillTimer`), updates `highscore.txt`, and transitions to `STATE_GAMEOVER` or `STATE_MENU`.

### 4. `ui.c`
Handles all GDI graphics rendering, button layouts, and keyboard/mouse input routing.
- **Drawing Helpers:**
  - `box()` & `cell()`: Draws solid rectangles and grid-aligned cells.
  - `drawText()`: Renders bold `"Segoe UI"` text with configurable size, color, and alignment.
  - `drawButton()`, `drawBackground()`, `drawPanel()`: Renders consistent UI buttons (with hover color feedback) and overlay dialogs.
- **Screen Renderers:**
  - `drawFrame()`: Master rendering dispatcher that draws the active screen based on `gameState`.
  - `drawBoard()`, `drawSnake()`, `drawFood()`, `drawScore()`: Renders the checkerboard grass grid, border walls, snake head/body, red apple, and top status bar.
  - `drawMainMenu()`, `drawDifficultyScreen()`, `drawHighScoreScreen()`, `drawPauseScreen()`, `drawGameOverScreen()`: Renders each menu and overlay screen.
- **Input & Button Handlers:**
  - `buttonAt()` & `doButton()`: Maps mouse coordinates and slot indices (`0–3`) to menu actions.
  - `handleKeyDown()`, `handleMouseClick()`, `handleMouseMove()`: Processes keyboard controls, button clicks, and mouse hover states.

### 5. `food_score.c`
Manages apple spawning, scoring, persistent file storage, difficulty settings, and pausing.
- **`generateFood()`:** Seeds the RNG once and picks random coordinates inside the playable grid (`1..WIDTH-2`, `1..HEIGHT-2`), looping until a cell not occupied by the snake is found. Ends the game in victory if the snake fills the entire board.
- **`checkFoodEaten()`:** Returns `1` if the snake's head (`snakeX[0]`, `snakeY[0]`) is on `(foodX, foodY)`.
- **`updateScore()` & `resetScore()`:** Adds `POINTS_PER_APPLE` (`10`) or resets `score` to `0`.
- **`loadHighScore()`, `saveHighScore()`, `updateHighScore()`:** Reads from and writes to `highscore.txt` using standard C file I/O (`fopen`, `fscanf`, `fprintf`, `fclose`).
- **`setDifficulty()`:** Updates `difficulty` and sets `speed` to `SPEED_EASY`, `SPEED_MEDIUM`, or `SPEED_HARD`.
- **`pauseGame()`:** Toggles `gameState` between `STATE_PLAYING` and `STATE_PAUSED`.

---

## How It Works

Instead of blocking the CPU with an infinite `while` loop and `Sleep()`, the game uses an **event-driven architecture**:
1. `startGame()` registers a Windows timer (`GAME_TIMER_ID`) firing every `speed` milliseconds.
2. Each `WM_TIMER` event invokes `gameTick()` to advance the game state by one grid step, then calls `InvalidateRect()` to trigger `WM_PAINT`.
3. `WM_PAINT` renders the current `gameState` off-screen and blits it to the window, keeping UI interactions responsive at all times.

---

## Compilation & Execution

### Using GCC (MinGW on Windows)
Open Command Prompt in the project directory and run:
```bash
gcc -Wall -Wextra main.c snake.c ui.c food_score.c -o snake.exe -luser32 -lgdi32 -mwindows
```

Run the compiled game:
```bash
snake.exe
```

### Using Code::Blocks
1. Create a new **Win32 GUI** or **Console** project and add `game.h`, `main.c`, `snake.c`, `ui.c`, and `food_score.c`.
2. Go to **Project → Build options → Linker settings** and add `gdi32` and `user32` under *Link libraries*.
3. Under *Other linker options*, add `-mwindows`.
4. Press **F9** to build and run.

---

## Team Contributions
- **Mohib:** Window management, event loop, snake movement, collision detection, and game loop (`main.c`, `snake.c`).
- **Aarish:** GDI graphics rendering, menus, overlays, and keyboard/mouse input handling (`ui.c`).
- **Hasan:** Food generation, scoring system, persistent high score file I/O, difficulty settings, pause logic, and documentation (`food_score.c`, `README.md`).
