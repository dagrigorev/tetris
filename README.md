# Tetris C++23 / clang++

Полностью обновлённая версия простого Tetris на **C++23 + SDL2**, ориентированная на сборку через **clang++** и CMake `FetchContent`.

## Что изменено в этой версии

- Проект переведён на стандарт **C++23**.
- `CMakeLists.txt` требует `cxx_std_23` и выставляет `CMAKE_CXX_STANDARD 23`.
- Добавлены `CMakePresets.json` профили для `clang++`:
  - `clang-debug`
  - `clang-release`
- SDL2 подключается через `FetchContent`, без ручной установки путей.
- Для clang включены строгие предупреждения: `-Wall`, `-Wextra`, `-Wpedantic`, `-Wconversion`, `-Wshadow` и другие.
- Для Debug-пресета включены sanitizers: AddressSanitizer + UndefinedBehaviorSanitizer.
- Логика времени переведена на `std::chrono::duration<double>`.
- Используются современные C++-подходы: `std::ranges`, `std::to_underlying`, structured bindings, `using enum`, `final`, `[[nodiscard]]`.

## Игровые исправления

- Убран монолитный `game.cpp`: код разделён на `App`, `Game`, `Renderer`, `Tetromino`.
- Поле приведено к классическому размеру Tetris: `10x20`.
- Оставлены 7 классических фигур.
- Добавлена 7-bag генерация фигур вместо `rand()`.
- Повороты безопасны: сначала проверяется кандидат, только потом меняется состояние.
- Добавлены простые wall-kick попытки.
- Добавлены score, lines, level, ghost piece, next queue, pause, restart и game over.

## Требования

- CMake 3.24+
- Ninja
- clang / clang++ с поддержкой C++23
- Интернет при первой конфигурации, потому что SDL2 скачивается через `FetchContent`

## Сборка clang++ Debug

```bash
cmake --preset clang-debug
cmake --build --preset clang-debug
```

Исполняемый файл:

```bash
./build/clang-debug/bin/tetris
```

## Сборка clang++ Release

```bash
cmake --preset clang-release
cmake --build --preset clang-release
```

Исполняемый файл:

```bash
./build/clang-release/bin/tetris
```

## Ручная сборка без preset

```bash
cmake -S . -B build/clang-release \
  -G Ninja \
  -DCMAKE_C_COMPILER=clang \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_BUILD_TYPE=Release

cmake --build build/clang-release
```

## Управление

| Клавиша | Действие |
|---|---|
| Left / A | Влево |
| Right / D | Вправо |
| Down / S | Soft drop |
| Up / W / X | Поворот по часовой |
| Z | Поворот против часовой |
| Space | Hard drop |
| P | Пауза |
| R | Рестарт |
| Esc | Выход |

## Структура

```text
src/
  App.h / App.cpp             SDL lifecycle, event loop
  Game.h / Game.cpp           игровая модель, правила, score, level
  Renderer.h / Renderer.cpp   SDL rendering
  Tetromino.h / Tetromino.cpp фигуры и цвета
```
