# Snake Game in C (Windows GUI)

A classic 2D Snake game with a graphical window, written in C as our first-semester project.

The player controls a snake with the arrow keys and guides it towards apples. Each apple makes the snake longer and adds to the score. The game ends if the snake hits a wall or its own body.

The game uses the **Windows API (Win32 and GDI)** to draw a real window with graphics, buttons and mouse support. No extra libraries need to be installed.

## Features

- Graphical window with a grass board, walls, a blue snake with eyes and a red apple
- Snake movement with the arrow keys
- Apples appear at random positions (never on the snake)
- Snake grows and the score increases after each apple
- Game over when the snake hits a wall or itself
- Three difficulty levels: Easy, Medium and Hard
- High score saved in a file, so it is kept after closing the game
- Pause and resume
- Main menu, difficulty screen, high score screen and game over screen
- Clickable buttons with mouse hover effect (keyboard shortcuts also work)

## How to Play

| Key / Mouse | Action |
|---|---|
| Arrow keys | Move the snake |
| P | Pause / resume |
| ESC | Go back to the main menu |
| Mouse click | Press the buttons in the menus |
| 1 - 4 | Menu shortcuts (Start, Difficulty, High Score, Exit) |
| Y / N | Play again / menu on the game over screen |

- Eat the red apples to grow and score points.
- Each apple gives **10 points**.
- Do not hit the walls or the snake's own body.
- The snake cannot turn directly back on itself.

### Difficulty Levels

| Level | Time between moves |
|---|---|
| Easy | 150 ms |
| Medium | 100 ms |
| Hard | 60 ms |

A shorter time means a faster snake. Medium is the default. Change the level from **Difficulty** in the main menu.

## Project Structure

| File | Description |
|---|---|
| `game.h` | Shared constants, global variables and function declarations |
| `main.c` | Global variables, window creation and event handling (`WinMain`, `WndProc`) |
| `snake.c` | Snake logic, collisions and game flow (`gameTick`, `startGame`, `endGame`) |
| `ui.c` | Drawing (GDI), screens, buttons, keyboard and mouse input |
| `food_score.c` | Food, score, high score, difficulty and pause |
| `highscore.txt` | Created automatically when the game saves a high score |

## How It Works

The game does not use a `while` loop with `Sleep()`. Windows sends our window a timer message every few milliseconds (the difficulty sets how often). Each timer message calls `gameTick()`, which performs one step: move the snake, check collisions, check food. Then the window is redrawn by `drawFrame()`.

The program is always in one screen (`gameState`): menu, difficulty, playing, paused, game over or high score. The drawing and the keyboard/mouse handling both depend on this state.

## Team Members and Contributions

| Member | Files | Functions |
|---|---|---|
| **Mohib** | `main.c`, `snake.c` | `WinMain`, `WndProc`, `initSnake`, `moveSnake`, `growSnake`, `checkWallCollision`, `checkSelfCollision`, `resetGame`, `startGame`, `gameTick`, `endGame`, `quitToMenu` |
| **Aarish** | `ui.c` | `drawFrame`, `drawBoard`, `drawSnake`, `drawFood`, `drawScore`, `drawMainMenu`, `drawDifficultyScreen`, `drawHighScoreScreen`, `drawPauseScreen`, `drawGameOverScreen`, `handleKeyDown`, `handleMouseClick`, `handleMouseMove` |
| **Hasan** | `food_score.c`, `README.md` | `generateFood`, `checkFoodEaten`, `updateScore`, `resetScore`, `loadHighScore`, `saveHighScore`, `updateHighScore`, `setDifficulty`, `pauseGame` |

## Tools and Technologies

- **Language:** C
- **Operating system:** Windows 10/11
- **Libraries:** Standard C libraries (`stdio.h`, `stdlib.h`, `string.h`, `time.h`) and the Windows API (`windows.h`: window, timer, GDI graphics, keyboard and mouse)
- **Compiler:** GCC (MinGW) or Code::Blocks

## How to Compile and Run

1. Download the project: click **Code, then Download ZIP** on GitHub, then extract it.
2. Open a Command Prompt in the extracted folder.
3. Compile all the `.c` files together:

   ```
   gcc main.c snake.c ui.c food_score.c -o snake.exe -mwindows -lgdi32
   ```

4. Run the game:

   ```
   snake.exe
   ```

**Using Code::Blocks:** create a new project (Console application is fine), add all four `.c` files and `game.h`, then go to **Project, Build options, Linker settings** and add `gdi32` under "Link libraries". In **Other linker options** type `-mwindows`. Then press F9 to build and run.

> This game only works on Windows because it uses the Windows API.

## Possible Future Improvements

- Obstacles inside the play area
- Speed that increases as the score grows
- Two-player mode
- Sound effects

## Project Information

- **Project:** Snake Game
- **Course:** *(write your course name here)*
- **Semester:** First
- **Submitted to:** *(write your teacher's name here)*
