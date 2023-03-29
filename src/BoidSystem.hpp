#include <SFML/Graphics.hpp>
#include <iostream>
#include <random>
#include <vector>

class BoidSystem : public sf::Drawable, public sf::Transformable {
private:
    struct Boid {
        sf::CircleShape circle;
        sf::Vector2f velocity;
        int index;
    };

    enum FishType {
        small,
        large
    };

    float VISUAL_RANGE{75}, CENTERING_FACTOR{0.005};
    float MIN_DISTANCE{20}, AVOID_FACTOR{0.05};
    float MATCHING_FACTOR{0.05};
    float SPEED_LIMIT{5};

    float TURNFACTOR{.5}, MARGIN{50};

    float VELOCITY_MULTIPLIER{5}; // For visualizing velocity vectors
    int BOID_COUNT;
    float CIRCLE_RADIUS{5.f};
    unsigned int WINDOW_WIDTH, WINDOW_HEIGHT;
    std::vector<Boid> m_boids;

public:
    BoidSystem(int BOID_COUNT,
               unsigned int WINDOW_WIDTH,
               unsigned int WINDOW_HEIGHT) : BOID_COUNT(BOID_COUNT),
                                             WINDOW_WIDTH(WINDOW_WIDTH),
                                             WINDOW_HEIGHT(WINDOW_HEIGHT) {
        initiateBoidSystem();
    }

    virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const {
        for (auto &boid : m_boids) {
            target.draw(boid.circle);

            // Drawing Line Vertex
            sf::Vertex line[] = {
                sf::Vertex(boid.circle.getPosition()),
                sf::Vertex(VELOCITY_MULTIPLIER * boid.velocity + boid.circle.getPosition())};

            target.draw(line, 2, sf::Lines);

            if (boid.index == 0) {
                sf::CircleShape rangecircle(VISUAL_RANGE);
                rangecircle.setPosition(boid.circle.getPosition());
                rangecircle.setOutlineColor(sf::Color::Green);
                rangecircle.setOutlineThickness(1);
                rangecircle.setFillColor(sf::Color::Transparent);
                rangecircle.setOrigin(VISUAL_RANGE, VISUAL_RANGE);

                target.draw(rangecircle);
            }
        }
    }
    // OLD Method but still works though the proper way is given above
    // void renderWindow(sf::RenderWindow &window) {
    //     for (Boid &boid : m_boids) {
    //         window.draw(boid.circle);
    //     }
    // }

    void initiateBoidSystem() {
        std::random_device rng;
        std::mt19937 gen(rng());
        std::uniform_real_distribution<> velocityDist(-SPEED_LIMIT, SPEED_LIMIT);
        std::uniform_real_distribution<> positionDist(0, 1);

        float position_X, position_Y, velocity_dx, velocity_dy;

        std::cout << "Initiating Boids : ";
        for (int i = 0; i < BOID_COUNT; ++i) {
            position_X = positionDist(gen);
            position_Y = positionDist(gen);

            velocity_dx = velocityDist(gen);
            velocity_dy = velocityDist(gen);
            Boid new_boid;
            new_boid.index = i;
            new_boid.circle.setPosition(position_X * WINDOW_WIDTH, position_Y * WINDOW_HEIGHT);
            new_boid.circle.setRadius(CIRCLE_RADIUS);
            new_boid.circle.setOrigin(CIRCLE_RADIUS, CIRCLE_RADIUS);
            new_boid.circle.setFillColor(sf::Color::Yellow);
            new_boid.velocity.x = velocity_dx;
            new_boid.velocity.y = velocity_dy;

            m_boids.push_back(new_boid);
        }
        std::cout << "Complete" << std::endl;
    }

