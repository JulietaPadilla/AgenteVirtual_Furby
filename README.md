# Agente Virtual Furby

Juego Tamagotchi en C++17 con SFML y MinGW64.

## Sprites

Coloca estos sprites en `assets/Images/`:

```text
huevo.png       normal.png      hambre.png     sucio.png
enfermo.png     con_sueno.png   muerto.png
```

El programa busca los recursos desde la raíz del proyecto y también desde `bin`, por lo que puedes ejecutar el `.exe` dentro de `bin`.

El programa carga automáticamente el sprite correspondiente al estado. Si no existe, dibuja una figura de respaldo. Usa preferiblemente PNG con transparencia; una imagen de 256 x 256 píxeles es un buen tamaño inicial.

Actualmente `huevo.png` es una tira horizontal de 5 frames: el juego divide automáticamente su ancho y muestra una etapa distinta por clic; el Furby nace al quinto clic. Los sprites `normal.png`, `hambre.png`, `sucio.png`, `enfermo.png`, `con_sueno.png` y `muerto.png` se cargan como imágenes individuales y se muestran completos.

Para cambiar el archivo o añadir otro sprite, modifica esta línea en `src/main.cpp`:

```cpp
personaje.CargarSpritesPorEstado("assets/Images/");
```

## Fuente

Para usar una fuente propia, crea `assets/Fonts/` y coloca allí `arial.ttf`. Si no está, el programa intenta usar la fuente de Windows en `C:/Windows/Fonts/arial.ttf`.

## Compilar con MinGW64

Desde CMD o PowerShell, abierto en la carpeta raíz del proyecto:

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic -Iinclude src/main.cpp -o bin/AgenteVirtualFurby.exe -lsfml-graphics -lsfml-window -lsfml-system
bin\AgenteVirtualFurby.exe
```

Si Windows informa que faltan DLL, agrega `C:\msys64\mingw64\bin` al `PATH` o copia junto al `.exe` las DLL de SFML (`sfml-graphics-2.dll`, `sfml-window-2.dll`, `sfml-system-2.dll`) y las dependencias de MinGW64.

## Compilar con CMake

Se necesita SFML 2.5 o superior instalado y disponible para CMake.

```powershell
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build
.\bin\AgenteVirtualFurby.exe
```

## Controles

Al iniciar aparece el huevo y no hay menú de controles. Haz clic tres veces sobre el huevo para que nazca. Después puedes usar los botones de acciones o sus teclas: `C` come, `B` lo bana, `P` lo ensucia, `D` lo duerme, `W` lo despierta y `R` lo cura. `E` lo enferma y `K` lo mata para probar esos estados.

`HUEVO` solo aparece al iniciar. Haz clic cinco veces sobre él para que nazca; cada clic muestra una etapa distinta y después ya no se puede volver a seleccionar ese estado. Las teclas `1` a `5` y los botones permiten probar `NORMAL`, `HAMBRE`, `SUCIO`, `ENFERMO` y `CON SUENO`. `D` ejecuta la acción de dormir y `W` la interrumpe. `K` o `6` sirven para probar la muerte. Al morir, `V` o el botón `REVIVIR` recuperan al Furby con valores al 70%.

Cada necesidad tiene su propio ritmo: despierto, hambre baja cada 8 segundos, sueño cada 10 segundos e higiene cada 14 segundos. Al usar `DORMIR`, el sueño conserva su valor actual y se recupera continuamente cada 5 segundos; no se detiene ni despierta automáticamente. Mientras duerme, hambre e higiene siguen bajando, pero más lentamente. Solo `DESPERTAR` termina el descanso. Cuando una barra baja de 50%, el Furby muestra automáticamente `CON SUENO`. La salud pierde más puntos cuanto mayor es el déficit combinado de hambre, sueño e higiene; si llega a cero, el Furby muere.

Al forzar un estado con un botón, las barras también se ajustan para mostrar una situación coherente: `NORMAL` coloca todo al 100%; `HAMBRE`, `SUCIO`, `ENFERMO` y `DORMIDO` reducen respectivamente la barra afectada; `MUERTO` coloca todas las barras en 0%.
