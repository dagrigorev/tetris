#include "App.h"

#include <exception>
#include <iostream>

int main(int, char**) {
    try {
        auto app = tetris::App{};
        return app.run();
    } catch (const std::exception& ex) {
        std::cerr << "Fatal error: " << ex.what() << '\n';
        return 1;
    }
}
