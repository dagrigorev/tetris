# Tetris

Полностью обновлённая версия простого Tetris на **C++17 + SDL2**.

## Что исправлено

- Убран монолитный `game.cpp`: код разделён на `App`, `Game`, `Renderer`, `Tetromino`.
- Удалены build-артефакты из структуры проекта.
- CMake переведён на `FetchContent`: SDL2 скачивается и собирается автоматически.
- Поле приведено к нормальному размеру Tetris: `10x20`.
- Оставлены 7 классических фигур.
- Добавлена 7-bag генерация фигур вместо простого `rand()`.
- Исправлены небезопасные повороты: сначала проверяется кандидат, потом применяется состояние.
- Добавлены wall-kick попытки для поворота около стен.
- Добавлен time-based game loop вместо frame-counter логики.
- Добавлены score, level и lines. Значения выводятся в заголовке окна.
- Добавлены ghost piece, next queue, pause, restart и game over.

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

## Сборка

Требуется CMake 3.24+ и C++17 компилятор.

```bash
cmake -S . -B build
cmake --build build --config Release
```

Запуск:

```bash
./build/bin/tetris
```

На Windows с multi-config генераторами исполняемый файл может быть здесь:

```bash
build/bin/Release/tetris.exe
```

## Структура

```text
src/
  App.h / App.cpp           SDL lifecycle, event loop
  Game.h / Game.cpp         игровая модель, правила, score, level
  Renderer.h / Renderer.cpp SDL rendering
  Tetromino.h / Tetromino.cpp фигуры и цвета
```
