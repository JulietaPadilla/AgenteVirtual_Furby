#include "JuegoAgente.h"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <algorithm>
#include <array>
#include <cmath>

namespace {
const std::array<std::string, 7> nombresSprites = {
    "huevo.png", "normal.png", "hambre.png", "sucio.png",
    "enfermo.png", "con_sueno.png", "muerto.png"
};

int IndiceEstado(EstadoFurby estado) {
    return static_cast<int>(estado);
}
}

const char* NombreEstado(EstadoFurby estado) {
    switch (estado) {
    case EstadoFurby::HUEVO: return "HUEVO";
    case EstadoFurby::NORMAL: return "NORMAL";
    case EstadoFurby::HAMBRE: return "HAMBRE";
    case EstadoFurby::SUCIO: return "SUCIO";
    case EstadoFurby::ENFERMO: return "ENFERMO";
    case EstadoFurby::CON_SUENO: return "CON SUENO";
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
      tHambre(30.f), tVida(60.f), tEnfermedad(20.f), tSueno(20.f) {
        tieneSpritePorEstado.fill(false);
                framesPorEstado.fill(1);
                framesPorEstado[IndiceEstado(EstadoFurby::HUEVO)] = totalFrames;
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
    if (estadoActual != EstadoFurby::HUEVO && estadoActual != EstadoFurby::MUERTO && !durmiendo) {
        hambre = std::min(100, hambre + 35);
        tVida.Pausar();
        if (estadoActual == EstadoFurby::HAMBRE) CambiarEstado(EstadoFurby::NORMAL);
    }
}
void Furby::Banar() {
    if (estadoActual != EstadoFurby::HUEVO && estadoActual != EstadoFurby::MUERTO && !durmiendo) {
        higiene = std::min(100, higiene + 40);
        if (estadoActual == EstadoFurby::SUCIO) CambiarEstado(EstadoFurby::NORMAL);
    }
}
void Furby::Enfermar() { if (estadoActual != EstadoFurby::HUEVO && estadoActual != EstadoFurby::MUERTO) CambiarEstado(EstadoFurby::ENFERMO); }
void Furby::Curar() {
    if (estadoActual != EstadoFurby::HUEVO && estadoActual != EstadoFurby::MUERTO && !durmiendo) {
        salud = std::min(100, salud + 40);
        tVida.Pausar();
        if (estadoActual == EstadoFurby::ENFERMO) CambiarEstado(EstadoFurby::NORMAL);
    }
}
void Furby::HacerPopo() { if (estadoActual == EstadoFurby::NORMAL) CambiarEstado(EstadoFurby::SUCIO); }
void Furby::Dormir() {
    if (estadoActual != EstadoFurby::HUEVO && estadoActual != EstadoFurby::MUERTO && !durmiendo) {
        durmiendo = true;
        CambiarEstado(EstadoFurby::CON_SUENO);
    }
}
void Furby::Despertar() {
    if (durmiendo) {
        durmiendo = false;
        ActualizarEstadoPorNecesidad();
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
            salud = std::max(0, salud - (deficit >= 180 ? 8 : deficit >= 90 ? 4 : 0));
            relojSalud.restart();
        }
    } else {
        if (relojHambre.getElapsedTime().asSeconds() >= 8.f) { hambre = std::max(0, hambre - 5); relojHambre.restart(); }
        if (relojHigiene.getElapsedTime().asSeconds() >= 14.f) { higiene = std::max(0, higiene - 3); relojHigiene.restart(); }
        if (relojSueno.getElapsedTime().asSeconds() >= 10.f) { sueno = std::max(0, sueno - 5); relojSueno.restart(); }
        if (relojSalud.getElapsedTime().asSeconds() >= 6.f) {
            int deficit = (100 - hambre) + (100 - higiene) + (100 - sueno);
            int deterioro = deficit >= 180 ? 8 : deficit >= 90 ? 4 : 0;
            salud = std::max(0, salud - deterioro);
            relojSalud.restart();
        }
    }
    if (salud <= 0) { Morir(); return; }
    if (!durmiendo) ActualizarEstadoPorNecesidad();
}

void Furby::ForzarEstado(EstadoFurby nuevoEstado) {
    if (nuevoEstado == EstadoFurby::HUEVO) return;

    switch (nuevoEstado) {
    case EstadoFurby::NORMAL:
        hambre = 100;
        higiene = 100;
        salud = 100;
        sueno = 100;
        break;
    case EstadoFurby::HAMBRE:
        hambre = 10;
        break;
    case EstadoFurby::SUCIO:
        higiene = 10;
        break;
    case EstadoFurby::ENFERMO:
        salud = 10;
        break;
    case EstadoFurby::CON_SUENO:
        sueno = 10;
        break;
    case EstadoFurby::MUERTO:
        hambre = 0;
        higiene = 0;
        salud = 0;
        sueno = 0;
        break;
    case EstadoFurby::HUEVO:
        return;
    }

    CambiarEstado(nuevoEstado);
}

void Furby::ActualizarEstadoPorNecesidad() {
    int menorValor = std::min({ hambre, higiene, salud, sueno });
    if (menorValor >= 50) {
        if (estadoActual != EstadoFurby::NORMAL) CambiarEstado(EstadoFurby::NORMAL);
        return;
    }

    if (menorValor == salud) CambiarEstado(EstadoFurby::ENFERMO);
    else if (menorValor == hambre) CambiarEstado(EstadoFurby::HAMBRE);
    else if (menorValor == higiene) CambiarEstado(EstadoFurby::SUCIO);
    else CambiarEstado(EstadoFurby::CON_SUENO);
}
void Furby::ActualizarAnimacion() {
    if (estadoActual == EstadoFurby::HUEVO) return;
    if (relojAnimacion.getElapsedTime().asSeconds() < 0.18f) return;
    relojAnimacion.restart();
    int cantidadFrames = framesPorEstado[IndiceEstado(estadoActual)];
    actualFrame = (actualFrame + 1) % cantidadFrames;
}

void Furby::Dibujar(sf::RenderWindow& ventana) const {
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

void Furby::CambiarEstado(EstadoFurby nuevoEstado) {
    estadoActual = nuevoEstado;
    actualFrame = 0;
    tEnfermedad.Pausar();
    if (nuevoEstado == EstadoFurby::HAMBRE || nuevoEstado == EstadoFurby::SUCIO) tEnfermedad.Reiniciar();
    if (nuevoEstado == EstadoFurby::HAMBRE || nuevoEstado == EstadoFurby::ENFERMO) tVida.Reiniciar();
    if (nuevoEstado == EstadoFurby::CON_SUENO) tSueno.Reiniciar();
    if (nuevoEstado == EstadoFurby::NORMAL) tHambre.Reiniciar();
}

void Furby::PrepararTemporizadores() {
    tHambre.Pausar(); tVida.Pausar(); tEnfermedad.Pausar(); tSueno.Pausar();
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
    : personaje("Furby"), ventana(sf::VideoMode(800, 600), "Agente Virtual Furby"), fuenteCargada(false) {
    ventana.setFramerateLimit(60);
    const std::array<std::string, 3> rutasFuente = {
        "assets/Fonts/arial.ttf", "../assets/Fonts/arial.ttf", "C:/Windows/Fonts/arial.ttf"
    };
    for (const auto& ruta : rutasFuente) {
        if (fuente.loadFromFile(ruta)) { fuenteCargada = true; break; }
    }
    if (!personaje.CargarSpritesPorEstado("assets/Images/")) {
        personaje.CargarSpritesPorEstado("../assets/Images/");
    }
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
        if (evento.type == sf::Event::KeyPressed) ProcesarTecla(evento.key.code);
        if (evento.type == sf::Event::MouseButtonPressed && evento.mouseButton.button == sf::Mouse::Left) {
            ProcesarClick(sf::Vector2i(evento.mouseButton.x, evento.mouseButton.y));
        }
    }
}

void JuegoAgente::ProcesarTecla(sf::Keyboard::Key tecla) {
    switch (tecla) {
    case sf::Keyboard::C: personaje.Comer(); break;
    case sf::Keyboard::B: personaje.Banar(); break;
    case sf::Keyboard::P: personaje.HacerPopo(); break;
    case sf::Keyboard::E: personaje.Enfermar(); break;
    case sf::Keyboard::R: personaje.Curar(); break;
    case sf::Keyboard::D: personaje.Dormir(); break;
    case sf::Keyboard::W: personaje.Despertar(); break;
    case sf::Keyboard::K: personaje.Morir(); break;
    case sf::Keyboard::Num1: personaje.ForzarEstado(EstadoFurby::NORMAL); break;
    case sf::Keyboard::Num2: personaje.ForzarEstado(EstadoFurby::HAMBRE); break;
    case sf::Keyboard::Num3: personaje.ForzarEstado(EstadoFurby::SUCIO); break;
    case sf::Keyboard::Num4: personaje.ForzarEstado(EstadoFurby::ENFERMO); break;
    case sf::Keyboard::Num5: personaje.ForzarEstado(EstadoFurby::CON_SUENO); break;
    case sf::Keyboard::Num6: personaje.ForzarEstado(EstadoFurby::MUERTO); break;
    case sf::Keyboard::V: personaje.Revivir(); break;
    default: break;
    }
}

void JuegoAgente::ProcesarClick(sf::Vector2i posicion) {
    if (personaje.ObtenerEstado() == EstadoFurby::HUEVO) {
        sf::FloatRect zonaHuevo(300.f, 150.f, 200.f, 200.f);
        if (zonaHuevo.contains(static_cast<float>(posicion.x), static_cast<float>(posicion.y))) {
            personaje.GolpearHuevo();
        }
        return;
    }

    for (std::size_t indice = 0; indice < botones.size(); ++indice) {
        if (!botones[indice].FueClickeado(posicion)) continue;
        if (personaje.ObtenerEstado() == EstadoFurby::MUERTO && indice == 5) personaje.Revivir();
        else if (indice < 5) personaje.ForzarEstado(static_cast<EstadoFurby>(indice + 1));
        else if (personaje.ObtenerEstado() != EstadoFurby::MUERTO && indice >= 6) {
            switch (indice - 6) {
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
    personaje.ActualizarTiempo();
    personaje.ActualizarAnimacion();
    if (fuenteCargada) {
        textoEstado.setString("Furby: " + personaje.ObtenerNombre() + " | Estado: " + NombreEstado(personaje.ObtenerEstado()));
        if (personaje.ObtenerEstado() == EstadoFurby::HUEVO) textoControles.setString("Haz clic 3 veces sobre el huevo");
        else textoControles.setString("ACCIONES");
    }
}

void JuegoAgente::Dibujar() {
    ventana.clear(sf::Color(247, 232, 196));
    personaje.Dibujar(ventana);
    bool estaEnHuevo = personaje.ObtenerEstado() == EstadoFurby::HUEVO;
    if (!estaEnHuevo) {
        DibujarBarra("Hambre", personaje.ObtenerHambre(), 25.f, 100.f, sf::Color(224, 126, 55));
        DibujarBarra("Higiene", personaje.ObtenerHigiene(), 25.f, 160.f, sf::Color(55, 150, 190));
        DibujarBarra("Salud", personaje.ObtenerSalud(), 25.f, 220.f, sf::Color(80, 170, 90));
        DibujarBarra("Sueno", personaje.ObtenerSueno(), 25.f, 280.f, sf::Color(110, 90, 170));
    }
    if (fuenteCargada && !estaEnHuevo) { ventana.draw(textoEstado); ventana.draw(textoControles); }
    if (estaEnHuevo && fuenteCargada) {
        sf::Text instruccion("Golpea el huevo para abrirlo", fuente, 22);
        instruccion.setPosition(250.f, 380.f);
        instruccion.setFillColor(sf::Color(55, 43, 35));
        ventana.draw(instruccion);
        sf::Text golpes("Golpes: " + std::to_string(personaje.ObtenerGolpesHuevo()) + "/5", fuente, 20);
        golpes.setPosition(335.f, 420.f);
        golpes.setFillColor(sf::Color(55, 43, 35));
        ventana.draw(golpes);
    }
    bool estaMuerto = personaje.ObtenerEstado() == EstadoFurby::MUERTO;
    if (estaMuerto) DibujarPantallaMuerte();
    for (std::size_t indice = 0; indice < botones.size(); ++indice) {
        if (!estaEnHuevo && (indice < 5 || (estaMuerto && indice == 5) || (!estaMuerto && indice >= 6))) botones[indice].Dibujar(ventana);
    }
    ventana.display();
}

void JuegoAgente::DibujarBarra(const std::string& etiqueta, int valor, float x, float y, sf::Color color) {
    sf::RectangleShape fondo(sf::Vector2f(220.f, 24.f));
    fondo.setPosition(x, y);
    fondo.setFillColor(sf::Color(210, 200, 180));
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
    textoEstado.setFont(fuente); textoEstado.setCharacterSize(24); textoEstado.setFillColor(sf::Color(55, 43, 35)); textoEstado.setPosition(25.f, 20.f);
    textoControles.setFont(fuente); textoControles.setCharacterSize(18); textoControles.setFillColor(sf::Color(55, 43, 35)); textoControles.setPosition(25.f, 370.f);
    const std::array<std::string, 5> etiquetas = { "NORMAL", "HAMBRE", "SUCIO", "ENFERMO", "CON SUENO" };
    for (std::size_t indice = 0; indice < etiquetas.size(); ++indice) {
        float x = 560.f + (indice % 2) * 155.f;
        float y = 100.f + (indice / 2) * 45.f;
        botones.emplace_back(etiquetas[indice], x, y, &fuente);
    }
    botones.emplace_back("REVIVIR", 280.f, 410.f, &fuente);
    const std::array<std::string, 5> acciones = { "COMER (C)", "BANAR (B)", "DORMIR (D)", "DESPERTAR (W)", "CURAR (R)" };
    for (std::size_t indice = 0; indice < acciones.size(); ++indice) {
        float x = 25.f + (indice % 3) * 175.f;
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
    textoEstado.setString("Furby: " + personaje.ObtenerNombre() + " | Estado: " + NombreEstado(personaje.ObtenerEstado()));
    textoControles.setString("Haz clic 3 veces sobre el huevo para que nazca");
}

int main() {
    JuegoAgente juego;
    juego.IniciarJuego();
    return 0;
}
