#include "JuegoAgente.h"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <limits>
#include <random>
#include <queue>

namespace {
const std::array<std::string, 10> nombresSprites = {
    "huevo.png", "normal.png", "hambre.png", "sucio.png",
    "enfermo.png", "cansado.png", "hambre_cansado.png",
    "hambre_sucio.png", "hambre_cansado_sucio.png", "muerto.png"
};

int IndiceEstado(EstadoFurby estado) {
    return static_cast<int>(estado);
}

int IndiceAccion(AccionFurby accion) {
    return static_cast<int>(accion) - 1;
}

const std::array<std::string, 5> nombresAcciones = {
    "comer.png", "banar.png", "despertar.png", "dormir.png", "curar.png"
};

int DistanciaManhattan(std::pair<int, int> origen, std::pair<int, int> destino) {
    return std::abs(origen.first - destino.first) + std::abs(origen.second - destino.second);
}
}

const char* NombreEstado(EstadoFurby estado) {
    switch (estado) {
    case EstadoFurby::HUEVO: return "HUEVO";
    case EstadoFurby::NORMAL: return "NORMAL";
    case EstadoFurby::HAMBRE: return "HAMBRE";
    case EstadoFurby::SUCIO: return "SUCIO";
    case EstadoFurby::ENFERMO: return "ENFERMO";
    case EstadoFurby::CANSADO: return "CANSADO";
    case EstadoFurby::HAMBRE_CANSADO: return "HAMBRE + CANSADO";
    case EstadoFurby::HAMBRE_SUCIO: return "HAMBRE + SUCIO";
    case EstadoFurby::HAMBRE_CANSADO_SUCIO: return "HAMBRE + CANSADO + SUCIO";
    case EstadoFurby::MUERTO: return "MUERTO";
    }
    return "DESCONOCIDO";
}

Temporizador::Temporizador(float limite, bool activoInicial)
    : tiempoLimite(limite), activo(activoInicial) {
    reloj.restart();
}

bool Temporizador::Actualizar() {
    return activo && reloj.getElapsedTime().asSeconds() >= tiempoLimite;
}

void Temporizador::Reiniciar() { reloj.restart(); activo = true; }
void Temporizador::Pausar() { activo = false; }
void Temporizador::Activar() { activo = true; }
bool Temporizador::EstaActivo() const { return activo; }

Furby::Furby(const std::string& nombre)
        : nombre(nombre), hambre(100), higiene(100), salud(100), sueno(100), golpesHuevo(0),
                        durmiendo(false), edad(0), estadoActual(EstadoFurby::HUEVO), tieneSprite(false), actualFrame(0),
            accionActual(AccionFurby::NINGUNA), frameAccion(0),
    tHambre(30.f), tVida(60.f), tEnfermedad(20.f) {
        tieneSpritePorEstado.fill(false);
                framesPorEstado.fill(1);
                framesPorEstado[IndiceEstado(EstadoFurby::HUEVO)] = totalFrames;
                tieneAnimacionAccion.fill(false);
                framesPorAccion.fill(1);
    PrepararTemporizadores();
}

void Furby::Nacer() {
    if (estadoActual == EstadoFurby::HUEVO) {
        hambre = 100;
        higiene = 100;
        salud = 100;
        sueno = 100;
        actualFrame = 0;
        CambiarEstado(EstadoFurby::NORMAL);
    }
}
void Furby::GolpearHuevo() {
    if (estadoActual == EstadoFurby::HUEVO) {
        ++golpesHuevo;
        if (golpesHuevo >= totalFrames) Nacer();
    }
}
void Furby::Comer() {
    if (estadoActual == EstadoFurby::HAMBRE || estadoActual == EstadoFurby::HAMBRE_CANSADO || estadoActual == EstadoFurby::HAMBRE_SUCIO || estadoActual == EstadoFurby::HAMBRE_CANSADO_SUCIO) {
        hambre = std::min(100, hambre + 35);
        ReproducirAccion(AccionFurby::COMER);
        if (estadoActual == EstadoFurby::HAMBRE_CANSADO_SUCIO) CambiarEstado(EstadoFurby::CANSADO);
        else if (estadoActual == EstadoFurby::HAMBRE_CANSADO) CambiarEstado(EstadoFurby::CANSADO);
        else if (estadoActual == EstadoFurby::HAMBRE_SUCIO) CambiarEstado(EstadoFurby::SUCIO);
        else CambiarEstado(EstadoFurby::NORMAL);
    }
}
void Furby::ComerPorcion() {
    if (estadoActual == EstadoFurby::HAMBRE || estadoActual == EstadoFurby::HAMBRE_CANSADO || estadoActual == EstadoFurby::HAMBRE_SUCIO) {
        hambre = std::min(100, hambre + 30);
        ReproducirAccion(AccionFurby::COMER);
    }
}
void Furby::Banar() {
    if (estadoActual == EstadoFurby::SUCIO || estadoActual == EstadoFurby::HAMBRE_SUCIO || estadoActual == EstadoFurby::HAMBRE_CANSADO_SUCIO) {
        higiene = std::min(100, higiene + 40);
        ReproducirAccion(AccionFurby::BANAR);
        if (estadoActual == EstadoFurby::HAMBRE_CANSADO_SUCIO) CambiarEstado(EstadoFurby::HAMBRE_CANSADO);
        else if (estadoActual == EstadoFurby::HAMBRE_SUCIO) CambiarEstado(EstadoFurby::HAMBRE);
        else CambiarEstado(EstadoFurby::NORMAL);
    }
}
void Furby::Curar() {
    if (estadoActual == EstadoFurby::ENFERMO) {
        salud = std::min(100, salud + 40);
        ReproducirAccion(AccionFurby::CURAR);
        CambiarEstado(EstadoFurby::NORMAL);
    }
}
void Furby::HacerPopo() {
    if (estadoActual == EstadoFurby::NORMAL) CambiarEstado(EstadoFurby::SUCIO);
    else if (estadoActual == EstadoFurby::HAMBRE) CambiarEstado(EstadoFurby::HAMBRE_SUCIO);
}
void Furby::Dormir() {
    if ((estadoActual == EstadoFurby::NORMAL || estadoActual == EstadoFurby::HAMBRE_CANSADO || estadoActual == EstadoFurby::HAMBRE_CANSADO_SUCIO || estadoActual == EstadoFurby::CANSADO) && sueno <= 50) {
        durmiendo = true;
        ReproducirAccion(AccionFurby::DORMIR);
        if (estadoActual == EstadoFurby::NORMAL || estadoActual == EstadoFurby::HAMBRE_CANSADO || estadoActual == EstadoFurby::HAMBRE_CANSADO_SUCIO) {
            CambiarEstado(EstadoFurby::CANSADO);
        }
    }
}
void Furby::Despertar() {
    if (durmiendo && (estadoActual == EstadoFurby::CANSADO || estadoActual == EstadoFurby::HAMBRE_CANSADO || estadoActual == EstadoFurby::HAMBRE_CANSADO_SUCIO)) {
        durmiendo = false;
        ReproducirAccion(AccionFurby::DESPERTAR);
        if (estadoActual == EstadoFurby::HAMBRE_CANSADO || estadoActual == EstadoFurby::HAMBRE_CANSADO_SUCIO) CambiarEstado(EstadoFurby::HAMBRE);
        else CambiarEstado(EstadoFurby::NORMAL);
    }
}
void Furby::Morir() {
    durmiendo = false;
    CambiarEstado(EstadoFurby::MUERTO);
}
void Furby::Revivir() {
    if (estadoActual == EstadoFurby::MUERTO) {
        hambre = 70; higiene = 70; salud = 70; sueno = 100;
        durmiendo = false;
        CambiarEstado(EstadoFurby::NORMAL);
    }
}

