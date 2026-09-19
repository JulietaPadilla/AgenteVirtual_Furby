# Agente Virtual Furby

**Autora:** Julieta Padilla Casillas

[![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![SFML](https://img.shields.io/badge/SFML-2.5%2B-green.svg)](https://www.sfml-dev.org/)
[![Build](https://img.shields.io/badge/build-CMake-orange.svg)](https://cmake.org/)
[![License](https://img.shields.io/badge/license-Educational-orange.svg)]()

## Descripcion

Agente Virtual Furby es un videojuego tipo Tamagotchi desarrollado en C++17 y SFML. El jugador cuida a un Furby, atiende sus necesidades y observa como cambia entre estados mediante una maquina de estados.

El juego incluye una pantalla de inicio para asignar un nombre, fondos pixel art, musica, animaciones y estados combinados como `HAMBRE + CANSADO` y `HAMBRE + SUCIO`.

---

## Caracteristicas principales

- **Maquina de estados:** el Furby cambia mediante acciones y condiciones definidas, no por cambios arbitrarios.
- **Necesidades del Furby:** hambre, higiene, salud y sueno.
- **Estados combinados:** hambre con cansancio y hambre con suciedad.
- **Nacimiento interactivo:** el huevo se abre despues de cinco golpes.
- **Nombre personalizado:** cada jugador puede escribir el nombre del Furby desde el teclado.
- **Sprites y animaciones:** imagenes para cada estado y acciones como comer, banarse, dormir y curarse.
- **Musica de fondo:** reproduccion continua durante el juego.
- **Fondos pixel art:** pantalla de inicio y escenario principal configurables.
- **Pantalla de muerte y reanimacion:** el jugador puede revivir al Furby despues de su muerte.
- **Minijuego A* opcional:** cuando el Furby tiene hambre, el jugador puede abrir una cuadricula y elegir si quiere buscar comida.

---

## Requisitos

- C++17 o superior.
- SFML 2.5 o superior.
- MinGW64 en Windows o un compilador compatible.
- CMake 3.16 o superior para compilar con CMake.

En Windows con MinGW64, agrega `C:\msys64\mingw64\bin` al `PATH` si el sistema no encuentra las DLL de SFML.

---

## Instalacion y compilacion

### Compilacion directa con MinGW64

Desde CMD o PowerShell, ubicado en la carpeta raiz del proyecto:

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic -Iinclude src/main.cpp -o bin/AgenteVirtualFurby.exe -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
```

### Compilacion con CMake

```powershell
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build
```

### Ejecutar

```powershell
.\bin\AgenteVirtualFurby.exe
```

Si aparecen errores de DLL, copia junto al ejecutable las bibliotecas de SFML y sus dependencias de MinGW64, o agrega la carpeta correspondiente al `PATH`.

---

## Inicio del juego

1. Ejecuta el programa.
2. Haz clic en el campo de nombre.
3. Escribe el nombre del Furby con el teclado.
4. Presiona `Enter` o selecciona `COMENZAR`.
5. Golpea el huevo cinco veces para que nazca.

---

## Controles

| Tecla | Accion |
|:-----:|:-------|
| `C` | Comer |
| `B` | Banar |
| `P` | Hacer popo |
| `D` | Dormir cuando el sueno es bajo |
| `W` | Despertar |
| `R` | Curar |
| `K` | Probar la muerte |
| `V` | Revivir al Furby |

Tambien puedes utilizar los botones visibles en pantalla, que muestran un icono de la accion correspondiente.

### Minijuego de comida con A*

Cuando el Furby entra en `HAMBRE`, `HAMBRE + CANSADO` o `HAMBRE + SUCIO`, aparece el boton `BUSCAR COMIDA (A)`. El minijuego es opcional: si el jugador no lo selecciona, puede continuar jugando normalmente. Cada episodio de hambre permite hasta tres busquedas, con obstaculos y comida diferentes en cada mapa. Al abrirlo, debe hacer clic sobre la comida; en ese momento A* calcula la ruta de menor costo desde el Furby hasta la comida usando:

```text
f(n) = g(n) + h(n)
```

`g(n)` es el costo acumulado de los movimientos y `h(n)` es la distancia Manhattan hasta la comida. Las celdas moradas son obstaculos, la marca naranja representa al Furby y la verde representa la comida. Al llegar al objetivo se ejecuta `COMER()` y se resuelve la parte de hambre del estado.

---

## Maquina de estados

Los estados principales son:

```text
HUEVO
NORMAL
HAMBRE
SUCIO
CANSADO
HAMBRE + CANSADO
HAMBRE + SUCIO
ENFERMO
MUERTO
```

### Umbrales de necesidades

- El hambre baja cada 8 segundos.
- La higiene baja cada 10 segundos, un poco mas lento que el hambre.
- El sueno baja cada 10 segundos.
- `HAMBRE`, `SUCIO` y `CANSADO` aparecen cuando la necesidad correspondiente llega al 50% o menos.
- `ENFERMO` aparece cuando la salud llega al 30% o menos.
- `MUERTO` aparece cuando la salud llega a 0%.
- `COMER`, `BANAR`, `DORMIR`, `DESPERTAR` y `CURAR` producen las transiciones permitidas.

El diagrama completo se encuentra en [`docs/diagrama_estados.puml`](docs/diagrama_estados.puml).

---

## Recursos visuales y audio

### Imagenes por estado

Coloca los siguientes archivos en `assets/Images/`:

```text
huevo.png
normal.png
hambre.png
sucio.png
enfermo.png
cansado.png
hambre_cansado.png
hambre_sucio.png
muerto.png
```

`huevo.png` debe ser una tira horizontal de cinco frames. Los demas sprites se cargan como imagenes individuales.

### Fondos

```text
assets/Images/pantalla_inicio.png
assets/Images/fondo_juego.png
```

Ambos fondos se ajustan automaticamente a la ventana de 800 x 600.

### Animaciones y musica

```text
assets/Images/acciones/comer.png
assets/Images/acciones/banar.png
assets/Images/acciones/despertar.png
assets/Images/acciones/dormir.png
assets/Images/acciones/curar.png
assets/Music/musica.mp3
assets/fonts/F25_Bank_Printer.otf
```

Las animaciones pueden ser imagenes individuales o tiras horizontales de cinco frames.

### Sprites del minijuego A*

El algoritmo ya esta preparado para usar estos recursos opcionales:

```text
assets/Images/astar/piso.png
assets/Images/astar/obstaculo.png
assets/Images/astar/comida.png
assets/Images/astar/ruta.png
```

`piso.png` se dibuja en las celdas transitables, `obstaculo.png` en las celdas bloqueadas, `comida.png` en el objetivo y `ruta.png` sobre el camino calculado. Si alguno no existe, se utiliza un marcador grafico de respaldo.

---

## Estructura del proyecto

```text
AgenteVirtual_Furby/
├── assets/
│   ├── fonts/       # Fuente del juego
│   ├── Images/      # Sprites, fondos y animaciones
│   ├── Music/       # Musica de fondo
│   └── Puntajes/   # Recursos de puntajes
├── bin/             # Ejecutable generado
├── docs/            # Diagramas PlantUML
├── include/         # Archivos de cabecera
├── src/             # Codigo fuente
├── CMakeLists.txt
└── README.md
```

---

## Diagramas

- [Diagrama de estados](docs/diagrama_estados.puml)
- [Diagrama de clases](docs/diagrama.puml)

Puedes visualizar los archivos `.puml` con la extension PlantUML de VS Code y Java instalado.

---

## Autoria y licencia

- **Autora:** Julieta Padilla Casillas
- **Proposito:** proyecto educativo
- **Tecnologias:** C++17, SFML y CMake

Los recursos visuales, la musica y la fuente se utilizan con fines educativos.

---

## Comienza a jugar

Cuida a tu Furby, manten sus necesidades equilibradas y descubre como evoluciona su estado.
