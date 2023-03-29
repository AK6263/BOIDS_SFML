// #include "BoidSystem_with_copy.hpp" // With copy of a the boidVectors
#include "BoidSystem.hpp"
#include <SFML/Graphics.hpp>
#include <cstdlib>
#include "utils/fps.hpp"
#include <sstream>

FPS fps;
unsigned int WINDOW_WIDTH{1280}, WINDOW_HEIGHT{720};

int main(int argc, char const *argv[]) {
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "BOIDS");
    window.setFramerateLimit(60);

    BoidSystem boids(1000, WINDOW_WIDTH, WINDOW_HEIGHT);

    while (window.isOpen()) {
        sf::Event event;

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        boids.update(); // Update It

        // Draw It
        window.clear();
        window.draw(boids);
        window.display();

        // Check FPS
        fps.update();
        std::ostringstream buffer;
        buffer << fps.getFPS();
        window.setTitle(buffer.str());
    }
    return 0;
}