    void borderWrapping(Boid &boid) {
        sf::Vector2f position = boid.circle.getPosition();
        if (position.x < 0)
            boid.circle.setPosition(position.x + WINDOW_WIDTH, position.y);
        else if (position.x > WINDOW_WIDTH)
            boid.circle.setPosition(position.x - WINDOW_WIDTH, position.y);

        if (position.y < 0)
            boid.circle.setPosition(position.x, position.y + WINDOW_HEIGHT);
        else if (position.y > WINDOW_HEIGHT)
            boid.circle.setPosition(position.x, position.y - WINDOW_HEIGHT);
    }

    void borderAvoidance(Boid &boid) {
        sf::Vector2f position = boid.circle.getPosition();
        if (position.x < MARGIN)
            boid.velocity.x += TURNFACTOR;
        else if (position.x > WINDOW_WIDTH - MARGIN)
            boid.velocity.x -= TURNFACTOR;

        if (position.y < MARGIN)
            boid.velocity.y += TURNFACTOR;
        else if (position.y > WINDOW_HEIGHT - MARGIN)
            boid.velocity.y -= TURNFACTOR;
    }

    float distance(Boid &b1, Boid &b2) {
        sf::Vector2f p1 = b1.circle.getPosition();
        sf::Vector2f p2 = b2.circle.getPosition();
        return std::sqrt((p1.x - p2.x) * (p1.x - p2.x) +
                         (p1.y - p2.y) * (p1.y - p2.y));
    }

    void seekCenter(Boid &boid) {
        sf::Vector2f center(0, 0);
        int numNeighbours = 0;
        for (auto &otherBoid : m_boids) {
            if (distance(otherBoid, boid) < VISUAL_RANGE) {
                center += otherBoid.circle.getPosition();
                ++numNeighbours;
            }
        }

        if (numNeighbours) {
            center.x /= numNeighbours;
            center.y /= numNeighbours;

            sf::Vector2f turn = (center - boid.circle.getPosition());

            boid.velocity.x += (CENTERING_FACTOR)*turn.x;
            boid.velocity.y += (CENTERING_FACTOR)*turn.y;
        }
    }

    void avoidOtherBoidS(Boid &boid) {
        sf::Vector2f moveAway(0, 0);

        for (auto &otherBoid : m_boids) {
            if (distance(otherBoid, boid) < MIN_DISTANCE) {
                moveAway += (boid.circle.getPosition() - otherBoid.circle.getPosition());
            }
        }
        boid.velocity += moveAway * AVOID_FACTOR;
    }

    void matchVelocity(Boid &boid) {
        sf::Vector2f avgVelocity(0, 0);
        int numNeighbours = 0;

        for (auto &otherBoid : m_boids) {
            if (distance(otherBoid, boid) < VISUAL_RANGE) {
                avgVelocity += otherBoid.velocity;
                ++numNeighbours;
            }
        }
        if (numNeighbours) {
            avgVelocity.x /= numNeighbours;
            avgVelocity.y /= numNeighbours;

            boid.velocity.x += (avgVelocity.x - boid.velocity.x) * MATCHING_FACTOR;
            boid.velocity.y += (avgVelocity.y - boid.velocity.y) * MATCHING_FACTOR;
        }
    }

    void limitSpeed(Boid &boid) {
        int speed = std::sqrt(boid.velocity.x * boid.velocity.x +
                              boid.velocity.y * boid.velocity.y);

        if (speed > SPEED_LIMIT) {
            boid.velocity.x = (boid.velocity.x / speed) * SPEED_LIMIT;
            boid.velocity.y = (boid.velocity.y / speed) * SPEED_LIMIT;
        }
    }

    void update() {

        for (int i = 0; i < BOID_COUNT; ++i) {

            Boid &boid = m_boids[i];
            seekCenter(boid);
            avoidOtherBoidS(boid);
            matchVelocity(boid);
            limitSpeed(boid);
            // borderWrapping(boid);
            borderAvoidance(boid);

            sf::Vector2f newPosition = boid.circle.getPosition() + boid.velocity;
            boid.circle.setPosition(newPosition);
        }
    }
};
