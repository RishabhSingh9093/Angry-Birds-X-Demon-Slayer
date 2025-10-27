#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <string>
#include <iostream>

using namespace sf;
using namespace std;

struct Particle {
    CircleShape shape;
    Vector2f velocity;
    float lifetime;
};

int main() {
    srand(static_cast<unsigned>(time(0)));

    RenderWindow window(VideoMode(1280, 720), "Angry Bird x Demon Slayer");
    window.setFramerateLimit(60);

    //loading of all three maps for,snow,vol 
    Texture forestTex, snowTex, volcanoTex;
    forestTex.loadFromFile("assets/images/background.png");
    snowTex.loadFromFile("assets/images/snow.png");
    volcanoTex.loadFromFile("assets/images/volcano.png");

    Sprite forest(forestTex), snow(snowTex), volcano(volcanoTex);

    float windowWidth = window.getSize().x;
    float windowHeight = window.getSize().y;

    forest.setScale((windowWidth / 3.f) / forest.getLocalBounds().width,
                    (windowHeight / 3.f) / forest.getLocalBounds().height);
    snow.setScale((windowWidth / 3.f) / snow.getLocalBounds().width,
                  (windowHeight / 3.f) / snow.getLocalBounds().height);
    volcano.setScale((windowWidth / 3.f) / volcano.getLocalBounds().width,
                     (windowHeight / 3.f) / volcano.getLocalBounds().height);

    forest.setPosition(50.f, 50.f);
    snow.setPosition(windowWidth - snow.getGlobalBounds().width - 50.f, 50.f);
    volcano.setPosition(windowWidth / 2.f - volcano.getGlobalBounds().width / 2.f,
                        windowHeight - volcano.getGlobalBounds().height - 20.f);

    // ---------- TERMINAL INPUT ----------
    string playerName = "Player";
    int gameTime;
    string difficulty;

    cout << "==============================\n";
    cout << " Welcome to Angry Bird x Demon Slayer\n";
    cout << "==============================\n";
    cout << "Enter game time (in seconds): ";
    cin >> gameTime;
    cout << "Enter difficulty (Easy/Medium/Hard): ";
    cin >> difficulty;
    cout << "\nSelect a map by clicking on the window...\n";

    // ---------- MAP SELECTION ----------
    Sprite selectedMap;
    bool mapSelected = false;

    while (window.isOpen() && !mapSelected) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed)
                window.close();

            if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left) {
                Vector2f mouse = window.mapPixelToCoords(Mouse::getPosition(window));
                if (forest.getGlobalBounds().contains(mouse)) {
                    selectedMap.setTexture(forestTex);
                    mapSelected = true;
                } else if (snow.getGlobalBounds().contains(mouse)) {
                    selectedMap.setTexture(snowTex);
                    mapSelected = true;
                } else if (volcano.getGlobalBounds().contains(mouse)) {
                    selectedMap.setTexture(volcanoTex);
                    mapSelected = true;
                }

                if (mapSelected) {
                    selectedMap.setScale(windowWidth / selectedMap.getLocalBounds().width,
                                         windowHeight / selectedMap.getLocalBounds().height);
                }
            }
        }

        window.clear(Color::Black);
        window.draw(forest);
        window.draw(snow);
        window.draw(volcano);
        window.display();
    }

    if (!window.isOpen())
        return 0;

    // ---------- LOAD GAME ASSETS ----------
    Texture birdTex, demonTex, slingshotTex;
    birdTex.loadFromFile("assets/images/bird.png");
    demonTex.loadFromFile("assets/images/demon.gif");
    slingshotTex.loadFromFile("assets/images/slingshot.png");

    Sprite slingshot(slingshotTex);
    slingshot.setScale(0.05f, 0.05f);
    slingshot.setPosition(100.f, 600.f);

    Sprite bird(birdTex);
    bird.setScale(0.1f, 0.1f);
    Vector2f startPos(120.f, 580.f);
    bird.setPosition(startPos);

    // ---------- DEMONS ----------
    vector<Sprite> demons;
    Clock spawnClock;
    float spawnInterval = (difficulty == "Easy") ? 3.f : (difficulty == "Medium") ? 2.f : 1.f;
    int maxDemons = 5;

    // ---------- PHYSICS ----------
    Vector2f velocity(0.f, 0.f);
    float gravity = 0.4f;
    float launchPower = 0.25f;
    bool isLaunched = false, isSelected = false, isAiming = false;
    Clock clickClock;
    float minAimTime = 0.5f;

    // ---------- TRAJECTORY & TRAIL ----------
    vector<CircleShape> trajectoryDots;
    vector<Vector2f> birdTrail;

    // ---------- FONT ----------
    Font font;
    font.loadFromFile("assets/fonts/arial.ttf");

    // ---------- SCORE ----------
    int score = 0;
    Text scoreText;
    scoreText.setFont(font);
    scoreText.setCharacterSize(30);
    scoreText.setFillColor(Color::White);
    scoreText.setPosition(10.f, 10.f);

    // ---------- PARTICLES ----------
    vector<Particle> particles;

    // ---------- GAME TIMER ----------
    Clock gameClock;

    // ---------- MAIN GAME LOOP ----------
    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed)
                window.close();

            Vector2f mouse = window.mapPixelToCoords(Mouse::getPosition(window));

            if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left && !isLaunched) {
                if (!isSelected) {
                    if (bird.getGlobalBounds().contains(mouse)) {
                        isSelected = true;
                        isAiming = true;
                        clickClock.restart();
                    }
                } else if (clickClock.getElapsedTime().asSeconds() > minAimTime) {
                    Vector2f diff = startPos - mouse;
                    velocity = diff * launchPower;
                    isLaunched = true;
                    isSelected = false;
                    isAiming = false;
                    trajectoryDots.clear();
                    birdTrail.clear();
                }
            }
        }

        // ---------- SPAWN DEMONS ----------
        if (spawnClock.getElapsedTime().asSeconds() > spawnInterval) {
            if (demons.size() < maxDemons) {
                Sprite d(demonTex);
                d.setScale(0.3f, 0.3f);
                float randX = rand() % 1200;
                float randY = rand() % 600;
                d.setPosition(randX, randY);
                demons.push_back(d);
            }
            spawnClock.restart();
        }

        // ---------- AIMING TRAJECTORY ----------
        if (isAiming && !isLaunched) {
            Vector2f diff = startPos - window.mapPixelToCoords(Mouse::getPosition(window));
            Vector2f tempVel = diff * launchPower;
            trajectoryDots.clear();
            Vector2f tempPos = startPos;
            for (int i = 0; i < 30; i++) {
                tempVel.y += gravity * 0.5f;
                tempPos += tempVel * 0.1f;
                CircleShape dot(3);
                dot.setFillColor(Color(255, 255, 255, 180));
                dot.setPosition(tempPos);
                trajectoryDots.push_back(dot);
            }
            bird.setPosition(window.mapPixelToCoords(Mouse::getPosition(window)));
        }

        // ---------- UPDATE BIRD ----------
        if (isLaunched) {
            velocity.y += gravity;
            bird.move(velocity);
            birdTrail.push_back(bird.getPosition());
            if (birdTrail.size() > 20)
                birdTrail.erase(birdTrail.begin());

            if (bird.getPosition().y > 680) {
                bird.setPosition(startPos);
                velocity = Vector2f(0.f, 0.f);
                isLaunched = false;
                birdTrail.clear();
            }

            // Collisions
            for (size_t i = 0; i < demons.size(); i++) {
                if (bird.getGlobalBounds().intersects(demons[i].getGlobalBounds())) {
                    for (int p = 0; p < 10; p++) {
                        Particle particle;
                        particle.shape.setRadius(3);
                        particle.shape.setFillColor(Color::Red);
                        particle.shape.setPosition(demons[i].getPosition());
                        particle.velocity = Vector2f((rand() % 5 - 2) * 0.5f, (rand() % 5 - 2) * 0.5f);
                        particle.lifetime = 1.f;
                        particles.push_back(particle);
                    }
                    demons.erase(demons.begin() + i);
                    score += 100;
                    break;
                }
            }
        }

        // ---------- UPDATE PARTICLES ----------
        for (size_t i = 0; i < particles.size();) {
            particles[i].shape.move(particles[i].velocity);
            particles[i].lifetime -= 0.02f;
            Color c = particles[i].shape.getFillColor();
            c.a = static_cast<Uint8>(255 * particles[i].lifetime);
            particles[i].shape.setFillColor(c);
            if (particles[i].lifetime <= 0)
                particles.erase(particles.begin() + i);
            else
                ++i;
        }

        // ---------- TIMER ----------
        int elapsedTime = gameClock.getElapsedTime().asSeconds();
        if (elapsedTime >= gameTime) {
            Text gameOverText;
            gameOverText.setFont(font);
            gameOverText.setCharacterSize(50);
            gameOverText.setFillColor(Color::Red);
            gameOverText.setString("Time's Up! Score: " + to_string(score) + "\nPress R to Restart");
            gameOverText.setPosition(windowWidth / 2 - 300, windowHeight / 2 - 50);
            window.clear();
            window.draw(selectedMap);
            window.draw(gameOverText);
            window.display();

            bool waiting = true;
            while (waiting && window.isOpen()) {
                Event e;
                while (window.pollEvent(e)) {
                    if (e.type == Event::Closed)
                        window.close();
                    if (e.type == Event::KeyPressed && e.key.code == Keyboard::R)
                        waiting = false;
                }
            }
            gameClock.restart();
            score = 0;
            bird.setPosition(startPos);
            velocity = Vector2f(0.f, 0.f);
            isLaunched = false;
            birdTrail.clear();
            demons.clear();
            particles.clear();
            continue;
        }

        // ---------- DRAW ----------
        window.clear();
        window.draw(selectedMap);
        window.draw(slingshot);
        for (auto &d : demons)
            window.draw(d);
        for (auto &dot : trajectoryDots)
            window.draw(dot);
        for (auto &pos : birdTrail) {
            CircleShape trailDot(4);
            trailDot.setFillColor(Color(255, 255, 0, 100));
            trailDot.setPosition(pos);
            window.draw(trailDot);
        }
        window.draw(bird);
        for (auto &p : particles)
            window.draw(p.shape);

        scoreText.setString("Score: " + to_string(score) + " | Player: " + playerName +" | Time Left: " + to_string(gameTime - elapsedTime));
        window.draw(scoreText);

        window.display();
    }

    return 0;
}
