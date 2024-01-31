#include <Kokkos_Core_fwd.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <tuple>
#include <Kokkos_Core.hpp>

namespace ss {

class ParallelGrid {
  public:
    float xSize, ySize, xCount, yCount;
    // originx, originy, x, y
    Kokkos::View<float*[4], Kokkos::DefaultHostExecutionSpace> points;
    ParallelGrid(int xSize, int ySize, int xCount, int yCount);

    void PrintPoints() const;
    void InitializePoints();
    void Draw(sf::RenderWindow &window) const;
};

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
