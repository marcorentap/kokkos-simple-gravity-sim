#include "SFML/Graphics/RenderWindow.hpp"
#include "simplesim/grid.hpp"
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
  float lineMax = 50;

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

  void Update(std::vector<MassedObject> massedObjects) {
    for (auto &point : this->points) {
      auto obj = massedObjects[0];
      auto force = CalculateGravity(obj, point);
      auto &startPos = point.line[0].position;
      auto &endPos = point.line[1].position;

      float diffX = obj.pos.x - startPos.x;
      float diffY = obj.pos.y - startPos.y;
      float dist = std::sqrt(std::pow(diffX, 2) + std::pow(diffY, 2));
      sf::Vector2f unit(diffX / dist, diffY / dist);
      endPos.x = startPos.x + unit.x * std::min(force, lineMax);
      endPos.y = startPos.y + unit.y * std::min(force, lineMax);

      std::cout << "point (" << startPos.x << ", " << startPos.y << ")-";
      std::cout << "point (" << endPos.x << ", " << endPos.y << ")";
      std::cout << " force " << force << std::endl;
    }
    return;
  }

  void PrintDebug() {
    for (auto &point : this->points) {
    }
  }
};

int main() {
  sf::RenderWindow window(sf::VideoMode(1000, 1000), "simplesim");

  GravityGrid gravGrid(window.getSize().x, window.getSize().y, 30, 30);
  MassedObject obj(10, 100, sf::Vector2f(300, 300));

  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed) {
        window.close();
      }
    }

    if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
      auto mousePos = sf::Mouse::getPosition(window);
      obj.SetPosition((sf::Vector2f)mousePos);
    }

    window.clear();
    gravGrid.Update({obj});
    gravGrid.Draw(window);
    window.draw(obj.shape);
    window.display();
  }

  return 0;
}
