#include "simplesim/grid.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include <Kokkos_Core_fwd.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <tuple>

namespace ss {

void ParallelGrid::PrintPoints() const {
    for (int i = 0; i < this->points.extent(0); i++) {
        printf("Found points (%f, %f)\n", this->points(i, 0),
               this->points(i, 1));
    }
}

void ParallelGrid::Draw(sf::RenderWindow &window) const {
    int pointCount = xCount * yCount;
    for (int i = 0; i < pointCount; i++) {
        sf::CircleShape circle(1);
        circle.setPosition(this->points(i, 0), this->points(i, 1));
        circle.setOrigin(1, 1);

        sf::VertexArray line(sf::Lines);
        line.append(sf::Vector2f(this->points(i, 0), this->points(i, 1)));
        line.append(sf::Vector2f(this->points(i, 2), this->points(i, 3)));

        // window.draw(circle);
        window.draw(line);
    }
}

void ParallelGrid::InitializePoints() {
    // 10 border
    float xSpace = (xSize - 20) / (xCount - 1);
    float ySpace = (ySize - 20) / (yCount - 1);
    Kokkos::parallel_for(
        "InitializePoints",
        Kokkos::MDRangePolicy<Kokkos::Rank<2>,
                              Kokkos::DefaultHostExecutionSpace>(
            {0, 0}, {(int)this->xCount, (int)this->yCount}),
        KOKKOS_LAMBDA(const int &x, const int &y) {
            int pointIndex = (xCount * y) + x;
            this->points(pointIndex, 0) = 10 + (x * xSpace);
            this->points(pointIndex, 1) = 10 + (y * ySpace);
            this->points(pointIndex, 2) = 0;
            this->points(pointIndex, 3) = 0;
        });
    Kokkos::fence();
}

ParallelGrid::ParallelGrid(int xSize, int ySize, int xCount, int yCount)
    : xSize(xSize), ySize(ySize), xCount(xCount), yCount(yCount),
      points("ParallelGrid::Point", xCount * yCount) {
    InitializePoints();
}

PointGrid::PointGrid(int xSize, int ySize, int xCount, int yCount) {
    // 10 border
    int xSpace = (xSize - 20) / (xCount - 1);
    int ySpace = (ySize - 20) / (yCount - 1);

    for (int x = 10; x <= xSize - 10; x += xSpace) {
        for (int y = 10; y <= ySize - 10; y += ySpace) {
            GridPoint point;
            point.point = sf::CircleShape(1);
            point.point.setPosition(x, y);
            point.point.setOrigin(1, 1);

            point.line = sf::VertexArray(sf::Lines);
            point.line.append(sf::Vector2f(x, y));
            point.line.append(sf::Vector2f(x, y));
            this->points.push_back(point);
        }
    }
}

void PointGrid::Draw(sf::RenderWindow &window) const {
    for (auto const &point : this->points) {
        window.draw(point.point);
        window.draw(point.line);
    }
}

} // namespace ss
