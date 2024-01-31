// #include "SFML/Graphics/RenderWindow.hpp"
// #include "SFML/Window/Keyboard.hpp"
#include "SFML/Graphics/Shape.hpp"
#include "simplesim/grid.hpp"
#include <Kokkos_Core.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/System/Vector2.hpp>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <vector>

#define WINDOW_SIZE_X 1000
#define WINDOW_SIZE_Y 1000
#define GRID_RESOLUTION 80
#define G 1000000
#define FORCEMAX 10

int main(int argc, char *argv[]) {
    Kokkos::ScopeGuard guard(argc, argv);
    sf::RenderWindow window(sf::VideoMode(WINDOW_SIZE_X, WINDOW_SIZE_Y),
                            "simplesim");
    ss::ParallelGrid grid(WINDOW_SIZE_X, WINDOW_SIZE_Y, GRID_RESOLUTION, GRID_RESOLUTION);
    grid.PrintPoints();
    printf("points size %d %d\n", grid.points.extent(0), grid.points.extent(1));
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        window.clear();
        grid.Draw(window);
        window.display();
    }

    return 0;
}