void Furby::ActualizarTiempo() {
    if (estadoActual == EstadoFurby::HUEVO || estadoActual == EstadoFurby::MUERTO) return;
    if (durmiendo) {
        if (relojHambre.getElapsedTime().asSeconds() >= 16.f) { hambre = std::max(0, hambre - 3); relojHambre.restart(); }
        if (relojHigiene.getElapsedTime().asSeconds() >= 24.f) { higiene = std::max(0, higiene - 2); relojHigiene.restart(); }
        if (relojSueno.getElapsedTime().asSeconds() >= 5.f) { sueno = std::min(100, sueno + 8); relojSueno.restart(); }
        if (relojSalud.getElapsedTime().asSeconds() >= 6.f) {
            int deficit = (100 - hambre) + (100 - higiene) + (100 - sueno);
            int necesidadesCriticas = (hambre <= 50 ? 1 : 0) + (higiene <= 50 ? 1 : 0) + (sueno <= 50 ? 1 : 0);
            salud = std::max(0, salud - (necesidadesCriticas >= 2 && deficit >= 180 ? 8 : necesidadesCriticas >= 2 && deficit >= 90 ? 4 : 0));
            relojSalud.restart();
        }
    } else {
        if (relojHambre.getElapsedTime().asSeconds() >= 8.f) { hambre = std::max(0, hambre - 5); relojHambre.restart(); }
        if (relojHigiene.getElapsedTime().asSeconds() >= 10.f) { higiene = std::max(0, higiene - 5); relojHigiene.restart(); }
        if (relojSueno.getElapsedTime().asSeconds() >= 10.f) { sueno = std::max(0, sueno - 5); relojSueno.restart(); }
        if (relojSalud.getElapsedTime().asSeconds() >= 6.f) {
            int deficit = (100 - hambre) + (100 - higiene) + (100 - sueno);
            int necesidadesCriticas = (hambre <= 50 ? 1 : 0) + (higiene <= 50 ? 1 : 0) + (sueno <= 50 ? 1 : 0);
            int deterioro = necesidadesCriticas >= 2 && deficit >= 180 ? 8 : necesidadesCriticas >= 2 && deficit >= 90 ? 4 : 0;
            salud = std::max(0, salud - deterioro);
            relojSalud.restart();
        }
    }
    if (salud <= 0) { Morir(); return; }
    ActualizarTransicionesPorTiempo();
}

void Furby::ActualizarTransicionesPorTiempo() {
    const bool hambreBaja = hambre <= 50;
    const bool higieneBaja = higiene <= 50;
    const bool cansancio = sueno <= 50;

    if (salud <= 30 && estadoActual != EstadoFurby::ENFERMO) {
        CambiarEstado(EstadoFurby::ENFERMO);
        return;
    }

    switch (estadoActual) {
    case EstadoFurby::NORMAL:
        if (hambreBaja && higieneBaja && cansancio) CambiarEstado(EstadoFurby::HAMBRE_CANSADO_SUCIO);
        else if (hambreBaja && cansancio) CambiarEstado(EstadoFurby::HAMBRE_CANSADO);
        else if (hambreBaja && higieneBaja) CambiarEstado(EstadoFurby::HAMBRE_SUCIO);
        else if (hambreBaja) CambiarEstado(EstadoFurby::HAMBRE);
        else if (higieneBaja) CambiarEstado(EstadoFurby::SUCIO);
        else if (cansancio) CambiarEstado(EstadoFurby::CANSADO);
        break;
    case EstadoFurby::HAMBRE:
        if (higieneBaja && cansancio) CambiarEstado(EstadoFurby::HAMBRE_CANSADO_SUCIO);
        else if (!hambreBaja && cansancio) CambiarEstado(EstadoFurby::CANSADO);
        else if (!hambreBaja && higieneBaja) CambiarEstado(EstadoFurby::SUCIO);
        else if (!hambreBaja) CambiarEstado(EstadoFurby::NORMAL);
        else if (cansancio) CambiarEstado(EstadoFurby::HAMBRE_CANSADO);
        else if (higieneBaja) CambiarEstado(EstadoFurby::HAMBRE_SUCIO);
        break;
    case EstadoFurby::SUCIO:
        if (hambreBaja && cansancio) CambiarEstado(EstadoFurby::HAMBRE_CANSADO_SUCIO);
        else if (hambreBaja) CambiarEstado(EstadoFurby::HAMBRE_SUCIO);
        break;
    case EstadoFurby::CANSADO:
        if (hambreBaja && higieneBaja) CambiarEstado(EstadoFurby::HAMBRE_CANSADO_SUCIO);
        else if (hambreBaja) CambiarEstado(EstadoFurby::HAMBRE_CANSADO);
        break;
    case EstadoFurby::HAMBRE_CANSADO:
        if (higieneBaja) CambiarEstado(EstadoFurby::HAMBRE_CANSADO_SUCIO);
        else if (!hambreBaja) CambiarEstado(EstadoFurby::CANSADO);
        break;
    case EstadoFurby::HAMBRE_CANSADO_SUCIO:
        if (!hambreBaja && !higieneBaja && !cansancio) CambiarEstado(EstadoFurby::NORMAL);
        else if (!hambreBaja && !higieneBaja) CambiarEstado(EstadoFurby::CANSADO);
        else if (!hambreBaja && !cansancio) CambiarEstado(EstadoFurby::SUCIO);
        else if (!higieneBaja && !cansancio) CambiarEstado(EstadoFurby::HAMBRE);
        break;
    case EstadoFurby::HAMBRE_SUCIO:
        if (!hambreBaja) CambiarEstado(EstadoFurby::SUCIO);
        break;
    case EstadoFurby::ENFERMO:
    case EstadoFurby::HUEVO:
    case EstadoFurby::MUERTO:
        break;
    }
}
void Furby::ActualizarAnimacion() {
    if (accionActual != AccionFurby::NINGUNA) {
        int cantidadFrames = framesPorAccion[IndiceAccion(accionActual)];
        if (relojAccion.getElapsedTime().asSeconds() >= 0.35f) {
            relojAccion.restart();
            ++frameAccion;
            if (accionActual == AccionFurby::DORMIR) frameAccion %= cantidadFrames;
            else if (frameAccion >= cantidadFrames) accionActual = AccionFurby::NINGUNA;
        }
    }
    if (estadoActual == EstadoFurby::HUEVO) return;
    if (relojAnimacion.getElapsedTime().asSeconds() < 0.18f) return;
    relojAnimacion.restart();
    int cantidadFrames = framesPorEstado[IndiceEstado(estadoActual)];
    actualFrame = (actualFrame + 1) % cantidadFrames;
}

