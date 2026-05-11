#pragma once

#include "Game.h"

#include <SDL.h>

namespace tetris {

class Renderer {
public:
    static constexpr int CellSize = 28;
    static constexpr int BoardOffsetX = 24;
    static constexpr int BoardOffsetY = 24;
    static constexpr int SidebarWidth = 180;
    static constexpr int WindowWidth = BoardOffsetX * 2 + Game::BoardWidth * CellSize + SidebarWidth;
    static constexpr int WindowHeight = BoardOffsetY * 2 + Game::BoardHeight * CellSize;

    explicit Renderer(SDL_Renderer* renderer);

    void render(const Game& game);

private:
    void drawBoardBackground();
    void drawBoardCells(const Game& game);
    void drawPiece(const Game::Piece& piece, bool ghost = false);
    void drawGrid();
    void drawSidebar(const Game& game);
    void drawPreviewPiece(TetrominoType type, int originX, int originY);
    void drawOverlay(const Game& game);
    void fillRect(int x, int y, int w, int h, SDL_Color color);
    void drawRect(int x, int y, int w, int h, SDL_Color color);

    SDL_Renderer* renderer_{};
};

} // namespace tetris
