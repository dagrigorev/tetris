// main.cpp
#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <array>
#include <cstdlib>
#include <ctime>
#include <queue>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int TILE_SIZE = 8;
const int GRID_WIDTH = SCREEN_WIDTH / TILE_SIZE;
const int GRID_HEIGHT = SCREEN_HEIGHT / TILE_SIZE;
const int QUEUE_SIZE = 4; // Number of shapes to preview

// Color definitions for the tetrominoes
const SDL_Color COLORS[] = {
    {255, 0, 0, 255},    // Red
    {0, 255, 0, 255},    // Green
    {0, 0, 255, 255},    // Blue
    {255, 255, 0, 255},  // Yellow
    {0, 255, 255, 255},  // Cyan
    {255, 165, 0, 255},  // Orange
    {128, 0, 128, 255},   // Purple
    {255, 105, 180, 255}, // Pink
    {128, 128, 128, 255}, // Gray
    {0, 128, 0, 255}      // Dark Green
};

// Tetromino shapes
std::array<std::array<int, 4>, 9> tetrominoes = {{
    {{1, 3, 5, 7}}, // I shape
    {{2, 4, 5, 7}}, // Z shape
    {{3, 5, 4, 6}}, // S shape
    {{3, 5, 4, 7}}, // T shape
    {{2, 3, 5, 7}}, // L shape
    {{3, 5, 7, 6}}, // J shape
    {{2, 3, 4, 5}}, // O shape (square)
    {{2, 3, 4, 6}}, // New shape 1 (example: a long "S")
    {{4, 5, 6, 7}}  // New shape 2 (example: a line of 4)
}};

int currentPiece = 0; // The index of the current tetromino shape
std::array<int, 4> currentTetromino; // The 4 blocks of the current tetromino
int currentX = GRID_WIDTH / 2 - 1;
int currentY = 0;

std::queue<int> nextPiecesQueue;
// Define a grid representing the game board (0 means empty, non-zero means occupied)
std::vector<std::vector<int>> grid(GRID_HEIGHT, std::vector<int>(GRID_WIDTH, 0));

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;

bool init(SDL_Window*& window, SDL_Renderer*& renderer) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    window = SDL_CreateWindow("Tetris", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    return true;
}

void initializePiecesQueue() {
    // Clear the queue in case it's not empty
    while (!nextPiecesQueue.empty()) {
        nextPiecesQueue.pop();
    }

    // Fill the queue with random pieces
    for (int i = 0; i < QUEUE_SIZE; ++i) {
        nextPiecesQueue.push(rand() % tetrominoes.size());
    }
}

void drawVisibleGrid(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255); // Light gray for grid lines
    // Draw vertical lines
    for (int x = 0; x <= GRID_WIDTH; ++x) {
        SDL_RenderDrawLine(renderer, x * TILE_SIZE, 0, x * TILE_SIZE, SCREEN_HEIGHT);
    }
    // Draw horizontal lines
    for (int y = 0; y <= GRID_HEIGHT; ++y) {
        SDL_RenderDrawLine(renderer, 0, y * TILE_SIZE, SCREEN_WIDTH, y * TILE_SIZE);
    }
}

void drawGrid(SDL_Renderer* renderer) {
    // Draw each block of the grid
    for (int y = 0; y < GRID_HEIGHT; ++y) {
        for (int x = 0; x < GRID_WIDTH; ++x) {
            if (grid[y][x] != 0) {
                SDL_Rect block = { x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE };
                SDL_Color color = COLORS[grid[y][x] - 1];
                SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
                SDL_RenderFillRect(renderer, &block);
            }
        }
    }

    // Draw the current tetromino
    for (int i = 0; i < 4; ++i) {
        int x = (currentTetromino[i] % 2) + currentX;
        int y = (currentTetromino[i] / 2) + currentY;

        // Only draw if the block is within the grid
        if (x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT) {
            SDL_Rect block = { x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE };
            SDL_Color color = COLORS[currentPiece]; // Use the current piece's color
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
            SDL_RenderFillRect(renderer, &block);
        }
    }
}

void drawBorders(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White color for the borders

    // Draw left border
    SDL_Rect leftBorder = { 0, 0, TILE_SIZE, SCREEN_HEIGHT };
    SDL_RenderFillRect(renderer, &leftBorder);

    // Draw right border
    SDL_Rect rightBorder = { SCREEN_WIDTH - TILE_SIZE, 0, TILE_SIZE, SCREEN_HEIGHT };
    SDL_RenderFillRect(renderer, &rightBorder);

    // Draw top border
    SDL_Rect topBorder = { 0, 0, SCREEN_WIDTH, TILE_SIZE };
    SDL_RenderFillRect(renderer, &topBorder);

    // Draw bottom border
    SDL_Rect bottomBorder = { 0, SCREEN_HEIGHT - TILE_SIZE, SCREEN_WIDTH, TILE_SIZE };
    SDL_RenderFillRect(renderer, &bottomBorder);
}

