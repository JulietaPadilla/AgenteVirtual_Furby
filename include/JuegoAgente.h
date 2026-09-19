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
#include <SFML/System/Clock.hpp>
#include <array>
#include <utility>
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
    void IniciarMinijuegoAStar();
    void CalcularRutaAStar();
    void ActualizarMinijuegoAStar();
    void DibujarMinijuegoAStar();
    bool EsEstadoDeHambre() const;
    void CargarRecursosAStar();

    Furby personaje;
    sf::RenderWindow ventana;
    sf::Font fuente;
    bool fuenteCargada;
    sf::Text textoEstado;
    sf::Text textoControles;
    sf::Text textoNombre;
    sf::Texture texturaPantallaInicio;
    sf::Texture texturaFondoJuego;
    sf::Texture texturaPisoAStar;
    sf::Texture texturaObstaculoAStar;
    sf::Texture texturaComidaAStar;
    sf::Texture texturaRutaAStar;
    sf::Texture texturaFurbyAStar;
    sf::Sprite spritePantallaInicio;
    sf::Sprite spriteFondoJuego;
    sf::Music musica;
    sf::String nombreEscrito;
    bool enPantallaInicio;
    bool escribiendoNombre;
    bool tienePantallaInicio;
    bool tieneFondoJuego;
    bool tienePisoAStar;
    bool tieneObstaculoAStar;
    bool tieneComidaAStar;
    bool tieneRutaAStar;
    bool tieneFurbyAStar;
    bool minijuegoActivo;
    bool esperandoComidaAStar;
    int comidasAStar;
    bool minijuegoCompletado;
    EstadoFurby estadoAnterior;
    std::array<std::array<bool, 12>, 8> obstaculosAStar;
    std::vector<std::pair<int, int>> rutaAStar;
    std::pair<int, int> inicioAStar;
    std::pair<int, int> objetivoAStar;
    std::size_t pasoAStar;
    sf::Clock relojAStar;
    std::vector<BotonInterfaz> botones;
};