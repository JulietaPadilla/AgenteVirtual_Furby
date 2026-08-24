#pragma once

#include "EstadoFurby.h"
#include "Temporizador.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Clock.hpp>
#include <array>
#include <string>

class Furby {
public:
    explicit Furby(const std::string& nombre);

    void Nacer();
    void GolpearHuevo();
    void Comer();
    void Banar();
    void Enfermar();
    void Curar();
    void HacerPopo();
    void Dormir();
    void Despertar();
    void Morir();
    void Revivir();
    void ActualizarTiempo();
    void ForzarEstado(EstadoFurby nuevoEstado);
    void ActualizarAnimacion();
    void Dibujar(sf::RenderWindow& ventana) const;

    bool CargarSprite(const std::string& ruta);
    bool CargarSpritesPorEstado(const std::string& carpeta);
    EstadoFurby ObtenerEstado() const;
    const std::string& ObtenerNombre() const;
    int ObtenerHambre() const;
    int ObtenerHigiene() const;
    int ObtenerSalud() const;
    int ObtenerSueno() const;
    int ObtenerGolpesHuevo() const;

private:
    void CambiarEstado(EstadoFurby nuevoEstado);
    void PrepararTemporizadores();
    void ActualizarEstadoPorNecesidad();

    std::string nombre;
    int hambre;
    int higiene;
    int salud;
    int sueno;
    int golpesHuevo;
    bool durmiendo;
    int edad;
    EstadoFurby estadoActual;
    std::array<sf::Texture, 7> texturas;
    std::array<bool, 7> tieneSpritePorEstado;
    sf::Sprite sprite;
    bool tieneSprite;
    sf::Clock relojAnimacion;
    int actualFrame;
    static constexpr int totalFrames = 5;
    std::array<int, 7> framesPorEstado;
    sf::Clock relojHambre;
    sf::Clock relojSueno;
    sf::Clock relojHigiene;
    sf::Clock relojSalud;
    THambre tHambre;
    TVida tVida;
    TEnfermedad tEnfermedad;
    TSueno tSueno;
};