#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>

const int WIDTH = 800;
const int HEIGHT = 600;
const int GRID_SIZE = 20;

enum class GameState { MainMenu, Playing, GameOver };
enum class Difficulty { Easy, Medium, Hard };

class Snake {
public:
    Snake(int initialSize) {
        for (int i = 0; i < initialSize; ++i) {
            body.push_back(sf::RectangleShape(sf::Vector2f(GRID_SIZE, GRID_SIZE)));
            body[i].setFillColor(i == 0 ? sf::Color::Yellow : sf::Color::Green);
            body[i].setPosition(WIDTH / 2 - i * GRID_SIZE, HEIGHT / 2);
        }
        direction = sf::Vector2i(1, 0);
    }

    void move() {
        for (int i = body.size() - 1; i > 0; --i) {
            body[i].setPosition(body[i - 1].getPosition());
        }
        body[0].move(direction.x * GRID_SIZE, direction.y * GRID_SIZE);
    }

    void grow() {
        sf::RectangleShape newPart(sf::Vector2f(GRID_SIZE, GRID_SIZE));
        newPart.setFillColor(sf::Color::Green);
        newPart.setPosition(body.back().getPosition());
        body.push_back(newPart);
    }

    void setDirection(sf::Vector2i newDir) {
        if (newDir.x != -direction.x || newDir.y != -direction.y) {
            direction = newDir;
        }
    }

    bool checkCollision() {
        sf::Vector2f head = body[0].getPosition();
        if (head.x < GRID_SIZE || head.x >= WIDTH - GRID_SIZE || head.y < GRID_SIZE || head.y >= HEIGHT - GRID_SIZE) {
            return true;
        }
        for (size_t i = 1; i < body.size(); ++i) {
            if (body[i].getPosition() == head) {
                return true;
            }
        }
        return false;
    }

    bool checkAppleCollision(sf::Vector2f applePos) {
        return body[0].getPosition() == applePos;
    }

    void draw(sf::RenderWindow& window) {
        for (auto& part : body) {
            window.draw(part);
        }
    }

private:
    std::vector<sf::RectangleShape> body;
    sf::Vector2i direction;
};

class Apple {
public:
    Apple() {
        shape.setRadius(GRID_SIZE / 2);
        shape.setFillColor(sf::Color::Red);
        spawn();
    }

    void spawn() {
        int x = (rand() % ((WIDTH - 2 * GRID_SIZE) / GRID_SIZE)) * GRID_SIZE + GRID_SIZE;
        int y = (rand() % ((HEIGHT - 2 * GRID_SIZE) / GRID_SIZE)) * GRID_SIZE + GRID_SIZE;
        shape.setPosition(x, y);
    }

    void draw(sf::RenderWindow& window) {
        window.draw(shape);
    }

    sf::Vector2f getPosition() {
        return shape.getPosition();
    }

private:
    sf::CircleShape shape;
};

class Game {
public:
    Game() : window(sf::VideoMode(WIDTH, HEIGHT), "Snake Game"), snake(3) {
        window.setFramerateLimit(10);
        if (!font.loadFromFile("Poppins-Bold.ttf")) {
            std::cerr << "Failed to load font!" << std::endl;
        }
        setupText(titleText, "Snake Game", 50, sf::Color::Green);
        titleText.setPosition(WIDTH / 2 - titleText.getLocalBounds().width / 2, 50);

        setupText(playText, "Play", 30, sf::Color::White);
        playText.setPosition(WIDTH / 2 - playText.getLocalBounds().width / 2, 200);

        setupText(difficultyText, "Difficulty: Easy", 30, sf::Color::White);
        difficultyText.setPosition(WIDTH / 2 - difficultyText.getLocalBounds().width / 2, 250);

        setupText(scoreText, "Score: 0", 20, sf::Color::White);
        scoreText.setPosition(10, 10);

        setupText(highScoreText, "", 20, sf::Color::Yellow);
        highScoreText.setPosition(WIDTH / 2 - highScoreText.getLocalBounds().width / 2, 300);

        setupWalls();
        loadHighScores();
        updateHighScoreText();
    }

    void run() {
        while (window.isOpen()) {
            handleEvents();
            update();
            render();
        }
    }

private:
    sf::RenderWindow window;
    Snake snake;
    Apple apple;
    int score = 0;
    GameState gameState = GameState::MainMenu;
    Difficulty difficulty = Difficulty::Easy;
    sf::Font font;
    sf::Text titleText, playText, difficultyText, scoreText, highScoreText;
    std::vector<sf::RectangleShape> walls;
    std::vector<int> highScores = { 0, 0, 0 }; // Easy, Medium, Hard

    void setupText(sf::Text& text, const std::string& string, int size, const sf::Color& color) {
        text.setFont(font);
        text.setString(string);
        text.setCharacterSize(size);
        text.setFillColor(color);
    }

    void setupWalls() {
        sf::RectangleShape wall;
        wall.setFillColor(sf::Color(100, 100, 100));

        // Top wall
        wall.setSize(sf::Vector2f(WIDTH, GRID_SIZE));
        wall.setPosition(0, 0);
        walls.push_back(wall);

        // Bottom wall
        wall.setPosition(0, HEIGHT - GRID_SIZE);
        walls.push_back(wall);

        // Left wall
        wall.setSize(sf::Vector2f(GRID_SIZE, HEIGHT));
        wall.setPosition(0, 0);
        walls.push_back(wall);

        // Right wall
        wall.setPosition(WIDTH - GRID_SIZE, 0);
        walls.push_back(wall);
    }

