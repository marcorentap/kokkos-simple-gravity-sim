#include "SFML/Graphics/RenderWindow.hpp"
#include "simplesim/grid.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/System/Vector2.hpp>
// #include <cstdio>
// #include <cstdlib>
// #include <iostream>
// #include <tuple>

class GravityGrid : public ss::PointGrid {
  public:
    GravityGrid(int xSize, int ySize, int xCount, int yCount)
        : ss::PointGrid(xSize, ySize, xCount, yCount) {}

    void Update() {
        return;
    }
};

int main() {
    sf::RenderWindow window(sf::VideoMode(720, 720), "simplesim");
    GravityGrid gravGrid(window.getSize().x, window.getSize().y, 50, 50);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        window.clear();
        gravGrid.Draw(window);
        window.display();
    }

    return 0;
}