void Furby::Dibujar(sf::RenderWindow& ventana) const {
    if (accionActual != AccionFurby::NINGUNA && tieneAnimacionAccion[IndiceAccion(accionActual)]) {
        const sf::Texture& texturaAccion = animacionesAcciones[IndiceAccion(accionActual)];
        sf::Vector2u tamano = texturaAccion.getSize();
        int cantidadFrames = tamano.x >= tamano.y * 2 ? 5 : 1;
        int anchoFrame = static_cast<int>(tamano.x) / cantidadFrames;
        sf::Sprite spriteAccion(texturaAccion);
        spriteAccion.setTextureRect(sf::IntRect(frameAccion * anchoFrame, 0, anchoFrame, static_cast<int>(tamano.y)));
        float escala = std::min(200.f / static_cast<float>(anchoFrame), 200.f / static_cast<float>(tamano.y));
        spriteAccion.setScale(escala, escala);
        sf::FloatRect limites = spriteAccion.getGlobalBounds();
        spriteAccion.setPosition(400.f - limites.width / 2.f, 240.f - limites.height / 2.f);
        ventana.draw(spriteAccion);
        return;
    }
    int indice = IndiceEstado(estadoActual);
    if (tieneSpritePorEstado[indice]) {
        sf::Sprite spriteEstado(texturas[indice]);
        sf::Vector2u tamano = texturas[indice].getSize();
        int cantidadFrames = framesPorEstado[indice];
        int anchoFrame = static_cast<int>(tamano.x) / cantidadFrames;
        int frameParaMostrar = actualFrame;
        if (estadoActual == EstadoFurby::HUEVO) frameParaMostrar = std::min(cantidadFrames - 1, golpesHuevo);
        spriteEstado.setTextureRect(sf::IntRect(frameParaMostrar * anchoFrame, 0, anchoFrame, static_cast<int>(tamano.y)));
        sf::Vector2u tamanoFrame(static_cast<unsigned int>(anchoFrame), tamano.y);
        float escala = std::min(200.f / static_cast<float>(tamanoFrame.x), 200.f / static_cast<float>(tamanoFrame.y));
        spriteEstado.setScale(escala, escala);
        sf::FloatRect limites = spriteEstado.getGlobalBounds();
        spriteEstado.setPosition(400.f - limites.width / 2.f, 240.f - limites.height / 2.f);
        ventana.draw(spriteEstado);
        return;
    }
    sf::CircleShape cuerpo(100.f);
    float movimiento = std::sin(relojAnimacion.getElapsedTime().asSeconds() * 3.f) * 5.f;
    cuerpo.setPosition(300.f, 150.f + movimiento);
    cuerpo.setFillColor(estadoActual == EstadoFurby::MUERTO ? sf::Color(80, 80, 80) : sf::Color(244, 186, 65));
    cuerpo.setOutlineThickness(5.f);
    cuerpo.setOutlineColor(sf::Color(60, 45, 35));
    ventana.draw(cuerpo);
}

bool Furby::CargarSprite(const std::string& ruta) {
    tieneSprite = texturas[IndiceEstado(EstadoFurby::NORMAL)].loadFromFile(ruta);
    tieneSpritePorEstado[IndiceEstado(EstadoFurby::NORMAL)] = tieneSprite;
    return tieneSprite;
}

bool Furby::CargarAnimacionesAcciones(const std::string& carpeta) {
    bool cargoAlguna = false;
    for (int indice = 0; indice < static_cast<int>(nombresAcciones.size()); ++indice) {
        tieneAnimacionAccion[indice] = animacionesAcciones[indice].loadFromFile(carpeta + nombresAcciones[indice]);
        if (tieneAnimacionAccion[indice]) {
            sf::Vector2u tamano = animacionesAcciones[indice].getSize();
            framesPorAccion[indice] = tamano.x >= tamano.y * 2 ? 5 : 1;
            cargoAlguna = true;
        }
    }
    return cargoAlguna;
}

void Furby::ReproducirAccion(AccionFurby accion) {
    int indice = IndiceAccion(accion);
    if (indice < 0 || indice >= static_cast<int>(tieneAnimacionAccion.size()) || !tieneAnimacionAccion[indice]) return;
    accionActual = accion;
    frameAccion = 0;
    relojAccion.restart();
}

bool Furby::CargarSpritesPorEstado(const std::string& carpeta) {
    bool cargoAlgunSprite = false;
    for (int indice = 0; indice < static_cast<int>(nombresSprites.size()); ++indice) {
        tieneSpritePorEstado[indice] = texturas[indice].loadFromFile(carpeta + nombresSprites[indice]);
        cargoAlgunSprite = cargoAlgunSprite || tieneSpritePorEstado[indice];
    }
    tieneSprite = tieneSpritePorEstado[IndiceEstado(EstadoFurby::NORMAL)];
    return cargoAlgunSprite;
}

EstadoFurby Furby::ObtenerEstado() const { return estadoActual; }
const std::string& Furby::ObtenerNombre() const { return nombre; }
int Furby::ObtenerHambre() const { return hambre; }
int Furby::ObtenerHigiene() const { return higiene; }
int Furby::ObtenerSalud() const { return salud; }
int Furby::ObtenerSueno() const { return sueno; }
int Furby::ObtenerGolpesHuevo() const { return golpesHuevo; }
void Furby::EstablecerNombre(const std::string& nuevoNombre) {
    if (!nuevoNombre.empty()) nombre = nuevoNombre;
}

