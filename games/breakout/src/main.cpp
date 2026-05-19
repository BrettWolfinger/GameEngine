#include "BreakoutGame.h"
#include <iostream>

int main() {
    try {
        BreakoutGame game;
        game.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
