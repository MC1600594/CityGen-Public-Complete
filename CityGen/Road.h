#pragma once
#include "SFML/Graphics.hpp"
#include "SFML/System.hpp"
#include <vector>
#include <array>

//based on sfLine by SFML at: https://github.com/SFML/SFML/wiki/source:-line-segment-with-thickness

class Road : public sf::Drawable
{
public:
    Road(const sf::Vector2f& point1, const sf::Vector2f& point2, float width = 5.f, sf::Color colour = sf::Color::Black) :
        color(colour), thickness(width)
    {
        position = point1 + point2;
        position /= 2.f;

        const sf::Vector2f direction = point2 - point1;
        const sf::Vector2f unitDirection = direction / std::sqrt(direction.x * direction.x + direction.y * direction.y);
        const sf::Vector2f unitPerpendicular(-unitDirection.y, unitDirection.x);

        const sf::Vector2f offset = (thickness / 2.f) * unitPerpendicular;

        vertices[0].position = point1 + offset;
        vertices[1].position = point2 + offset;
        vertices[2].position = point2 - offset;
        vertices[3].position = point1 - offset;

        for (int i = 0; i < 4; ++i)
            vertices[i].color = color;
    }

    void draw(sf::RenderTarget& target, sf::RenderStates states = sf::RenderStates::Default) const override
    {
        target.draw(vertices, 4, sf::Quads, states);
    }

    std::vector<sf::Vertex> getVertices()
    {
        std::vector<sf::Vertex> out;

        for (int i = 0; i < 4; ++i)
        {
            out.push_back(vertices[i]);
        }

        return out;
    };

    void SetWidth(float width) noexcept
    {
        thickness = width;
    }
    void SetColor(sf::Color colour) noexcept
    {
        color = colour;
    }

    sf::Vector2f position;

protected:
    sf::Vertex vertices[4];
    float thickness;
    sf::Color color;
};