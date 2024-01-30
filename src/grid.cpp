#include "simplesim/grid.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <tuple>

namespace ss {

PointGrid::PointGrid(int xSize, int ySize, int xCount, int yCount) {
    // 10 border
    int xSpace = (xSize - 20) / xCount;
    int ySpace = (ySize - 20) / yCount;

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
