#include <SFML/Graphics.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <tuple>

// :)
#define G 100000

struct object {
    sf::Vector2f point;
    float mass;
};

float CalcGravityForce(float M, float m, float r) {
    return r ? (G * M * m) / (r * r) : 0;
}

sf::Vector2f CalcGravityForceVector(struct object &obj1, struct object &obj2) {
    float x = CalcGravityForce(obj1.mass, obj2.mass, obj1.point.x - obj2.point.x);
    float y = CalcGravityForce(obj1.mass, obj2.mass, obj1.point.y - obj2.point.y);
    return sf::Vector2f(x, y);
}

int main() {
    sf::RenderWindow window(sf::VideoMode(500, 500), "some window");
    sf::VertexArray lines(sf::Lines, 1000);
    struct object thing = {
        .point = sf::Vector2f(300, 300),
        .mass = 10
    };
    sf::CircleShape circle(thing.point.x, thing.point.y);

    for(int i = 0; i < 500; i++) {
        struct object start = {
            .point = sf::Vector2f((i%10)*50, (i/10)*50),
            .mass = 1
        };
        if (start.point.x > 500 || start.point.y > 500) break;

        sf::Vector2f force = CalcGravityForceVector(thing, start);
        sf::Vector2f end(start.point.x + force.x, start.point.y + force.y);
        printf("point (%f, %f) has force (%f, %f)\n", start.point.x, start.point.y, force.x, force.y);

        lines.append(start.point);
        lines.append(end);
    }
 

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }
        window.clear();
    window.draw(lines);
    window.draw(circle);
        window.display();
    }
    return 0;
}
