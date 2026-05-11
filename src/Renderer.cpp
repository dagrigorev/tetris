#include "Renderer.h"

#include <algorithm>

namespace tetris {
namespace {

SDL_Color rgb(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255) {
    return SDL_Color{r, g, b, a};
}

SDL_Color dim(SDL_Color color, unsigned char alpha) {
    color.a = alpha;
    return color;
}

} // namespace

Renderer::Renderer(SDL_Renderer* renderer)
    : renderer_(renderer) {
    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
}

void Renderer::render(const Game& game) {
    SDL_SetRenderDrawColor(renderer_, 10, 12, 18, 255);
    SDL_RenderClear(renderer_);

    drawBoardBackground();
    drawBoardCells(game);

    if (game.state() != Game::State::GameOver) {
        drawPiece(game.ghostPiece(), true);
        drawPiece(game.activePiece(), false);
    }

    drawGrid();
    drawSidebar(game);
    drawOverlay(game);

    SDL_RenderPresent(renderer_);
}

void Renderer::drawBoardBackground() {
    fillRect(BoardOffsetX - 2,
             BoardOffsetY - 2,
             Game::BoardWidth * CellSize + 4,
             Game::BoardHeight * CellSize + 4,
             rgb(38, 45, 58));

    fillRect(BoardOffsetX,
             BoardOffsetY,
             Game::BoardWidth * CellSize,
             Game::BoardHeight * CellSize,
             rgb(16, 20, 28));
}

void Renderer::drawBoardCells(const Game& game) {
    const auto& board = game.board();

    for (int y = 0; y < Game::BoardHeight; ++y) {
        for (int x = 0; x < Game::BoardWidth; ++x) {
            const int value = board[static_cast<std::size_t>(y * Game::BoardWidth + x)];
            if (value == 0) continue;

            const auto type = static_cast<TetrominoType>(value - 1);
            fillRect(BoardOffsetX + x * CellSize + 1,
                     BoardOffsetY + y * CellSize + 1,
                     CellSize - 2,
                     CellSize - 2,
                     color(type));
        }
    }
}

void Renderer::drawPiece(const Game::Piece& piece, bool ghost) {
    SDL_Color pieceColor = color(piece.type);
    if (ghost) {
        pieceColor = dim(pieceColor, 55);
    }

    for (const Cell& cell : shape(piece.type, piece.rotation)) {
        const int x = piece.x + cell.x;
        const int y = piece.y + cell.y;
        if (x < 0 || x >= Game::BoardWidth || y < 0 || y >= Game::BoardHeight) continue;

        const int px = BoardOffsetX + x * CellSize;
        const int py = BoardOffsetY + y * CellSize;

        if (ghost) {
            drawRect(px + 3, py + 3, CellSize - 6, CellSize - 6, pieceColor);
        } else {
            fillRect(px + 1, py + 1, CellSize - 2, CellSize - 2, pieceColor);
            fillRect(px + 4, py + 4, CellSize - 8, 4, dim(rgb(255, 255, 255), 70));
        }
    }
}

void Renderer::drawGrid() {
    const SDL_Color line = rgb(55, 62, 76, 150);

    for (int x = 0; x <= Game::BoardWidth; ++x) {
        const int px = BoardOffsetX + x * CellSize;
        SDL_SetRenderDrawColor(renderer_, line.r, line.g, line.b, line.a);
        SDL_RenderDrawLine(renderer_, px, BoardOffsetY, px, BoardOffsetY + Game::BoardHeight * CellSize);
    }

    for (int y = 0; y <= Game::BoardHeight; ++y) {
        const int py = BoardOffsetY + y * CellSize;
        SDL_SetRenderDrawColor(renderer_, line.r, line.g, line.b, line.a);
        SDL_RenderDrawLine(renderer_, BoardOffsetX, py, BoardOffsetX + Game::BoardWidth * CellSize, py);
    }
}

void Renderer::drawSidebar(const Game& game) {
    const int x = BoardOffsetX + Game::BoardWidth * CellSize + 28;
    const int y = BoardOffsetY;
    const int w = SidebarWidth - 48;

    fillRect(x - 12, y, w + 24, Game::BoardHeight * CellSize, rgb(19, 24, 34));
    drawRect(x - 12, y, w + 24, Game::BoardHeight * CellSize, rgb(44, 52, 68));

    // Decorative stat bars. Actual values are in the window title to avoid SDL_ttf dependency.
    const int scoreWidth = std::min(w, 20 + (game.score() % 3000) * w / 3000);
    const int levelWidth = std::min(w, 20 + (game.level() % 12) * w / 12);
    const int linesWidth = std::min(w, 20 + (game.lines() % 40) * w / 40);

    fillRect(x, y + 24, w, 10, rgb(35, 42, 55));
    fillRect(x, y + 24, scoreWidth, 10, rgb(0, 220, 255));
    fillRect(x, y + 48, w, 10, rgb(35, 42, 55));
    fillRect(x, y + 48, levelWidth, 10, rgb(255, 220, 0));
    fillRect(x, y + 72, w, 10, rgb(35, 42, 55));
    fillRect(x, y + 72, linesWidth, 10, rgb(90, 230, 80));

    const auto preview = game.nextPreview();
    int previewY = y + 122;
    for (TetrominoType type : preview) {
        drawPreviewPiece(type, x + 20, previewY);
        previewY += 78;
    }
}

void Renderer::drawPreviewPiece(TetrominoType type, int originX, int originY) {
    constexpr int previewCell = 16;
    const SDL_Color pieceColor = color(type);

    for (const Cell& cell : shape(type, 0)) {
        const int px = originX + cell.x * previewCell;
        const int py = originY + cell.y * previewCell;
        fillRect(px + 1, py + 1, previewCell - 2, previewCell - 2, pieceColor);
    }
}

void Renderer::drawOverlay(const Game& game) {
    if (game.state() == Game::State::Running) {
        return;
    }

    fillRect(BoardOffsetX,
             BoardOffsetY + Game::BoardHeight * CellSize / 2 - 42,
             Game::BoardWidth * CellSize,
             84,
             rgb(0, 0, 0, 170));

    const SDL_Color accent = game.state() == Game::State::Paused
        ? rgb(255, 220, 0, 220)
        : rgb(255, 80, 95, 220);

    const int centerX = BoardOffsetX + Game::BoardWidth * CellSize / 2;
    const int centerY = BoardOffsetY + Game::BoardHeight * CellSize / 2;

    if (game.state() == Game::State::Paused) {
        fillRect(centerX - 28, centerY - 24, 16, 48, accent);
        fillRect(centerX + 12, centerY - 24, 16, 48, accent);
    } else {
        drawRect(centerX - 34, centerY - 34, 68, 68, accent);
        SDL_SetRenderDrawColor(renderer_, accent.r, accent.g, accent.b, accent.a);
        SDL_RenderDrawLine(renderer_, centerX - 24, centerY - 24, centerX + 24, centerY + 24);
        SDL_RenderDrawLine(renderer_, centerX + 24, centerY - 24, centerX - 24, centerY + 24);
    }
}

void Renderer::fillRect(int x, int y, int w, int h, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a);
    SDL_Rect rect{x, y, w, h};
    SDL_RenderFillRect(renderer_, &rect);
}

void Renderer::drawRect(int x, int y, int w, int h, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a);
    SDL_Rect rect{x, y, w, h};
    SDL_RenderDrawRect(renderer_, &rect);
}

} // namespace tetris