void Furby::CambiarEstado(EstadoFurby nuevoEstado) {
    estadoActual = nuevoEstado;
    actualFrame = 0;
    tHambre.Pausar();
    tVida.Pausar();
    tEnfermedad.Pausar();
    if (nuevoEstado == EstadoFurby::HAMBRE || nuevoEstado == EstadoFurby::SUCIO) tEnfermedad.Reiniciar();
    if (nuevoEstado == EstadoFurby::HAMBRE || nuevoEstado == EstadoFurby::ENFERMO) tVida.Reiniciar();
    if (nuevoEstado == EstadoFurby::NORMAL) tHambre.Reiniciar();
}

void Furby::PrepararTemporizadores() {
    tHambre.Pausar(); tVida.Pausar(); tEnfermedad.Pausar();
}

BotonInterfaz::BotonInterfaz(const std::string& etiqueta, float x, float y, const sf::Font* fuente, const std::string& rutaIcono)
    : forma(sf::Vector2f(140.f, 42.f)), tieneIcono(false), esImagenCompleta(false) {
    forma.setPosition(x, y);
    forma.setFillColor(sf::Color(224, 126, 55));
    texto.setString(etiqueta);
    texto.setCharacterSize(16);
    texto.setFillColor(sf::Color::White);
    if (fuente != nullptr) texto.setFont(*fuente);
    bool cargoIcono = !rutaIcono.empty() && texturaIcono.loadFromFile(rutaIcono);
    if (!cargoIcono && !rutaIcono.empty()) cargoIcono = texturaIcono.loadFromFile("../" + rutaIcono);
    if (cargoIcono) {
        tieneIcono = true;
        esImagenCompleta = true;
        icono.setTexture(texturaIcono);
        icono.setColor(sf::Color::White);
        sf::Vector2u tamano = texturaIcono.getSize();
        float escala = std::min(155.f / static_cast<float>(tamano.x), 62.f / static_cast<float>(tamano.y));
        icono.setScale(escala, escala);
        icono.setPosition(x + (155.f - tamano.x * escala) / 2.f, y + (62.f - tamano.y * escala) / 2.f);
        forma.setSize(sf::Vector2f(155.f, 62.f));
        forma.setPosition(x, y);
        forma.setFillColor(sf::Color::Transparent);
    }
    if (!esImagenCompleta) texto.setPosition(x + 10.f, y + 11.f);
}

bool BotonInterfaz::FueClickeado(sf::Vector2i posicionRaton) const {
    return forma.getGlobalBounds().contains(static_cast<float>(posicionRaton.x), static_cast<float>(posicionRaton.y));
}

void BotonInterfaz::Dibujar(sf::RenderWindow& ventana) const {
    if (esImagenCompleta) ventana.draw(icono);
    else { ventana.draw(forma); if (tieneIcono) ventana.draw(icono); ventana.draw(texto); }
}
void BotonInterfaz::EstablecerPosicion(float x, float y) {
    forma.setPosition(x, y);
    if (esImagenCompleta) {
        sf::FloatRect limites = icono.getLocalBounds();
        icono.setPosition(x + (155.f - limites.width * icono.getScale().x) / 2.f, y + (62.f - limites.height * icono.getScale().y) / 2.f);
    }
    else {
        texto.setPosition(x + 10.f, y + 11.f);
        if (tieneIcono) icono.setPosition(x + 8.f, y + 9.f);
    }
}

JuegoAgente::JuegoAgente()
    : personaje("Furby"), ventana(sf::VideoMode(800, 600), "Agente Virtual Furby"), fuenteCargada(false), nombreEscrito(), enPantallaInicio(true), escribiendoNombre(true), tienePantallaInicio(false), tieneFondoJuego(false), tienePisoAStar(false), tieneObstaculoAStar(false), tieneComidaAStar(false), tieneRutaAStar(false), tieneFurbyAStar(false), minijuegoActivo(false), esperandoComidaAStar(false), comidasAStar(0), minijuegoCompletado(false), estadoAnterior(EstadoFurby::HUEVO), pasoAStar(0) {
    ventana.setFramerateLimit(60);
    obstaculosAStar = {};
    const std::array<std::string, 2> rutasFuente = {
        "assets/fonts/F25_Bank_Printer.otf", "../assets/fonts/F25_Bank_Printer.otf"
    };
    for (const auto& ruta : rutasFuente) {
        if (fuente.loadFromFile(ruta)) { fuenteCargada = true; break; }
    }
    if (!personaje.CargarSpritesPorEstado("assets/Images/")) {
        personaje.CargarSpritesPorEstado("../assets/Images/");
    }
    if (!personaje.CargarAnimacionesAcciones("assets/Images/acciones/")) {
        personaje.CargarAnimacionesAcciones("../assets/Images/acciones/");
    }
    tienePantallaInicio = texturaPantallaInicio.loadFromFile("assets/Images/pantalla_inicio.png");
    tieneFondoJuego = texturaFondoJuego.loadFromFile("assets/Images/fondo_juego.png");
    if (!tienePantallaInicio) tienePantallaInicio = texturaPantallaInicio.loadFromFile("../assets/Images/pantalla_inicio.png");
    if (!tieneFondoJuego) tieneFondoJuego = texturaFondoJuego.loadFromFile("../assets/Images/fondo_juego.png");
    if (tienePantallaInicio) spritePantallaInicio.setTexture(texturaPantallaInicio);
    if (tieneFondoJuego) spriteFondoJuego.setTexture(texturaFondoJuego);
    CargarRecursosAStar();
    ventana.requestFocus();
    if (!musica.openFromFile("assets/Music/musica.mp3")) {
        musica.openFromFile("../assets/Music/musica.mp3");
    }
    musica.setLoop(true);
    musica.setVolume(35.f);
    musica.play();
    CrearInterfaz();
}

void JuegoAgente::CargarRecursosAStar() {
    auto cargar = [](sf::Texture& textura, const std::string& nombre) {
        return textura.loadFromFile("assets/Images/astar/" + nombre) || textura.loadFromFile("../assets/Images/astar/" + nombre);
    };
    tienePisoAStar = cargar(texturaPisoAStar, "piso.png");
    tieneObstaculoAStar = cargar(texturaObstaculoAStar, "obstaculo.png");
    tieneComidaAStar = cargar(texturaComidaAStar, "comida.png");
    tieneRutaAStar = cargar(texturaRutaAStar, "ruta.png");
    tieneFurbyAStar = texturaFurbyAStar.loadFromFile("assets/Images/normal.png");
    if (!tieneFurbyAStar) tieneFurbyAStar = texturaFurbyAStar.loadFromFile("../assets/Images/normal.png");
}

