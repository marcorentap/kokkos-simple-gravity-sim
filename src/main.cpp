#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Window/Keyboard.hpp"
#include "simplesim/grid.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/System/Vector2.hpp>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <vector>
#include <Kokkos_Core.hpp>
// #include <cstdio>
// #include <cstdlib>
// #include <iostream>
// #include <tuple>

class MassedObject {
  public:
    sf::CircleShape shape;
    float mass;
    sf::Vector2f velocity;
    sf::Vector2f acceleration;
    sf::Vector2f pos;
    MassedObject(float radius, float mass, sf::Vector2f pos)
        : shape(radius), mass(mass), pos(pos) {
        SetPosition(pos);
    }

    void SetPosition(sf::Vector2f pos) {
        this->pos = pos;
        shape.setPosition(pos);
        shape.setOrigin(shape.getRadius(), shape.getRadius());
    }
};

class GravityGrid : public ss::PointGrid {
  private:
    int pointMass = 1;
    int G = 1000000;
    int forceScale = 10;
    float lineMax = 10;

    float CalculateGravity(MassedObject M, MassedObject m) {
        float diffX = M.pos.x - m.pos.x;
        float diffY = M.pos.y - m.pos.y;
        float r = std::sqrt(std::pow(diffX, 2) + std::pow(diffY, 2));
        return r ? (G * M.mass * m.mass) / pow(r, 2) : 0;
    }
    float CalculateGravity(MassedObject M, ss::GridPoint point) {
        return CalculateGravity(
            M, MassedObject(pointMass, pointMass, point.point.getPosition()));
    }

  public:
    GravityGrid(int xSize, int ySize, int xCount, int yCount)
        : ss::PointGrid(xSize, ySize, xCount, yCount) {}

    void ParallelUpdate(std::vector<MassedObject> massedObjects) {
        int pointCount = this->points.size();
        int objectCount = massedObjects.size();
        printf("pointcount: %d objectCount: %d\n", pointCount, objectCount);

        Kokkos::View<float *[3]> pointView("pointView", pointCount);
        Kokkos::View<float *[3]> objectView("objectView", objectCount);
    }

    void Update(std::vector<MassedObject> massedObjects) {
        for (auto &point : this->points) {
            auto &startPos = point.line[0].position;
            auto &endPos = point.line[1].position;
            endPos = startPos;
            // printf("point start(%f, %f) ", startPos.x, startPos.y);
            for (auto &obj : massedObjects) {
                auto force = CalculateGravity(obj, point);
                float diffX = obj.pos.x - startPos.x;
                float diffY = obj.pos.y - startPos.y;
                float dist = std::sqrt(std::pow(diffX, 2) + std::pow(diffY, 2));
                sf::Vector2f unit(diffX / dist, diffY / dist);
                endPos += unit * std::min(std::abs(force), lineMax);
            }
        }
        return;
    }

    void PrintDebug() {
        for (auto &point : this->points) {
        }
    }
};

int main(int argc, char *argv[]) {
    Kokkos::ScopeGuard guard(argc, argv);
    sf::RenderWindow window(sf::VideoMode(1000, 1000), "simplesim");

    GravityGrid gravGrid(window.getSize().x, window.getSize().y, 80, 80);
    std::vector<MassedObject> objects = {
        MassedObject(10, 100, sf::Vector2f(300, 300)),
        MassedObject(10, 100, sf::Vector2f(700, 300)),
        MassedObject(10, 100, sf::Vector2f(500, 500)),
    };
    MassedObject *selectedObj = NULL;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::MouseButtonPressed) {
                // Left click selects an object
                if (event.mouseButton.button == sf::Mouse::Left) {
                    auto mousePos = sf::Mouse::getPosition(window);
                    for (auto &obj : objects) {
                        if (obj.pos.x < mousePos.x + obj.shape.getRadius() &&
                            obj.pos.y < mousePos.y + obj.shape.getRadius() &&
                            obj.pos.x > mousePos.x - obj.shape.getRadius() &&
                            obj.pos.y > mousePos.y - obj.shape.getRadius()) {
                            selectedObj = &obj;
                            printf("Selected obj(%f, %f)\n", obj.pos.x,
                                   obj.pos.y);
                        }
                    }
                }

                // Right click deselect object
                if (event.mouseButton.button == sf::Mouse::Right) {
                    selectedObj = NULL;
                    printf("Removed object selection\n");
                }
            }

            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            auto mousePos = sf::Mouse::getPosition(window);
            if (selectedObj) {
                selectedObj->SetPosition((sf::Vector2f)mousePos);
                printf("Move obj(%f, %f) to (%d,%d)\n", selectedObj->pos.x,
                       selectedObj->pos.y, mousePos.x, mousePos.y);
            } else {
                printf("No object selected\n");
            }
        }

        window.clear();
        gravGrid.Update(objects);
        gravGrid.ParallelUpdate(objects);
        gravGrid.Draw(window);
        for (auto obj : objects) {
            window.draw(obj.shape);
        }
        window.display();
    }

    return 0;
}
