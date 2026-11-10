// ParticleSimulation.cpp
// Requires SFML library
// Installation: https://www.sfml-dev.org/download.php
// Compile with: g++ ParticleSimulation.cpp -o ParticleSimulation -lsfml-graphics -lsfml-window -lsfml-system

#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <random>
#include <string>

struct Particle {
    sf::Vector2f position;
    sf::Vector2f velocity;
    float radius;
    sf::Color color;
};

int main() {
    const unsigned int WINDOW_WIDTH = 1200;
    const unsigned int WINDOW_HEIGHT = 800;
    const int PARTICLE_COUNT = 64;
    const float REPULSION_STRENGTH = 80000.0f; // Adjust for stronger/weaker repulsion
    const float MIN_DISTANCE = 5.0f;           // Prevents infinite forces at very close range
    const float DAMPING = 0.995f;              // Velocity damping to keep things stable
    const float MAX_SPEED = 400.0f;            // Cap speed to avoid explosions

    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Particle Repulsion Simulation");
    window.setFramerateLimit(60);

    // Random setup
    std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)));
    std::uniform_real_distribution<float> xDist(50.0f, WINDOW_WIDTH - 50.0f);
    std::uniform_real_distribution<float> yDist(50.0f, WINDOW_HEIGHT - 50.0f);
    std::uniform_real_distribution<float> velDist(-50.0f, 50.0f);
    std::uniform_real_distribution<float> radDist(4.0f, 8.0f);
    std::uniform_int_distribution<int> colorDist(100, 255);

    std::vector<Particle> particles(PARTICLE_COUNT);
    for (auto& p : particles) {
        p.position = sf::Vector2f(xDist(rng), yDist(rng));
        p.velocity = sf::Vector2f(velDist(rng), velDist(rng));
        p.radius = radDist(rng);
        p.color = sf::Color(
            static_cast<sf::Uint8>(colorDist(rng)),
            static_cast<sf::Uint8>(colorDist(rng)),
            static_cast<sf::Uint8>(colorDist(rng))
        );
    }

    sf::CircleShape shape;
    sf::Clock clock;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        float dt = clock.restart().asSeconds();
        if (dt > 0.05f) dt = 0.05f; // Clamp dt to avoid instability on lag

        // Compute forces (repulsion between all pairs)
        for (size_t i = 0; i < particles.size(); ++i) {
            sf::Vector2f force(0.0f, 0.0f);

            for (size_t j = 0; j < particles.size(); ++j) {
                if (i == j) continue;

                sf::Vector2f diff = particles[i].position - particles[j].position;
                float distSq = diff.x * diff.x + diff.y * diff.y;

                if (distSq < MIN_DISTANCE * MIN_DISTANCE)
                    distSq = MIN_DISTANCE * MIN_DISTANCE;

                float dist = std::sqrt(distSq);
                sf::Vector2f dir = diff / dist;

                // Inverse-square repulsion force: F = k / d^2
                float forceMag = REPULSION_STRENGTH / distSq;
                force += dir * forceMag;
            }

            // Apply force as acceleration (mass = 1)
            particles[i].velocity += force * dt;
        }

        // Integrate positions & handle boundaries
        for (auto& p : particles) {
            // Damping
            p.velocity *= DAMPING;

            // Speed limit
            float speed = std::sqrt(p.velocity.x * p.velocity.x + p.velocity.y * p.velocity.y);
            if (speed > MAX_SPEED) {
                p.velocity = (p.velocity / speed) * MAX_SPEED;
            }

            p.position += p.velocity * dt;

            // Bounce off walls
            if (p.position.x - p.radius < 0.0f) {
                p.position.x = p.radius;
                p.velocity.x = -p.velocity.x;
            }
            if (p.position.x + p.radius > WINDOW_WIDTH) {
                p.position.x = WINDOW_WIDTH - p.radius;
                p.velocity.x = -p.velocity.x;
            }
            if (p.position.y - p.radius < 0.0f) {
                p.position.y = p.radius;
                p.velocity.y = -p.velocity.y;
            }
            if (p.position.y + p.radius > WINDOW_HEIGHT) {
                p.position.y = WINDOW_HEIGHT - p.radius;
                p.velocity.y = -p.velocity.y;
            }
        }

        // Render
        window.clear(sf::Color(15, 15, 25));

        for (const auto& p : particles) {
            shape.setRadius(p.radius);
            shape.setOrigin(p.radius, p.radius);
            shape.setPosition(p.position);
            shape.setFillColor(p.color);
            window.draw(shape);
        }

        window.display();
    }

    return 0;
}