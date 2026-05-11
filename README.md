# Layered Games / Tetris C++23

Проект переписан как заготовка для набора игр, а не как одиночный `game.cpp`.

## Слои

```text
include/gamecore + src/core
  Общеигровые абстракции: IGame, IRenderer2D, IInputSource, GameRegistry, EventBus, Application.

include/games/tetris + src/games/tetris
  Предметная логика Tetris: Board, Tetromino, SevenBagTetrominoProvider, scoring strategy, TetrisGame.

include/platform/sdl2 + src/platform/sdl2
  Конкретная реализация окна, ввода и 2D-рендера через SDL2.
```

## Использованные принципы и паттерны

- SOLID:
  - `Application` зависит от абстракций `IGame`, `IRenderer2D`, `IInputSource`.
  - SDL2 изолирован в `platform/sdl2`.
  - Tetris не знает про `SDL_Renderer` и `SDL_Window`.
- GoF:
  - **Abstract Factory / Factory Method**: `IGameFactory`, `TetrisGameFactory`.
  - **Registry**: `GameRegistry` для подключения игр.
  - **Strategy**: `IScoringStrategy`, `ClassicScoringStrategy`.
  - **Observer**: `EventBus`.
  - **Command**: `InputCommand` как абстрактные команды ввода.
  - **State**: `TetrisPhase` (`Playing`, `Paused`, `GameOver`).
  - **Bridge**: `IRenderer2D` отделяет игровую логику от SDL2.

## Сборка clang++

```bash
cmake --preset clang-release
cmake --build --preset clang-release
```

Debug:

```bash
cmake --preset clang-debug
cmake --build --preset clang-debug
```

Запуск:

```bash
./build/clang-release/layered_tetris
```

На Windows путь будет примерно:

```powershell
.\build\clang-release\layered_tetris.exe
```

## Управление

- `Left / A` — влево
- `Right / D` — вправо
- `Down / S` — soft drop
- `Up / W / X` — поворот по часовой
- `Z` — поворот против часовой
- `Space` — hard drop
- `P` — pause
- `R` — restart
- `Esc` — выход

## Как добавить новую игру

1. Создать `include/games/<game>` и `src/games/<game>`.
2. Реализовать `IGame`.
3. Реализовать `IGameFactory`.
4. Зарегистрировать фабрику в `main.cpp` через `GameRegistry`.
5. Не использовать SDL2 внутри предметного слоя — только абстракции из `gamecore`.