void JuegoAgente::IniciarJuego() {
    MostrarPantallaInicio();
    while (ventana.isOpen()) { ProcesarEventos(); Actualizar(); Dibujar(); }
}

void JuegoAgente::ProcesarEventos() {
    sf::Event evento{};
    while (ventana.pollEvent(evento)) {
        if (evento.type == sf::Event::Closed) ventana.close();
        if (enPantallaInicio && evento.type == sf::Event::TextEntered) {
                if (evento.text.unicode >= 32 && evento.text.unicode != 127 && nombreEscrito.getSize() < 14) {
                    nombreEscrito += evento.text.unicode;
                    textoNombre.setString(nombreEscrito);
            }
        }
        if (evento.type == sf::Event::KeyPressed) ProcesarTecla(evento.key.code);
        if (evento.type == sf::Event::MouseButtonPressed && evento.mouseButton.button == sf::Mouse::Left) {
            ProcesarClick(sf::Vector2i(evento.mouseButton.x, evento.mouseButton.y));
        }
    }
}

void JuegoAgente::ProcesarTecla(sf::Keyboard::Key tecla) {
    if (enPantallaInicio) {
        if (tecla == sf::Keyboard::BackSpace && !nombreEscrito.isEmpty()) {
                nombreEscrito.erase(nombreEscrito.getSize() - 1, 1);
                textoNombre.setString(nombreEscrito.isEmpty() ? "Escribe un nombre" : nombreEscrito);
        }
        else if (tecla == sf::Keyboard::Enter) IniciarPartida();
        return;
    }
    if (minijuegoActivo) return;
    if (tecla == sf::Keyboard::A && EsEstadoDeHambre() && comidasAStar < 3) {
        IniciarMinijuegoAStar();
        return;
    }
    switch (tecla) {
    case sf::Keyboard::C: personaje.Comer(); break;
    case sf::Keyboard::B: personaje.Banar(); break;
    case sf::Keyboard::P: personaje.HacerPopo(); break;
    case sf::Keyboard::R: personaje.Curar(); break;
    case sf::Keyboard::D: personaje.Dormir(); break;
    case sf::Keyboard::W: personaje.Despertar(); break;
    case sf::Keyboard::K: personaje.Morir(); break;
    case sf::Keyboard::V: personaje.Revivir(); break;
    default: break;
    }
}

void JuegoAgente::ProcesarClick(sf::Vector2i posicion) {
    if (enPantallaInicio) {
        sf::FloatRect campoNombre(205.f, 305.f, 390.f, 52.f);
        sf::FloatRect botonInicio(275.f, 390.f, 250.f, 55.f);
        if (campoNombre.contains(static_cast<float>(posicion.x), static_cast<float>(posicion.y))) {
            escribiendoNombre = true;
            ventana.requestFocus();
            return;
        }
        if (botonInicio.contains(static_cast<float>(posicion.x), static_cast<float>(posicion.y))) IniciarPartida();
        return;
    }
    if (minijuegoActivo) {
        if (esperandoComidaAStar) {
            const int columna = (posicion.x - 112) / 48;
            const int fila = (posicion.y - 78) / 48;
            if (std::make_pair(columna, fila) == objetivoAStar) {
                CalcularRutaAStar();
            }
        }
        return;
    }
    if (EsEstadoDeHambre() && comidasAStar < 3 && sf::FloatRect(310.f, 500.f, 230.f, 45.f).contains(static_cast<float>(posicion.x), static_cast<float>(posicion.y))) {
        IniciarMinijuegoAStar();
        return;
    }
    if (personaje.ObtenerEstado() == EstadoFurby::HUEVO) {
        sf::FloatRect zonaHuevo(300.f, 150.f, 200.f, 200.f);
        if (zonaHuevo.contains(static_cast<float>(posicion.x), static_cast<float>(posicion.y))) {
            personaje.GolpearHuevo();
        }
        return;
    }

    for (std::size_t indice = 0; indice < botones.size(); ++indice) {
        if (!botones[indice].FueClickeado(posicion)) continue;
        if (personaje.ObtenerEstado() == EstadoFurby::MUERTO && indice == 0) personaje.Revivir();
        else if (personaje.ObtenerEstado() != EstadoFurby::MUERTO && indice >= 1) {
            switch (indice - 1) {
            case 0: personaje.Comer(); break;
            case 1: personaje.Banar(); break;
            case 2: personaje.Dormir(); break;
            case 3: personaje.Despertar(); break;
            case 4: personaje.Curar(); break;
            default: break;
            }
        }
        break;
    }
}

void JuegoAgente::Actualizar() {
    if (enPantallaInicio) return;
    if (minijuegoActivo) {
        ActualizarMinijuegoAStar();
        personaje.ActualizarAnimacion();
        return;
    }
    personaje.ActualizarTiempo();
    personaje.ActualizarAnimacion();
    if (!EsEstadoDeHambre()) {
        minijuegoCompletado = false;
        comidasAStar = 0;
    }
    estadoAnterior = personaje.ObtenerEstado();
    if (fuenteCargada) {
        textoEstado.setString("Furby: " + personaje.ObtenerNombre() + " | Estado: " + NombreEstado(personaje.ObtenerEstado()));
        if (personaje.ObtenerEstado() == EstadoFurby::HUEVO) textoControles.setString("Haz clic 5 veces sobre el huevo");
        else textoControles.setString("ACCIONES");
    }
}

