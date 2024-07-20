#include <SFML/Graphics.hpp>
#include <iostream>
#include <complex>
#include <thread>
#include <vector>
#include <atomic>

static const int SCREEN_WIDTH = 1920;  
static const int SCREEN_HEIGHT = 1080;
static const int MAX_ITER = 20;     
static const float CHANGE_AMOUNT = 0.01f; 
static const std::chrono::milliseconds INPUT_DELAY(100); 

class PixelGrid {
public:
    sf::VertexArray pixels;
    int width;
    int height;
    std::complex<float> c;
    std::atomic<bool> updating;

    PixelGrid(int width, int height) : pixels(sf::Points, width * height), width(width), height(height), c(-0.7f, 0.27015f), updating(false) {
        generateJuliaSet();
    }

    void setConstant(std::complex<float> newC) {
        c = newC;
        updating = true;
        generateJuliaSet();
        updating = false;
    }

    void generateJuliaSet() {
        std::vector<std::thread> threads;
        int numThreads = std::thread::hardware_concurrency();
        int rowsPerThread = height / numThreads;

        auto computeSection = [&](int startRow, int endRow) {
            for (int i = startRow; i < endRow; ++i) {
                for (int j = 0; j < width; ++j) {
                    float x = (float)j / width * 4.0f - 2.0f;
                    float y = (float)i / height * 4.0f - 2.0f;
                    std::complex<float> z(x, y);
                    int iterations = 0;

                    while (std::abs(z) < 2.0f && iterations < MAX_ITER) {
                        z = z * z + c;
                        iterations++;
                    }

                    int index = i * width + j;
                    pixels[index].position = sf::Vector2f(j, i);
                    if (iterations == MAX_ITER) {
                        pixels[index].color = sf::Color::Black;
                    } else {
                        int hue = static_cast<int>(255.0f * iterations / MAX_ITER);
                        pixels[index].color = sf::Color(hue, 255 - hue, 255 - hue / 2);
                    }
                }
            }
        };

        for (int t = 0; t < numThreads; ++t) {
            int startRow = t * rowsPerThread;
            int endRow = (t == numThreads - 1) ? height : startRow + rowsPerThread;
            threads.emplace_back(computeSection, startRow, endRow);
        }

        for (auto& thread : threads) {
            thread.join();
        }
    }

    void draw(sf::RenderWindow& window) {
        window.draw(pixels);
    }
};

int main() {
    sf::RenderWindow window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "Julia Set Fractal");

    PixelGrid grid(SCREEN_WIDTH, SCREEN_HEIGHT);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        if (!grid.updating) {
            bool updated = false;
            std::complex<float> newC = grid.c;

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
		    std::cout << "lewy " << std::endl;
                newC.real(newC.real() - CHANGE_AMOUNT);
                updated = true;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
		    std::cout << "prawy " << std::endl;
                newC.real(newC.real() + CHANGE_AMOUNT);
                updated = true;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
		    std::cout << "gora" << std::endl;
                newC.imag(newC.imag() - CHANGE_AMOUNT);
                updated = true;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
		    std::cout << "dol" << std::endl;
                newC.imag(newC.imag() + CHANGE_AMOUNT);
                updated = true;
            }

            if (newC.real() < -2.0f) newC.real(-2.0f);
            if (newC.real() > 2.0f) newC.real(2.0f);
            if (newC.imag() < -2.0f) newC.imag(-2.0f);
            if (newC.imag() > 2.0f) newC.imag(2.0f);

            if (updated) {
                grid.setConstant(newC);
                std::this_thread::sleep_for(INPUT_DELAY);
            }
        }

        window.clear();
        grid.draw(window);
        window.display();
    }

    return 0;
}

