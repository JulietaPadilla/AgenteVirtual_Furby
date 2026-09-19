#include "JuegoAgente.h"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <algorithm>
#include <array>
#include <cmath>

namespace {
const std::array<std::string, 9> nombresSprites = {
    "huevo.png", "normal.png", "hambre.png", "sucio.png",
    "enfermo.png", "cansado.png", "hambre_cansado.png",
    "hambre_sucio.png", "muerto.png"
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
    if (estadoActual == EstadoFurby::HAMBRE || estadoActual == EstadoFurby::HAMBRE_CANSADO || estadoActual == EstadoFurby::HAMBRE_SUCIO) {
        hambre = std::min(100, hambre + 35);
        ReproducirAccion(AccionFurby::COMER);
        if (estadoActual == EstadoFurby::HAMBRE_CANSADO) CambiarEstado(EstadoFurby::CANSADO);
        else if (estadoActual == EstadoFurby::HAMBRE_SUCIO) CambiarEstado(EstadoFurby::SUCIO);
        else CambiarEstado(EstadoFurby::NORMAL);
    }
}
void Furby::Banar() {
    if (estadoActual == EstadoFurby::SUCIO || estadoActual == EstadoFurby::HAMBRE_SUCIO) {
        higiene = std::min(100, higiene + 40);
        ReproducirAccion(AccionFurby::BANAR);
        if (estadoActual == EstadoFurby::HAMBRE_SUCIO) CambiarEstado(EstadoFurby::HAMBRE);
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
    if ((estadoActual == EstadoFurby::NORMAL || estadoActual == EstadoFurby::HAMBRE_CANSADO || estadoActual == EstadoFurby::CANSADO) && sueno <= 50) {
        durmiendo = true;
        ReproducirAccion(AccionFurby::DORMIR);
        if (estadoActual == EstadoFurby::NORMAL || estadoActual == EstadoFurby::HAMBRE_CANSADO) {
            CambiarEstado(EstadoFurby::CANSADO);
        }
    }
}
void Furby::Despertar() {
    if (estadoActual == EstadoFurby::CANSADO || estadoActual == EstadoFurby::HAMBRE_CANSADO) {
        durmiendo = false;
        ReproducirAccion(AccionFurby::DESPERTAR);
        if (estadoActual == EstadoFurby::HAMBRE_CANSADO) CambiarEstado(EstadoFurby::HAMBRE);
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
        if (relojHigiene.getElapsedTime().asSeconds() >= 8.f) { higiene = std::max(0, higiene - 5); relojHigiene.restart(); }
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
        if (hambreBaja && cansancio) CambiarEstado(EstadoFurby::HAMBRE_CANSADO);
        else if (hambreBaja && higieneBaja) CambiarEstado(EstadoFurby::HAMBRE_SUCIO);
        else if (hambreBaja) CambiarEstado(EstadoFurby::HAMBRE);
        else if (higieneBaja) CambiarEstado(EstadoFurby::SUCIO);
        else if (cansancio) CambiarEstado(EstadoFurby::CANSADO);
        break;
    case EstadoFurby::HAMBRE:
        if (cansancio) CambiarEstado(EstadoFurby::HAMBRE_CANSADO);
        else if (higieneBaja) CambiarEstado(EstadoFurby::HAMBRE_SUCIO);
        break;
    case EstadoFurby::SUCIO:
        if (hambreBaja) CambiarEstado(EstadoFurby::HAMBRE_SUCIO);
        break;
    case EstadoFurby::CANSADO:
        if (hambreBaja) CambiarEstado(EstadoFurby::HAMBRE_CANSADO);
        break;
    case EstadoFurby::HAMBRE_CANSADO:
        if (higieneBaja) CambiarEstado(EstadoFurby::HAMBRE_SUCIO);
        break;
    case EstadoFurby::HAMBRE_SUCIO:
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

BotonInterfaz::BotonInterfaz(const std::string& etiqueta, float x, float y, const sf::Font* fuente)
    : forma(sf::Vector2f(140.f, 38.f)) {
    forma.setPosition(x, y);
    forma.setFillColor(sf::Color(224, 126, 55));
    texto.setString(etiqueta);
    texto.setCharacterSize(16);
    texto.setFillColor(sf::Color::White);
    if (fuente != nullptr) texto.setFont(*fuente);
    texto.setPosition(x + 10.f, y + 8.f);
}

bool BotonInterfaz::FueClickeado(sf::Vector2i posicionRaton) const {
    return forma.getGlobalBounds().contains(static_cast<float>(posicionRaton.x), static_cast<float>(posicionRaton.y));
}

void BotonInterfaz::Dibujar(sf::RenderWindow& ventana) const { ventana.draw(forma); ventana.draw(texto); }
void BotonInterfaz::EstablecerPosicion(float x, float y) { forma.setPosition(x, y); texto.setPosition(x + 10.f, y + 8.f); }

JuegoAgente::JuegoAgente()
    : personaje("Furby"), ventana(sf::VideoMode(800, 600), "Agente Virtual Furby"), fuenteCargada(false), nombreEscrito(), enPantallaInicio(true), escribiendoNombre(true), tienePantallaInicio(false), tieneFondoJuego(false) {
    ventana.setFramerateLimit(60);
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
    ventana.requestFocus();
    if (!musica.openFromFile("assets/Music/musica.mp3")) {
        musica.openFromFile("../assets/Music/musica.mp3");
    }
    musica.setLoop(true);
    musica.setVolume(35.f);
    musica.play();
    CrearInterfaz();
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
    personaje.ActualizarTiempo();
    personaje.ActualizarAnimacion();
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
        sf::RectangleShape panelEstado(sf::Vector2f(270.f, 254.f));
        panelEstado.setPosition(35.f, 78.f);
        panelEstado.setFillColor(sf::Color(35, 28, 48, 215));
        panelEstado.setOutlineThickness(2.f);
        panelEstado.setOutlineColor(sf::Color(130, 195, 177));
        ventana.draw(panelEstado);
        DibujarBarra("Hambre", personaje.ObtenerHambre(), 45.f, 100.f, sf::Color(224, 126, 55));
        DibujarBarra("Higiene", personaje.ObtenerHigiene(), 45.f, 160.f, sf::Color(55, 150, 190));
        DibujarBarra("Salud", personaje.ObtenerSalud(), 45.f, 220.f, sf::Color(80, 170, 90));
        DibujarBarra("Sueno", personaje.ObtenerSueno(), 45.f, 280.f, sf::Color(110, 90, 170));
    }
    if (!estaEnHuevo && !estaMuerto) {
        sf::RectangleShape panelAcciones(sf::Vector2f(540.f, 150.f));
        panelAcciones.setPosition(130.f, 350.f);
        panelAcciones.setFillColor(sf::Color(35, 28, 48, 220));
        panelAcciones.setOutlineThickness(2.f);
        panelAcciones.setOutlineColor(sf::Color(130, 195, 177));
        ventana.draw(panelAcciones);
    }
    if (fuenteCargada && !estaEnHuevo) { ventana.draw(textoEstado); ventana.draw(textoControles); }
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
    for (std::size_t indice = 0; indice < botones.size(); ++indice) {
        if (!estaEnHuevo && ((estaMuerto && indice == 0) || (!estaMuerto && indice >= 1))) botones[indice].Dibujar(ventana);
    }
    ventana.display();
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
    textoEstado.setFont(fuente); textoEstado.setCharacterSize(24); textoEstado.setFillColor(sf::Color::White); textoEstado.setPosition(155.f, 23.f);
    textoControles.setFont(fuente); textoControles.setCharacterSize(18); textoControles.setFillColor(sf::Color(255, 214, 120)); textoControles.setPosition(148.f, 365.f);
    textoNombre.setFont(fuente); textoNombre.setCharacterSize(24); textoNombre.setFillColor(sf::Color(55, 43, 35));
    botones.emplace_back("REVIVIR", 280.f, 410.f, &fuente);
    const std::array<std::string, 5> acciones = { "COMER (C)", "BANAR (B)", "DORMIR (D)", "DESPERTAR (W)", "CURAR (R)" };
    for (std::size_t indice = 0; indice < acciones.size(); ++indice) {
        float x = 145.f + (indice % 3) * 175.f;
        float y = 410.f + (indice / 3) * 45.f;
        botones.emplace_back(acciones[indice], x, y, &fuente);
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