void JuegoAgente::Dibujar() {
    if (enPantallaInicio) {
        DibujarPantallaInicio();
        return;
    }
    ventana.clear(sf::Color(247, 232, 196));
    if (tieneFondoJuego) {
        sf::Vector2u tamano = texturaFondoJuego.getSize();
        spriteFondoJuego.setScale(800.f / static_cast<float>(tamano.x), 600.f / static_cast<float>(tamano.y));
        ventana.draw(spriteFondoJuego);
    }
    sf::RectangleShape encabezado(sf::Vector2f(520.f, 54.f));
    encabezado.setPosition(140.f, 12.f);
    encabezado.setFillColor(sf::Color(35, 28, 48, 225));
    encabezado.setOutlineThickness(2.f);
    encabezado.setOutlineColor(sf::Color(224, 126, 55));
    if (personaje.ObtenerEstado() != EstadoFurby::HUEVO) ventana.draw(encabezado);
    personaje.Dibujar(ventana);
    bool estaEnHuevo = personaje.ObtenerEstado() == EstadoFurby::HUEVO;
    bool estaMuerto = personaje.ObtenerEstado() == EstadoFurby::MUERTO;
    if (!estaEnHuevo) {
        DibujarBarra("Hambre", personaje.ObtenerHambre(), 45.f, 100.f, sf::Color(224, 126, 55));
        DibujarBarra("Higiene", personaje.ObtenerHigiene(), 45.f, 160.f, sf::Color(55, 150, 190));
        DibujarBarra("Salud", personaje.ObtenerSalud(), 45.f, 220.f, sf::Color(80, 170, 90));
        DibujarBarra("Sueno", personaje.ObtenerSueno(), 45.f, 280.f, sf::Color(110, 90, 170));
    }
    if (EsEstadoDeHambre() && comidasAStar < 3 && fuenteCargada) {
        sf::RectangleShape botonComida(sf::Vector2f(230.f, 42.f));
        botonComida.setPosition(285.f, 505.f);
        botonComida.setFillColor(sf::Color(90, 190, 105));
        ventana.draw(botonComida);
        sf::Text textoComida("BUSCAR COMIDA (A)", fuente, 16);
        textoComida.setPosition(300.f, 516.f);
        textoComida.setFillColor(sf::Color::White);
        ventana.draw(textoComida);
    }
    if (fuenteCargada && !estaEnHuevo) ventana.draw(textoEstado);
    if (estaEnHuevo && fuenteCargada) {
        sf::RectangleShape panelHuevo(sf::Vector2f(540.f, 112.f));
        panelHuevo.setPosition(130.f, 345.f);
        panelHuevo.setFillColor(sf::Color(35, 28, 48, 220));
        panelHuevo.setOutlineThickness(2.f);
        panelHuevo.setOutlineColor(sf::Color(224, 126, 55));
        ventana.draw(panelHuevo);
        sf::Text instruccion("Golpea el huevo para abrirlo", fuente, 22);
        instruccion.setPosition(225.f, 370.f);
        instruccion.setFillColor(sf::Color::White);
        ventana.draw(instruccion);
        sf::Text golpes("Golpes: " + std::to_string(personaje.ObtenerGolpesHuevo()) + "/5", fuente, 20);
        golpes.setPosition(330.f, 415.f);
        golpes.setFillColor(sf::Color(255, 214, 120));
        ventana.draw(golpes);
    }
    if (estaMuerto) DibujarPantallaMuerte();
    if (minijuegoActivo) DibujarMinijuegoAStar();
    for (std::size_t indice = 0; indice < botones.size(); ++indice) {
        if (!minijuegoActivo && !estaEnHuevo && ((estaMuerto && indice == 0) || (!estaMuerto && indice >= 1))) botones[indice].Dibujar(ventana);
    }
    ventana.display();
}

bool JuegoAgente::EsEstadoDeHambre() const {
    EstadoFurby estado = personaje.ObtenerEstado();
    return estado == EstadoFurby::HAMBRE || estado == EstadoFurby::HAMBRE_CANSADO || estado == EstadoFurby::HAMBRE_SUCIO || estado == EstadoFurby::HAMBRE_CANSADO_SUCIO;
}

void JuegoAgente::IniciarMinijuegoAStar() {
    obstaculosAStar = {};
        const std::array<std::pair<int, int>, 18> bloques = comidasAStar % 3 == 0 ? std::array<std::pair<int, int>, 18>{{
            {2, 1}, {3, 1}, {4, 1}, {5, 1}, {7, 2}, {8, 2}, {9, 2}, {10, 2},
            {1, 3}, {2, 3}, {3, 3}, {6, 3}, {7, 3}, {8, 3}, {4, 4}, {5, 4}, {6, 4}, {7, 4}
        }} : comidasAStar % 3 == 1 ? std::array<std::pair<int, int>, 18>{{
            {3, 0}, {3, 1}, {3, 2}, {3, 3}, {3, 5}, {3, 6}, {6, 1}, {6, 2},
            {6, 3}, {6, 4}, {6, 5}, {9, 2}, {9, 3}, {9, 4}, {2, 6}, {4, 6}, {7, 6}, {10, 6}
        }} : std::array<std::pair<int, int>, 18>{{
            {1, 1}, {2, 1}, {5, 1}, {6, 1}, {9, 1}, {10, 1}, {4, 2}, {7, 2},
            {2, 3}, {3, 3}, {4, 3}, {8, 4}, {9, 4}, {10, 4}, {1, 5}, {5, 5}, {6, 5}, {7, 5}
        }};
    for (const auto& bloque : bloques) obstaculosAStar[bloque.second][bloque.first] = true;
    inicioAStar = {1, 6};
    objetivoAStar = comidasAStar % 3 == 0 ? std::make_pair(10, 1) : comidasAStar % 3 == 1 ? std::make_pair(11, 7) : std::make_pair(1, 0);
    rutaAStar.clear();
    pasoAStar = 0;
    esperandoComidaAStar = true;
    minijuegoActivo = true;
    minijuegoCompletado = false;
    relojAStar.restart();
}

void JuegoAgente::CalcularRutaAStar() {
    const std::pair<int, int> inicio = inicioAStar;
    const std::pair<int, int> objetivo = objetivoAStar;
    const int infinito = std::numeric_limits<int>::max();
    std::array<std::array<int, 12>, 8> costos;
    std::array<std::array<std::pair<int, int>, 12>, 8> padres;
    for (auto& fila : costos) fila.fill(infinito);
    for (auto& fila : padres) fila.fill({-1, -1});

    using Nodo = std::pair<int, std::pair<int, int>>;
    std::priority_queue<Nodo, std::vector<Nodo>, std::greater<Nodo>> pendientes;
    costos[inicio.second][inicio.first] = 0;
    pendientes.push({DistanciaManhattan(inicio, objetivo), inicio});
    const std::array<std::pair<int, int>, 4> direcciones = {{{1, 0}, {-1, 0}, {0, 1}, {0, -1}}};

    while (!pendientes.empty()) {
        auto actual = pendientes.top().second;
        pendientes.pop();
        if (actual == objetivo) break;
        for (const auto& direccion : direcciones) {
            int siguienteX = actual.first + direccion.first;
            int siguienteY = actual.second + direccion.second;
            if (siguienteX < 0 || siguienteX >= 12 || siguienteY < 0 || siguienteY >= 8 || obstaculosAStar[siguienteY][siguienteX]) continue;
            int nuevoCosto = costos[actual.second][actual.first] + 1;
            if (nuevoCosto < costos[siguienteY][siguienteX]) {
                costos[siguienteY][siguienteX] = nuevoCosto;
                padres[siguienteY][siguienteX] = actual;
                int prioridad = nuevoCosto + DistanciaManhattan({siguienteX, siguienteY}, objetivo);
                pendientes.push({prioridad, {siguienteX, siguienteY}});
            }
        }
    }

    rutaAStar.clear();
    if (costos[objetivo.second][objetivo.first] != infinito) {
        for (auto actual = objetivo; actual != std::pair<int, int>(-1, -1); actual = padres[actual.second][actual.first]) {
            rutaAStar.push_back(actual);
        }
        std::reverse(rutaAStar.begin(), rutaAStar.end());
    }
    pasoAStar = rutaAStar.size() > 1 ? 1 : 0;
    esperandoComidaAStar = false;
    relojAStar.restart();
}

