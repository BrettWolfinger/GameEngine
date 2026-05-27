#include "MarioGame.h"
#include <iostream>

int main() {
    try {
        MarioGame game;
        game.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
