#include <SFML/Graphics.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <cstdlib>
#include <iostream>
#include <tuple>


int main() {
    sf::RenderWindow window(sf::VideoMode(500, 500), "some window");
    sf::VertexArray lines(sf::Lines, 1000);
    for(int i = 0; i < 500; i++) {
        sf::Vector2f start((i%10)*50, (i/10)*50);
        sf::Vector2f end(start.x + random()%20, start.y + random()%20);
        lines.append(start);
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
        window.display();
    }
    return 0;
}