void drawQueue(SDL_Renderer* renderer) {
    // Position to draw the queue (to the right of the visible grid)
    int queueX = SCREEN_WIDTH - 8 * TILE_SIZE; // X position for the queue, adjust to be visible on the screen
    int queueY = TILE_SIZE * 2; // Y position (start from 2 tiles below the top)

    std::queue<int> tempQueue = nextPiecesQueue; // Create a copy of the queue to preserve it

    // Loop through the next pieces in the queue
    for (int i = 0; i < QUEUE_SIZE && !tempQueue.empty(); ++i) {
        int piece = tempQueue.front(); // Get the next piece from the queue
        tempQueue.pop(); // Remove the piece from the temporary queue

        // Draw the tetromino shape
        for (int j = 0; j < 4; ++j) {
            int x = (tetrominoes[piece][j] % 2) * TILE_SIZE + queueX;  // Adjust block X position
            int y = (tetrominoes[piece][j] / 2) * TILE_SIZE + queueY + (i * 4 * TILE_SIZE); // Adjust Y position with spacing

            // Set the color for the current tetromino
            SDL_Color color = COLORS[piece];
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);

            // Draw the tetromino block
            SDL_Rect block = { x, y, TILE_SIZE, TILE_SIZE };
            SDL_RenderFillRect(renderer, &block);
        }
    }
}

// Rotate the tetromino
void rotateTetromino() {
    if (currentPiece == 6) return; // O-shape doesn't rotate
    for (int i = 0; i < 4; ++i) {
        int x = currentTetromino[i] % 2;
        int y = currentTetromino[i] / 2;
        currentTetromino[i] = y + 2 * (1 - x); // Simple 90-degree rotation
    }
}

// Move the tetromino left, right, or down
bool moveTetromino(int dx, int dy) {
    for (int i = 0; i < 4; ++i) {
        int x = (currentTetromino[i] % 2) + currentX + dx;
        int y = (currentTetromino[i] / 2) + currentY + dy;
        if (x < 0 || x >= GRID_WIDTH || y >= GRID_HEIGHT || grid[y][x] != 0)
            return false;
    }
    currentX += dx;
    currentY += dy;
    return true;
}

// Function to spawn a new tetromino with randomized X position and Y at the top
void spawnNewTetromino() {
    // Get the next piece from the queue
    currentPiece = nextPiecesQueue.front();
    nextPiecesQueue.pop();

    // Add a new random piece to the back of the queue
    nextPiecesQueue.push(rand() % tetrominoes.size());

    currentTetromino = tetrominoes[currentPiece];    // Assign shape

    // Randomize X position within the grid width, ensuring it's within bounds
    currentX = rand() % (GRID_WIDTH - 2);            // Adjust width to avoid boundary overflow

    // Reset Y position to the top
    currentY = 0;                                    // Start at the top of the grid
}

void clearLines() {
    for (int y = 0; y < GRID_HEIGHT; ++y) {
        bool full = true;
        for (int x = 0; x < GRID_WIDTH; ++x) {
            if (grid[y][x] == 0) {
                full = false;
                break;
            }
        }
        if (full) {
            // Shift all lines above down
            for (int i = y; i > 0; --i) {
                grid[i] = grid[i - 1];
            }
            grid[0] = std::vector<int>(GRID_WIDTH, 0); // Clear the top line
        }
    }
}

void close(SDL_Window* window, SDL_Renderer* renderer) {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int main(int argc, char* args[]) {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    if (!init(window, renderer)) {
        std::cerr << "Failed to initialize!" << std::endl;
        return -1;
    }

    bool quit = false; // Quit flag
    bool paused = false; // Flag to track pause state
    SDL_Event e;
    int dropCounter = 0;

    // Initialize the first tetromino
    currentPiece = rand() % 7;
    currentTetromino = tetrominoes[currentPiece];

    initializePiecesQueue();

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            // Handle user input for tetromino movement
            if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_LEFT: moveTetromino(-1, 0); break;
                    case SDLK_RIGHT: moveTetromino(1, 0); break;
                    case SDLK_DOWN: moveTetromino(0, 1); break;
                    case SDLK_UP: rotateTetromino(); break;
                    case SDLK_ESCAPE: quit = true; break;
                    case SDLK_SPACE: paused = !paused; break;
                }
            }
        }

        if (!paused) {
            // Move tetromino down after a certain time
            if (++dropCounter >= 100) { // Change this value to adjust fall speed
                if (!moveTetromino(0, 1)) {
                    // Lock tetromino in place if it can't move further
                    for (int i = 0; i < 4; ++i) {
                        int x = (currentTetromino[i] % 2) + currentX;
                        int y = (currentTetromino[i] / 2) + currentY;
                        grid[y][x] = currentPiece + 1;
                    }
                    clearLines();

                    // Spawn new tetromino
                    spawnNewTetromino();

                    if (!moveTetromino(0, 0)) {
                        std::cout << "Game Over!" << std::endl;
                        quit = true;
                    }
                }
                dropCounter = 0;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
        SDL_RenderClear(renderer);

        if (paused) { // Only draw the queue when paused
            drawQueue(renderer);
        }
        else {
            drawGrid(renderer);
        }

        drawVisibleGrid(renderer);
        drawBorders(renderer);

        SDL_RenderPresent(renderer);
    }

    close(window, renderer);
    return 0;
}