void JuegoAgente::ActualizarMinijuegoAStar() {
    if (esperandoComidaAStar) return;
    if (rutaAStar.empty()) {
        minijuegoActivo = false;
        return;
    }
    if (relojAStar.getElapsedTime().asSeconds() < 0.28f) return;
    relojAStar.restart();
    if (pasoAStar + 1 < rutaAStar.size()) {
        ++pasoAStar;
        return;
    }
    personaje.ComerPorcion();
    ++comidasAStar;
    estadoAnterior = personaje.ObtenerEstado();
    if (comidasAStar < 3) {
        IniciarMinijuegoAStar();
        return;
    }
    minijuegoActivo = false;
    minijuegoCompletado = true;
}

void JuegoAgente::DibujarMinijuegoAStar() {
    const float tamanoCelda = 48.f;
    const float origenX = 112.f;
    const float origenY = 78.f;
    sf::RectangleShape panel(sf::Vector2f(600.f, 420.f));
    panel.setPosition(100.f, 55.f);
    panel.setFillColor(sf::Color(20, 18, 30, 240));
    panel.setOutlineThickness(3.f);
    panel.setOutlineColor(sf::Color(255, 214, 120));
    ventana.draw(panel);

    if (fuenteCargada) {
        sf::Text titulo("BUSCA LA COMIDA", fuente, 24);
        titulo.setPosition(285.f, 62.f);
        titulo.setFillColor(sf::Color::White);
        ventana.draw(titulo);
        sf::Text subtitulo("Ruta optima calculada con A*", fuente, 14);
        subtitulo.setPosition(285.f, 91.f);
        subtitulo.setFillColor(sf::Color(130, 195, 177));
        ventana.draw(subtitulo);
    }

    for (int fila = 0; fila < 8; ++fila) {
        for (int columna = 0; columna < 12; ++columna) {
            sf::RectangleShape celda(sf::Vector2f(tamanoCelda - 2.f, tamanoCelda - 2.f));
            celda.setPosition(origenX + columna * tamanoCelda, origenY + fila * tamanoCelda);
            celda.setFillColor(obstaculosAStar[fila][columna] ? sf::Color(75, 54, 75) : sf::Color(55, 78, 86));
            celda.setOutlineThickness(1.f);
            celda.setOutlineColor(sf::Color(130, 195, 177, 120));
            ventana.draw(celda);
            const sf::Texture* texturaCelda = obstaculosAStar[fila][columna]
                ? (tieneObstaculoAStar ? &texturaObstaculoAStar : nullptr)
                : (tienePisoAStar ? &texturaPisoAStar : nullptr);
            if (texturaCelda != nullptr) {
                sf::Sprite spriteCelda(*texturaCelda);
                sf::Vector2u tamano = texturaCelda->getSize();
                spriteCelda.setScale((tamanoCelda - 2.f) / static_cast<float>(tamano.x), (tamanoCelda - 2.f) / static_cast<float>(tamano.y));
                spriteCelda.setPosition(celda.getPosition());
                ventana.draw(spriteCelda);
            }
        }
    }

    if (!esperandoComidaAStar) for (std::size_t indice = pasoAStar; indice < rutaAStar.size(); ++indice) {
        if (tieneRutaAStar) {
            sf::Sprite marca(texturaRutaAStar);
            sf::Vector2u tamano = texturaRutaAStar.getSize();
            marca.setScale((tamanoCelda - 2.f) / static_cast<float>(tamano.x), (tamanoCelda - 2.f) / static_cast<float>(tamano.y));
            marca.setPosition(origenX + rutaAStar[indice].first * tamanoCelda, origenY + rutaAStar[indice].second * tamanoCelda);
            ventana.draw(marca);
        } else {
            sf::CircleShape marca(5.f);
            marca.setPosition(origenX + rutaAStar[indice].first * tamanoCelda + 19.f, origenY + rutaAStar[indice].second * tamanoCelda + 19.f);
            marca.setFillColor(sf::Color(255, 214, 120, 180));
            ventana.draw(marca);
        }
    }
    {
        const auto& posicionFurby = rutaAStar.empty() ? inicioAStar : rutaAStar[std::min(pasoAStar, rutaAStar.size() - 1)];
        if (tieneFurbyAStar) {
            sf::Sprite furby(texturaFurbyAStar);
            sf::Vector2u tamano = texturaFurbyAStar.getSize();
            float escala = std::min(38.f / static_cast<float>(tamano.x), 38.f / static_cast<float>(tamano.y));
            furby.setScale(escala, escala);
            furby.setPosition(origenX + posicionFurby.first * tamanoCelda + (tamanoCelda - tamano.x * escala) / 2.f, origenY + posicionFurby.second * tamanoCelda + (tamanoCelda - tamano.y * escala) / 2.f);
            ventana.draw(furby);
        } else {
            sf::CircleShape furby(13.f);
            furby.setPosition(origenX + posicionFurby.first * tamanoCelda + 11.f, origenY + posicionFurby.second * tamanoCelda + 11.f);
            furby.setFillColor(sf::Color(224, 126, 55));
            furby.setOutlineThickness(3.f);
            furby.setOutlineColor(sf::Color::White);
            ventana.draw(furby);
        }

        if (tieneComidaAStar) {
            sf::Sprite comida(texturaComidaAStar);
            sf::Vector2u tamano = texturaComidaAStar.getSize();
            comida.setScale(30.f / static_cast<float>(tamano.x), 30.f / static_cast<float>(tamano.y));
            comida.setPosition(origenX + objetivoAStar.first * tamanoCelda + 9.f, origenY + objetivoAStar.second * tamanoCelda + 9.f);
            ventana.draw(comida);
        } else {
            sf::CircleShape comida(13.f);
            comida.setPosition(origenX + objetivoAStar.first * tamanoCelda + 11.f, origenY + objetivoAStar.second * tamanoCelda + 11.f);
            comida.setFillColor(sf::Color(90, 190, 105));
            comida.setOutlineThickness(3.f);
            comida.setOutlineColor(sf::Color(255, 214, 120));
            ventana.draw(comida);
        }
    }
    if (fuenteCargada) {
        sf::Text leyenda(esperandoComidaAStar ? "Haz clic en la comida para calcular la ruta" : "El Furby sigue el camino mas corto hasta la comida", fuente, 14);
        leyenda.setPosition(205.f, 470.f);
        leyenda.setFillColor(sf::Color::White);
        ventana.draw(leyenda);
    }
}

