#pragma once

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Window/Mouse.hpp>
#include <string>

class BotonInterfaz {
public:
    BotonInterfaz(const std::string& etiqueta, float x, float y, const sf::Font* fuente = nullptr);

    bool FueClickeado(sf::Vector2i posicionRaton) const;
    void Dibujar(sf::RenderWindow& ventana) const;
    void EstablecerPosicion(float x, float y);

private:
    sf::RectangleShape forma;
    sf::Text texto;
};