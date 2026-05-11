#include "App.h"

#include <exception>
#include <iostream>

int main(int, char**) {
    try {
        tetris::App app;
        return app.run();
    } catch (const std::exception& ex) {
        std::cerr << "Fatal error: " << ex.what() << '\n';
        return 1;
    }
}
