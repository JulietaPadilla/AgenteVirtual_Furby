#pragma once

#include "BotonInterfaz.h"
#include "Furby.h"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Audio/Music.hpp>
#include <SFML/System/String.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <vector>

class JuegoAgente {
public:
    JuegoAgente();
    void IniciarJuego();

private:
    void ProcesarEventos();
    void ProcesarTecla(sf::Keyboard::Key tecla);
    void ProcesarClick(sf::Vector2i posicion);
    void Actualizar();
    void Dibujar();
    void CrearInterfaz();
    void MostrarPantallaInicio();
    void DibujarBarra(const std::string& etiqueta, int valor, float x, float y, sf::Color color);
    void DibujarPantallaMuerte();
    void DibujarPantallaInicio();
    void IniciarPartida();

    Furby personaje;
    sf::RenderWindow ventana;
    sf::Font fuente;
    bool fuenteCargada;
    sf::Text textoEstado;
    sf::Text textoControles;
    sf::Text textoNombre;
    sf::Texture texturaPantallaInicio;
    sf::Texture texturaFondoJuego;
    sf::Sprite spritePantallaInicio;
    sf::Sprite spriteFondoJuego;
    sf::Music musica;
    sf::String nombreEscrito;
    bool enPantallaInicio;
    bool escribiendoNombre;
    bool tienePantallaInicio;
    bool tieneFondoJuego;
    std::vector<BotonInterfaz> botones;
};