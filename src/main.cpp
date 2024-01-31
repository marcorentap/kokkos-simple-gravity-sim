#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Window/Keyboard.hpp"
#include "simplesim/grid.hpp"
#include <Kokkos_Core.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/System/Vector2.hpp>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <vector>
// #include <cstdio>
// #include <cstdlib>
// #include <iostream>
// #include <tuple>

#define WINDOW_SIZE_X 1000
#define WINDOW_SIZE_Y 1000
#define GRID_RESOLUTION 10
#define G 1000000
#define FORCEMAX 10

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

    Kokkos::View<float *[4]>
    ParallelCalculateGravity(Kokkos::View<float *[3]> pointView,
                             Kokkos::View<float *[3]> objectView) {
        // point, force magnitude, force unit vector x, force unit vector y
        int pointCount = pointView.extent(0);
        int objectCount = objectView.extent(0);
        Kokkos::View<float *[4]> resultView("resultView",
                                            pointCount * objectCount);
        struct massObject {
            float mass;
            float posX;
            float posY;
        };

        Kokkos::parallel_for(
            "ParallelCalculateGravity",
            Kokkos::MDRangePolicy<Kokkos::Rank<2>>({0, 0},
                                                   {pointCount, objectCount}),
            KOKKOS_LAMBDA(const int &i, const int &j) {
                struct massObject point = {
                    .mass = pointView(i, 0),
                    .posX = pointView(i, 1),
                    .posY = pointView(i, 2),
                };
                struct massObject object = {
                    .mass = objectView(j, 0),
                    .posX = objectView(j, 1),
                    .posY = objectView(j, 2),
                };
                float diffX = point.posX - object.posX;
                float diffY = point.posY - object.posY;
                float r =
                    Kokkos::sqrt(Kokkos::pow(diffX, 2) + Kokkos::pow(diffY, 2));
                float force =
                    r ? (G * object.mass * point.mass) / Kokkos::pow(r, 2) : 0;
                float unitX = r ? diffX / r : 0;
                float unitY = r ? diffY / r : 0;
                // printf("calculate result M %f m %f diffX %f diffY %f r %f force %f\n",
                // object.mass, point.mass,
                    // diffX, diffY, r, force
                // );

                // Write result
                int resultIndex = i * j + j;
                resultView(resultIndex, 0) = i;
                resultView(resultIndex, 1) = force;
                resultView(resultIndex, 2) = unitX;
                resultView(resultIndex, 3) = unitY;
            });

        return resultView;
    }

    void ParallelUpdate(std::vector<MassedObject> massedObjects) {
        int pointCount = this->points.size();
        int objectCount = massedObjects.size();

        // mass, pos.x, pos.y
        Kokkos::View<float *[3]> pointView("pointView", pointCount);
        Kokkos::View<float *[3]> objectView("objectView", objectCount);
        auto pointHostView = Kokkos::create_mirror(pointView);
        auto objectHostView = Kokkos::create_mirror(objectView);

        for (int i = 0; i < pointCount; i++) {
            auto point = this->points[i].line[0];
            pointHostView(i, 0) = 1;
            pointHostView(i, 1) = point.position.x;
            pointHostView(i, 2) = point.position.y;
        }
        for (int i = 0; i < objectCount; i++) {
            auto object = massedObjects[i];
            objectHostView(i, 0) = object.mass;
            objectHostView(i, 1) = object.pos.x;
            objectHostView(i, 2) = object.pos.y;
        }

        Kokkos::deep_copy(pointView, pointHostView);
        Kokkos::deep_copy(objectView, objectHostView);
        // point, force magnitude, force unit vector x, force unit vector y
        auto resultView = ParallelCalculateGravity(pointView, objectView);
        auto resultHostView = Kokkos::create_mirror(resultView);
        Kokkos::deep_copy(resultHostView, resultView);

        // Calculate point ends
        // Simply add the forces to point starts
        struct pointEndReduce {
            float points[GRID_RESOLUTION * GRID_RESOLUTION][2];
        };

        struct pointEndReduce pointEnds;

        for (int i = 0; i < pointCount; i++) {
            pointEnds.points[i][0] = pointHostView(i, 1);
            pointEnds.points[i][1] = pointHostView(i, 2);
        }

        Kokkos::View<float *[2]> pointEndView("pointEndView", pointCount);
        auto pointEndHostView = Kokkos::create_mirror(pointEndView);

        Kokkos::parallel_for(
            "ParallelCalculate", Kokkos::RangePolicy(0, pointCount),
            KOKKOS_LAMBDA(const int &i) {
                float pointEndX = pointView(i, 1);
                float pointEndY = pointView(i, 2);
                for (int j = 0; j < objectCount; j++) {
                    int resultIndex = i * j + j;
                    float pointIndex = resultView(resultIndex, 0);
                    float force = resultView(resultIndex, 1);
                    float unitX = resultView(resultIndex, 2);
                    float unitY = resultView(resultIndex, 3);
                    // printf("found result(%f %f %f %f)\n", pointIndex, force, unitX, unitY);
                    pointEndX += unitX * Kokkos::fmin(force, FORCEMAX);
                    pointEndY += unitY * Kokkos::fmin(force, FORCEMAX);
                }
                pointEndView(i, 0) = pointEndX;
                pointEndView(i, 1) = pointEndY;
            });

        Kokkos::deep_copy(pointEndHostView, pointEndView);

        for (int i = 0; i < pointCount; i++) {
            float pointX = pointHostView(i, 1);
            float pointY = pointHostView(i, 2);
            float endX = pointEndHostView(i, 0);
            float endY = pointEndHostView(i, 1);

            auto pointEnd = &this->points[i].line[1];
            pointEnd->position.x = endX;
            pointEnd->position.y = endY;
        }

        Kokkos::fence();
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
    sf::RenderWindow window(sf::VideoMode(WINDOW_SIZE_X, WINDOW_SIZE_Y),
                            "simplesim");

    GravityGrid gravGrid(window.getSize().x, window.getSize().y,
                         GRID_RESOLUTION, GRID_RESOLUTION);
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
        // gravGrid.ParallelUpdate(objects);
        gravGrid.Draw(window);
        for (auto obj : objects) {
            window.draw(obj.shape);
        }
        window.display();
    }

    return 0;
}
