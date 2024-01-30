#include <SFML/Graphics.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <tuple>

namespace ss {

class GridPoint {
  public:
    GridPoint(sf::VertexArray line, sf::CircleShape point)
        : line(line), point(point) {}
    GridPoint() {}
    sf::VertexArray line;
    sf::CircleShape point;
};

class PointGrid {
  public:
    std::vector<GridPoint> points;
    PointGrid(int xSize, int ySize, int xCount, int yCount);
    void Draw(sf::RenderWindow &window) const;
};

} // namespace ss
