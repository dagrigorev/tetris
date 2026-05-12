# Layered Games / Tetris + Arkanoid C++23

Проект теперь является заготовкой для набора игр, а не одиночным `game.cpp`.

Добавлено:

- главное меню выбора игры;
- космический фон меню с эффектом движения среди звезд;
- новая игра `Arkanoid`;
- переключение между играми без нарушения слоев архитектуры.

## Слои

```text
include/gamecore + src/core
  Общеигровые абстракции: IGame, IRenderer2D, IInputSource, GameRegistry,
  EventBus, Application, GameShell.

include/games/tetris + src/games/tetris
  Предметная логика Tetris: Board, Tetromino, SevenBagTetrominoProvider,
  scoring strategy, TetrisGame.

include/games/arkanoid + src/games/arkanoid
  Предметная логика Arkanoid: paddle, ball, bricks, scoring, lives,
  collisions, ArkanoidGame.

include/platform/sdl2 + src/platform/sdl2
  Конкретная реализация окна, ввода и 2D-рендера через SDL2.
```

## Архитектура переключения игр

`main.cpp` регистрирует фабрики игр в `GameRegistry`:

- `TetrisGameFactory`
- `ArkanoidGameFactory`

После этого запускается `gamecore::GameShell`. Он сам:

- показывает главное меню;
- рисует звездный фон;
- создает выбранную игру через `GameRegistry`;
- возвращает пользователя в меню по `Esc`;
- не зависит от SDL2 напрямую.

## Использованные принципы и паттерны

- SOLID:
  - `Application` зависит от абстракций `IGame`, `IRenderer2D`, `IInputSource`.
  - SDL2 изолирован в `platform/sdl2`.
  - Tetris и Arkanoid не знают про `SDL_Renderer` и `SDL_Window`.
- GoF:
  - **Abstract Factory / Factory Method**: `IGameFactory`, `TetrisGameFactory`, `ArkanoidGameFactory`.
  - **Registry**: `GameRegistry` для подключения игр.
  - **Strategy**: `IScoringStrategy`, `ClassicScoringStrategy` в Tetris.
  - **Observer**: `EventBus`.
  - **Command**: `InputCommand` как абстрактные команды ввода.
  - **State**: состояния Tetris, Arkanoid и меню.
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
./build/clang-release/layered_games
```

На Windows путь будет примерно:

```powershell
.\build\clang-release\layered_games.exe
```

## Управление в меню

- `Up / W` — выбрать игру выше
- `Down / S` — выбрать игру ниже
- `Enter / Space` — запустить игру
- `Esc` — выйти из приложения
- `Q` — выйти из приложения

## Управление Tetris

- `Left / A` — влево
- `Right / D` — вправо
- `Down / S` — soft drop
- `Up / W / X` — поворот по часовой
- `Z` — поворот против часовой
- `Space` — hard drop
- `P` — pause
- `R` — restart
- `Esc` — вернуться в главное меню
- `Q` — выход из приложения

## Управление Arkanoid

- `Left / A` — движение платформы влево
- `Right / D` — движение платформы вправо
- `Space / Enter` — запустить шар
- `P` — pause
- `R` — restart
- `Esc` — вернуться в главное меню
- `Q` — выход из приложения

## Как добавить новую игру

1. Создать `include/games/<game>` и `src/games/<game>`.
2. Реализовать класс игры от `gamecore::IGame`.
3. Реализовать фабрику от `gamecore::IGameFactory`.
4. Добавить новый static library target в `CMakeLists.txt`.
5. Зарегистрировать фабрику в `main.cpp` через `GameRegistry`.
6. Не использовать SDL2 внутри предметного слоя — только абстракции из `gamecore`.
