// #include "SFML/Graphics/RenderWindow.hpp"
// #include "SFML/Window/Keyboard.hpp"
#include "SFML/Graphics/Shape.hpp"
#include "simplesim/grid.hpp"
#include <Kokkos_Core.hpp>
#include <Kokkos_Core_fwd.hpp>
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

class GravityGrid : public ss::ParallelGrid {
  public:
    GravityGrid(int xSize, int ySize, int xCount, int yCount)
        : ss::ParallelGrid(xSize, ySize, xCount, yCount) {}

    void Update(Kokkos::View<float *[3], Kokkos::DefaultHostExecutionSpace> objects) {
        int pointCount = this->xCount * this->yCount;
        float offsetX = random()%20 - 10;
        float offsetY = random()%20 - 10;
        for (int i = 0; i < pointCount; i++) {
            this->points(i, 2) = this->points(i, 0) + offsetX;
            this->points(i, 3) = this->points(i, 1) + offsetY;
        }
        return;
    }
};

int main(int argc, char *argv[]) {
    Kokkos::ScopeGuard guard(argc, argv);
    sf::RenderWindow window(sf::VideoMode(WINDOW_SIZE_X, WINDOW_SIZE_Y),
                            "simplesim");
    GravityGrid grid(WINDOW_SIZE_X, WINDOW_SIZE_Y, GRID_RESOLUTION,
                          GRID_RESOLUTION);
    Kokkos::View<float *[3], Kokkos::DefaultHostExecutionSpace> objects("objects", 5);
    for (int i = 0; i < 3; i++) {
        objects(i, 0) = 100;
        objects(i, 1) = random()%WINDOW_SIZE_X;
        objects(i, 2) = random()%WINDOW_SIZE_Y;
    }


    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        window.clear();
        grid.Update(objects);
        for (int i = 0; i < 3; i++) {
            sf::CircleShape circle(10, 10);
            circle.setPosition(objects(i, 1), objects(i, 2));
            circle.setOrigin(10, 10);
            window.draw(circle);
        }
        grid.Draw(window);
        window.display();
    }

    return 0;
}
