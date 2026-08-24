#pragma once

#include <SFML/System/Clock.hpp>

class Temporizador {
public:
    explicit Temporizador(float limite, bool activo = false);

    bool Actualizar();
    void Reiniciar();
    void Pausar();
    void Activar();
    bool EstaActivo() const;

private:
    float tiempoLimite;
    sf::Clock reloj;
    bool activo;
};

class THambre : public Temporizador {
public:
    explicit THambre(float limite) : Temporizador(limite) {}
};

class TVida : public Temporizador {
public:
    explicit TVida(float limite) : Temporizador(limite) {}
};

class TEnfermedad : public Temporizador {
public:
    explicit TEnfermedad(float limite) : Temporizador(limite) {}
};

class TSueno : public Temporizador {
public:
    explicit TSueno(float limite) : Temporizador(limite) {}
};