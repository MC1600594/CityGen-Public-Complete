#include <SFML/Graphics.hpp>
#include "CityGen.h"

#include <string>
#include <sstream>

int main()
{
    //seed rand
    srand(_time32(NULL));

    //set window size, position and framerate
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "CityGen");
    window.setPosition(sf::Vector2i(0, 5));
    window.setFramerateLimit(120);

    //setup the generator object
    CityGen generator;
    generator.AutomatedGenerate();

    //setup the clock
    float fpsTime = 0.f;
    const sf::Clock clock = sf::Clock::Clock();
    sf::Time previousTime = clock.getElapsedTime();
    sf::Time currentTime;

    //init font
    sf::Font font;
    if (!font.loadFromFile("media/sansation.ttf"))
    {
        std::cout << "font load failed" << std::endl;
        return 0;

    }
    
    //setup fps counter
    sf::Text fps;
    fps.setFont(font);
    fps.setCharacterSize(40);
    fps.setFillColor(sf::Color::Magenta);
    fps.setOutlineColor(sf::Color::Black);
    fps.setOutlineThickness(2.0f);
    fps.setPosition(5.0f, 5.0f);

    //main loop
    while (window.isOpen())
    {
        //init event for potential inputs
        sf::Event event{};

        //calculate fps
        currentTime = clock.getElapsedTime();
        fpsTime = 1.0f / (currentTime.asSeconds() - previousTime.asSeconds());
        previousTime = currentTime;

        //display fps
        std::ostringstream ss;
        ss << "fps: " << static_cast<int>(fpsTime);
        fps.setString(ss.str());

        //draw the generation
        window.clear(sf::Color(0, 120, 0));

        generator.Draw(window);
        window.draw(fps);

        window.display();

        //closing the window
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }
    }

    return 0;
}