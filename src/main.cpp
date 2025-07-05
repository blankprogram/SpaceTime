#include "Simulation.h"

int main() {
    try {
        Simulation sim(1280, 720);
        sim.run();
    } catch (const std::exception &e) {
        std::fprintf(stderr, "[Fatal Error] %s\n", e.what());
        return 1;
    }
    return 0;
}
