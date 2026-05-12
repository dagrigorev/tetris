# Layered Games / Tetris + Arkanoid + Galaga + Pacman C++23

Проект теперь является заготовкой для набора игр, а не одиночным `game.cpp`.

Добавлено:

- главное меню выбора игры;
- космический фон меню с эффектом движения среди звезд;
- новая игра `Arkanoid`;
- новая игра `Galaga`;
- новая игра `Pacman`;
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

include/games/galaga + src/games/galaga
  Предметная логика Galaga: player ship, enemy formation, bullets, waves,
  scoring, lives, GalagaGame.

include/games/pacman + src/games/pacman
  Предметная логика Pacman: tile maze, pellets, ghosts, movement, scoring,
  lives, PacmanGame.

include/platform/sdl2 + src/platform/sdl2
  Конкретная реализация окна, ввода и 2D-рендера через SDL2.
```

## Архитектура переключения игр

`main.cpp` регистрирует фабрики игр в `GameRegistry`:

- `TetrisGameFactory`
- `ArkanoidGameFactory`
- `GalagaGameFactory`
- `PacmanGameFactory`

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
  - Tetris, Arkanoid, Galaga и Pacman не знают про `SDL_Renderer` и `SDL_Window`.
- GoF:
  - **Abstract Factory / Factory Method**: `IGameFactory`, игровые фабрики Tetris/Arkanoid/Galaga/Pacman.
  - **Registry**: `GameRegistry` для подключения игр.
  - **Strategy**: `IScoringStrategy`, `ClassicScoringStrategy` в Tetris.
  - **Observer**: `EventBus`.
  - **Command**: `InputCommand` как абстрактные команды ввода.
  - **State**: состояния Tetris, Arkanoid и меню.
  - **Bridge**: `IRenderer2D` отделяет игровую логику от SDL2.

## Автоматическая сборка в `dist`

Windows:

```bat
build.cmd -Configuration Release -Clean
```

Или напрямую через PowerShell:

```powershell
.\scripts\build.ps1 -Configuration Release -Clean
```

Скрипт автоматически:

- выбирает рабочий CMake generator на Windows;
- по умолчанию предпочитает установленный Visual Studio generator, чтобы не падать на `clang++` без `oldnames.lib` / `msvcrt*.lib`;
- для `Ninja` / `NMake Makefiles` импортирует MSVC Developer Environment через `vswhere` и `VsDevCmd.bat`;
- конфигурирует CMake;
- собирает target `layered_games`;
- при необходимости подтягивает SDL2 через `FetchContent`;
- копирует итоговый `layered_games.exe` и `SDL2.dll` в директорию `dist` в корне проекта.

Запуск после сборки:

```powershell
.\dist\layered_games.exe
```

Debug-сборка:

```powershell
.\scripts\build.ps1 -Configuration Debug -Clean
```

Принудительно использовать Ninja можно так:

```powershell
.\scripts\build.ps1 -Generator Ninja -Configuration Release -Clean
```

В этом режиме скрипт сам попытается импортировать окружение Visual Studio C++ Build Tools, чтобы линковщик видел системные MSVC-библиотеки.

Если SDL2 уже установлен и должен находиться через `find_package(SDL2 CONFIG REQUIRED)`, можно отключить загрузку SDL2:

```powershell
.\scripts\build.ps1 -Configuration Release -NoFetchSdl2
```

## Ручная сборка clang++

```bash
cmake --preset clang-release
cmake --build --preset clang-release
```

После успешной сборки CMake post-build шаг также копирует исполняемый файл и runtime-зависимости в `dist`.

## Модель ввода

Ввод разделен на два типа команд:

- `pressedCommands` — одноразовые действия в кадр нажатия: запуск, поворот, пауза, restart, выход в меню;
- `heldCommands` — состояние зажатых клавиш, которое приходит каждый кадр, пока клавиша физически удерживается.

За счет этого движение в обеих играх продолжается до тех пор, пока удерживаются клавиши движения:

- в `Arkanoid` платформа непрерывно движется при удержании `Left/A` или `Right/D`;
- в `Galaga` корабль непрерывно движется при удержании `Left/A` или `Right/D`;
- в `Pacman` направление движения выбирается и продолжается при удержании `W/A/S/D` или стрелок;
- в `Tetris` боковое движение работает через controlled repeat: первый шаг сразу, затем задержка и регулярное повторение; `SoftDrop` тоже работает удержанием.

## Визуальные улучшения Pacman и Galaga

- `Pacman` и призраки больше не рисуются квадратами: Pacman строится как круг с направленной пастью, призраки имеют округлую голову, волнистый низ и глаза.
- `Galaga` больше не рисует корабли прямоугольниками: игрок и враги строятся из треугольников, кругов, кабины, крыльев, подсветки и двигателей.
- Для этого в `IRenderer2D` добавлены базовые примитивы `fillCircle`, `drawCircle`, `fillTriangle`. SDL2-реализация находится в `platform/sdl2`.

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


## Управление Galaga

- `Left / A` — движение корабля влево
- `Right / D` — движение корабля вправо
- `Space / Enter` — старт / выстрел
- `P` — pause
- `R` — restart
- `Esc` — вернуться в главное меню
- `Q` — выход из приложения

## Управление Pacman

- `W / A / S / D` или стрелки — движение по лабиринту
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


## Games

- Tetris
- Arkanoid
- Galaga
- Pacman
- Battle City


### Battle City

Классическая игра с танками: разрушимые кирпичные блоки, стальные стены, вода, лес, база-орел, вражеские танки, пули и переход между стадиями. Управление: WASD/стрелки — движение, Space/Enter — огонь/старт.

## Последнее обновление

- В `Pacman` добавлена базовая анимация: открытие/закрытие рта, подпрыгивание призраков, волнистый низ призраков и направление взгляда.
- В `Battle City` улучшена детализация графики: кирпичи, сталь, вода, деревья, танки, снаряды и база рисуются более выразительно.
- В `Battle City` добавлены анимации: движение гусениц, волны воды, покачивание деревьев, следы снарядов и взрывы при попаданиях.