    void loadHighScores() {
        std::ifstream file("highscores.txt");
        if (file.is_open()) {
            for (int i = 0; i < 3; ++i) {
                file >> highScores[i];
            }
            file.close();
        }
    }

    void saveHighScores() {
        std::ofstream file("highscores.txt");
        if (file.is_open()) {
            for (int score : highScores) {
                file << score << std::endl;
            }
            file.close();
        }
    }

    void updateHighScoreText() {
        std::string difficultyStr;
        int currentHighScore;
        switch (difficulty) {
        case Difficulty::Easy:
            difficultyStr = "Easy";
            currentHighScore = highScores[0];
            break;
        case Difficulty::Medium:
            difficultyStr = "Medium";
            currentHighScore = highScores[1];
            break;
        case Difficulty::Hard:
            difficultyStr = "Hard";
            currentHighScore = highScores[2];
            break;
        }
        highScoreText.setString("High Score (" + difficultyStr + "): " + std::to_string(currentHighScore));
        highScoreText.setPosition(WIDTH / 2 - highScoreText.getLocalBounds().width / 2, 300);
    }

    void handleEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (event.type == sf::Event::KeyPressed) {
                handleKeyPress(event.key.code);
            }
            if (event.type == sf::Event::MouseButtonPressed) {
                handleMouseClick(event.mouseButton);
            }
        }
    }

    void handleKeyPress(sf::Keyboard::Key key) {
        if (gameState == GameState::Playing) {
            if (key == sf::Keyboard::Up) {
                snake.setDirection(sf::Vector2i(0, -1));
            }
            else if (key == sf::Keyboard::Down) {
                snake.setDirection(sf::Vector2i(0, 1));
            }
            else if (key == sf::Keyboard::Left) {
                snake.setDirection(sf::Vector2i(-1, 0));
            }
            else if (key == sf::Keyboard::Right) {
                snake.setDirection(sf::Vector2i(1, 0));
            }
        }
        else if (gameState == GameState::MainMenu) {
            if (key == sf::Keyboard::Left || key == sf::Keyboard::Right) {
                changeDifficulty();
            }
        }
    }

    void handleMouseClick(const sf::Event::MouseButtonEvent& mouseButton) {
        if (gameState == GameState::MainMenu) {
            if (playText.getGlobalBounds().contains(mouseButton.x, mouseButton.y)) {
                startGame();
            }
            else if (difficultyText.getGlobalBounds().contains(mouseButton.x, mouseButton.y)) {
                changeDifficulty();
            }
        }
        else if (gameState == GameState::GameOver) {
            gameState = GameState::MainMenu;
            score = 0;
        }
    }

    void changeDifficulty() {
        switch (difficulty) {
        case Difficulty::Easy:
            difficulty = Difficulty::Medium;
            difficultyText.setString("Difficulty: Medium");
            break;
        case Difficulty::Medium:
            difficulty = Difficulty::Hard;
            difficultyText.setString("Difficulty: Hard");
            break;
        case Difficulty::Hard:
            difficulty = Difficulty::Easy;
            difficultyText.setString("Difficulty: Easy");
            break;
        }
        difficultyText.setPosition(WIDTH / 2 - difficultyText.getLocalBounds().width / 2, 250);
        updateHighScoreText();
    }

    void startGame() {
        gameState = GameState::Playing;
        int initialSize;
        switch (difficulty) {
        case Difficulty::Easy:
            initialSize = 3;
            window.setFramerateLimit(10);
            break;
        case Difficulty::Medium:
            initialSize = 5;
            window.setFramerateLimit(15);
            break;
        case Difficulty::Hard:
            initialSize = 7;
            window.setFramerateLimit(20);
            break;
        }
        snake = Snake(initialSize);
        apple.spawn();
        score = 0;
    }

    void update() {
        if (gameState == GameState::Playing) {
            snake.move();
            if (snake.checkCollision()) {
                gameState = GameState::GameOver;
                updateHighScore();
            }
            if (snake.checkAppleCollision(apple.getPosition())) {
                score++;
                snake.grow();
                apple.spawn();
            }
            scoreText.setString("Score: " + std::to_string(score));
        }
    }

    void updateHighScore() {
        int difficultyIndex;
        switch (difficulty) {
        case Difficulty::Easy: difficultyIndex = 0; break;
        case Difficulty::Medium: difficultyIndex = 1; break;
        case Difficulty::Hard: difficultyIndex = 2; break;
        }
        if (score > highScores[difficultyIndex]) {
            highScores[difficultyIndex] = score;
            saveHighScores();
            updateHighScoreText();
        }
    }

    void render() {
        window.clear(sf::Color::Black);

        for (const auto& wall : walls) {
            window.draw(wall);
        }

        if (gameState == GameState::MainMenu) {
            window.draw(titleText);
            window.draw(playText);
            window.draw(difficultyText);
            window.draw(highScoreText);
        }
        else if (gameState == GameState::Playing) {
            snake.draw(window);
            apple.draw(window);
            window.draw(scoreText);
        }
        else if (gameState == GameState::GameOver) {
            sf::Text gameOverText;
            setupText(gameOverText, "Game Over!\nFinal Score: " + std::to_string(score) + "\nClick to return to menu", 30, sf::Color::Red);
            gameOverText.setPosition(WIDTH / 2 - gameOverText.getLocalBounds().width / 2,
                HEIGHT / 2 - gameOverText.getLocalBounds().height / 2);
            window.draw(gameOverText);
        }

        window.display();
    }
};

int main() {
    srand(static_cast<unsigned>(time(0)));
    Game game;
    game.run();
    return 0;
}