void JuegoAgente::DibujarBarra(const std::string& etiqueta, int valor, float x, float y, sf::Color color) {
    sf::RectangleShape fondo(sf::Vector2f(220.f, 24.f));
    fondo.setPosition(x, y);
    fondo.setFillColor(sf::Color(20, 18, 30, 210));
    fondo.setOutlineThickness(1.f);
    fondo.setOutlineColor(sf::Color(255, 249, 226, 150));
    sf::RectangleShape progreso(sf::Vector2f(2.2f * static_cast<float>(valor), 24.f));
    progreso.setPosition(x, y);
    progreso.setFillColor(color);
    ventana.draw(fondo);
    ventana.draw(progreso);
    if (fuenteCargada) {
        sf::Text textoBarra(etiqueta + ": " + std::to_string(valor) + "%", fuente, 16);
        textoBarra.setPosition(x + 8.f, y + 2.f);
        textoBarra.setFillColor(sf::Color::White);
        ventana.draw(textoBarra);
    }
}

void JuegoAgente::CrearInterfaz() {
    if (!fuenteCargada) return;
    botones.reserve(6);
    textoEstado.setFont(fuente); textoEstado.setCharacterSize(24); textoEstado.setFillColor(sf::Color::White); textoEstado.setPosition(155.f, 23.f);
    textoControles.setFont(fuente); textoControles.setCharacterSize(18); textoControles.setFillColor(sf::Color(255, 214, 120)); textoControles.setPosition(148.f, 365.f);
    textoNombre.setFont(fuente); textoNombre.setCharacterSize(24); textoNombre.setFillColor(sf::Color(55, 43, 35));
    botones.emplace_back("REVIVIR", 280.f, 410.f, &fuente);
    const std::array<std::string, 5> acciones = { "COMER (C)", "BANAR (B)", "DORMIR (D)", "DESPERTAR (W)", "CURAR (R)" };
    const std::array<std::string, 5> iconos = {
        "assets/Images/b_comer.png", "assets/Images/b_banar.png",
        "assets/Images/b_dormir.png", "assets/Images/b_despertar.png",
        "assets/Images/b_curar.png"
    };
    for (std::size_t indice = 0; indice < acciones.size(); ++indice) {
        float x = 20.f + indice * 160.f;
        float y = 410.f;
        botones.emplace_back(acciones[indice], x, y, &fuente, iconos[indice]);
    }
}

void JuegoAgente::DibujarPantallaMuerte() {
    sf::RectangleShape panel(sf::Vector2f(500.f, 150.f));
    panel.setPosition(150.f, 300.f);
    panel.setFillColor(sf::Color(55, 43, 35, 235));
    ventana.draw(panel);
    if (fuenteCargada) {
        sf::Text mensaje("Tu Furby ha muerto", fuente, 28);
        mensaje.setPosition(245.f, 320.f);
        mensaje.setFillColor(sf::Color::White);
        ventana.draw(mensaje);
    }
}

void JuegoAgente::MostrarPantallaInicio() {
    textoNombre.setString(nombreEscrito.isEmpty() ? "Escribe un nombre" : nombreEscrito);
}

void JuegoAgente::IniciarPartida() {
    if (nombreEscrito.isEmpty()) nombreEscrito = "Furby";
    personaje.EstablecerNombre(nombreEscrito.toAnsiString());
    enPantallaInicio = false;
    escribiendoNombre = false;
    MostrarPantallaInicio();
}

void JuegoAgente::DibujarPantallaInicio() {
    ventana.clear(sf::Color(247, 232, 196));
    if (tienePantallaInicio) {
        sf::Vector2u tamano = texturaPantallaInicio.getSize();
        spritePantallaInicio.setScale(800.f / static_cast<float>(tamano.x), 600.f / static_cast<float>(tamano.y));
        ventana.draw(spritePantallaInicio);
    }
    sf::RectangleShape franja(sf::Vector2f(800.f, 220.f));
    franja.setPosition(0.f, 0.f);
    franja.setFillColor(sf::Color(55, 43, 35));
    if (!tienePantallaInicio) ventana.draw(franja);
    if (fuenteCargada) {
        sf::RectangleShape placa(sf::Vector2f(470.f, 42.f));
        placa.setPosition(165.f, 250.f);
        placa.setFillColor(sf::Color(255, 249, 226, 235));
        ventana.draw(placa);
        sf::Text indicacion("Escribe el nombre de tu Furby", fuente, 22);
        indicacion.setPosition(205.f, 258.f);
        indicacion.setFillColor(sf::Color(55, 43, 35));
        ventana.draw(indicacion);
        sf::RectangleShape campo(sf::Vector2f(390.f, 52.f));
        campo.setPosition(205.f, 305.f);
        campo.setFillColor(sf::Color(255, 249, 226));
        campo.setOutlineThickness(3.f);
        campo.setOutlineColor(escribiendoNombre ? sf::Color(224, 126, 55) : sf::Color(55, 43, 35));
        ventana.draw(campo);
        textoNombre.setPosition(220.f, 316.f);
        ventana.draw(textoNombre);
        sf::RectangleShape boton(sf::Vector2f(250.f, 55.f));
        boton.setPosition(275.f, 390.f);
        boton.setFillColor(sf::Color(224, 126, 55));
        ventana.draw(boton);
        sf::Text comenzar("COMENZAR", fuente, 22);
        comenzar.setPosition(328.f, 405.f);
        comenzar.setFillColor(sf::Color::White);
        ventana.draw(comenzar);
        sf::Text ayuda("ENTER para comenzar", fuente, 16);
        ayuda.setPosition(300.f, 470.f);
        ayuda.setFillColor(sf::Color(55, 43, 35));
        ventana.draw(ayuda);
    }
    ventana.display();
}

int main() {
    JuegoAgente juego;
    juego.IniciarJuego();
    return 0;
}
