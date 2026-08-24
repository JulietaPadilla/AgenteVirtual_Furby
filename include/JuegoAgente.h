#pragma once

#include "BotonInterfaz.h"
#include "Furby.h"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
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

    Furby personaje;
    sf::RenderWindow ventana;
    sf::Font fuente;
    bool fuenteCargada;
    sf::Text textoEstado;
    sf::Text textoControles;
    std::vector<BotonInterfaz> botones;
};