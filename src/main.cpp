#include "SFML/Graphics/RenderWindow.hpp"
#include "simplesim/grid.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/System/Vector2.hpp>
// #include <cstdio>
// #include <cstdlib>
// #include <iostream>
// #include <tuple>

class MassedObject {
  public:
    sf::CircleShape shape;
    float mass;
    sf::Vector2f pos;
    MassedObject(float radius, float mass, sf::Vector2f pos)
        : shape(radius), mass(mass), pos(pos) {
        SetPosition(pos);
    }

    void SetPosition(sf::Vector2f pos) {
        shape.setPosition(pos);
        shape.setOrigin(shape.getRadius(), shape.getRadius());
    }
};

class GravityGrid : public ss::PointGrid {
  public:
    GravityGrid(int xSize, int ySize, int xCount, int yCount)
        : ss::PointGrid(xSize, ySize, xCount, yCount) {}

    void Update() { return; }
};

int main() {
    sf::RenderWindow window(sf::VideoMode(720, 720), "simplesim");
    
    GravityGrid gravGrid(window.getSize().x, window.getSize().y, 50, 50);
    MassedObject obj(10, 30, sf::Vector2f(300, 300));

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

        }

        auto mousePos = sf::Mouse::getPosition(window);
        printf("mouse at (%d, %d)\n", mousePos.x, mousePos.y);
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            auto mousePos = sf::Mouse::getPosition(window);
            obj.SetPosition((sf::Vector2f) mousePos);
        }

        window.clear();
        gravGrid.Draw(window);
        window.draw(obj.shape);
        window.display();
    }

    return 0;
}
