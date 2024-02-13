// #include "SFML/Graphics/RenderWindow.hpp"
// #include "SFML/Window/Keyboard.hpp"
#include "SFML/Graphics/Shape.hpp"
#include "simplesim/grid.hpp"
#include <Kokkos_Core.hpp>
#include <Kokkos_Core_fwd.hpp>
#include <Kokkos_Macros.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/System/Vector2.hpp>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <impl/Kokkos_Profiling.hpp>
#include <random>
#include <vector>

#define WINDOW_SIZE_X 1000
#define WINDOW_SIZE_Y 1000
#define GRID_RESOLUTION 100
#define G 6.674 * (Kokkos::pow(10, -11))
#define FORCESCALE 14
#define FORCEMAXLINE 10
#define POINT_MASS 1
#define OBJ_MASS 100
#define OBJ_COUNT 10
#define OBJ_RADIUS 10
#define FPS_LIMIT 30

class GravityGrid : public ss::ParallelGrid {
  public:
    GravityGrid(int xSize, int ySize, int xCount, int yCount)
        : ss::ParallelGrid(xSize, ySize, xCount, yCount) {}

    void Update(
        Kokkos::View<float *[3], Kokkos::DefaultHostExecutionSpace> objects) {
        int pointCount = this->points.extent(0);
        int objectCount = objects.extent(0);
        // (point, object) => unitX, unitY, force
        Kokkos::View<float **[3], Kokkos::DefaultHostExecutionSpace>
            forceHostView("forceHostView", pointCount, objectCount);

        auto objectView =
            Kokkos::create_mirror(Kokkos::DefaultExecutionSpace(), objects);
        auto pointView = Kokkos::create_mirror(Kokkos::DefaultExecutionSpace(),
                                               this->points);
        auto forceView = Kokkos::create_mirror(Kokkos::DefaultExecutionSpace(),
                                               forceHostView);
        Kokkos::deep_copy(objectView, objects);
        Kokkos::deep_copy(pointView, points);
        Kokkos::deep_copy(forceView, forceHostView);

        Kokkos::parallel_for(
            "GravityGrid::Update Calculate Unit Vector",
            Kokkos::MDRangePolicy<Kokkos::Rank<2>>({0, 0},
                                                   {pointCount, objectCount}),
            KOKKOS_LAMBDA(const int &i, const int &j) {
                float objMass = objectView(j, 2);
                float pointMass = POINT_MASS;
                float distX = objectView(j, 0) - pointView(i, 0);
                float distY = objectView(j, 1) - pointView(i, 1);
                float dist =
                    Kokkos::sqrt(Kokkos::pow(distX, 2) + Kokkos::pow(distY, 2));
                forceView(i, j, 0) = dist ? distX / dist : 0;
                forceView(i, j, 1) = dist ? distY / dist : 0;
                forceView(i, j, 2) = dist ? Kokkos::pow(10, FORCESCALE) *
                                                (G * objMass * pointMass) /
                                                Kokkos::pow(dist, 2)
                                          : 0;
            });
        Kokkos::deep_copy(forceHostView, forceView);

        // Kokkosify, create view<[pointCount][objCount]> then sum each col to
        // ith point
        for (int i = 0; i < pointCount; i++) {
            this->points(i, 2) = this->points(i, 0);
            this->points(i, 3) = this->points(i, 1);
            for (int j = 0; j < objectCount; j++) {
                float unitX = forceHostView(i, j, 0);
                float unitY = forceHostView(i, j, 1);
                float dist =
                    Kokkos::sqrt(Kokkos::pow(unitX, 2) + Kokkos::pow(unitY, 2));
                float force = forceHostView(i, j, 2);
                this->points(i, 2) += unitX * Kokkos::fmin(FORCEMAXLINE, force);
                this->points(i, 3) += unitY * Kokkos::fmin(FORCEMAXLINE, force);
            }
        }
        return;
    }
};

int main(int argc, char *argv[]) {
    Kokkos::ScopeGuard guard(argc, argv);
    Kokkos::Profiling::pushRegion("Initialize Simulation");
    sf::RenderWindow window(sf::VideoMode(WINDOW_SIZE_X, WINDOW_SIZE_Y),
                            "simplesim");
    window.setFramerateLimit(FPS_LIMIT);
    GravityGrid grid(WINDOW_SIZE_X, WINDOW_SIZE_Y, GRID_RESOLUTION,
                     GRID_RESOLUTION);
    // posX, posY, mass
    Kokkos::View<float *[3], Kokkos::DefaultHostExecutionSpace> objects(
        "objects", OBJ_COUNT);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> positionRand(100, WINDOW_SIZE_X - 100);
    for (int i = 0; i < OBJ_COUNT; i++) {
        objects(i, 0) = positionRand(gen);
        objects(i, 1) = positionRand(gen);
        objects(i, 2) = OBJ_MASS;
    }
    Kokkos::Profiling::popRegion();

    int selectedObject = -1;
    Kokkos::Profiling::pushRegion("SFML Main Loop");
    while (window.isOpen()) {
        sf::Event event;
        Kokkos::Profiling::pushRegion("SFML Event Check");
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (event.type == sf::Event::MouseButtonPressed) {
                // Left click selects an object
                if (event.mouseButton.button == sf::Mouse::Left) {
                    auto mousePos = sf::Mouse::getPosition(window);
                    for (int i = 0; i < OBJ_COUNT; i++) {
                        float objX = objects(i, 0);
                        float objY = objects(i, 1);
                        if (objX < mousePos.x + OBJ_RADIUS &&
                            objY < mousePos.y + OBJ_RADIUS &&
                            objX > mousePos.x - OBJ_RADIUS &&
                            objY > mousePos.y - OBJ_RADIUS) {
                            selectedObject = i;
                            printf("Selected obj(%f, %f)\n", objX, objY);
                            break;
                        }
                    }
                }

                // Right click deselect object
                if (event.mouseButton.button == sf::Mouse::Right) {
                    selectedObject = -1;
                    printf("Removed object selection\n");
                }
            }
        }

        // object drag
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            auto mousePos = sf::Mouse::getPosition(window);
            objects(selectedObject, 0) = mousePos.x;
            objects(selectedObject, 1) = mousePos.y;
        }
        Kokkos::Profiling::popRegion();

        Kokkos::Profiling::pushRegion("SFML Frame Update");
        window.clear();
        grid.Update(objects);
        for (int i = 0; i < OBJ_COUNT; i++) {
            sf::CircleShape circle(10, 10);
            circle.setPosition(objects(i, 0), objects(i, 1));
            circle.setOrigin(10, 10);
            window.draw(circle);
        }
        grid.Draw(window);
        window.display();
        Kokkos::Profiling::popRegion();
    }
    Kokkos::Profiling::popRegion();

    return 0;
}
