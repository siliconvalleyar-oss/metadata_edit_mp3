# MetadataMP3

Editor grafico de metadatos ID3 para archivos MP3 con reproductor integrado. Construido con Qt5 y C++17.

Permite visualizar, editar y guardar metadatos (artista, album, titulo, anio, genero, pista, comentario, compositor) y caratula de album de multiples archivos MP3. Incluye reproductor de audio con controles play/pause, next/prev y barra de seek.

## Caracteristicas

### Editor de metadatos
- Carga individual de archivos o carpetas completas (recursivo)
- Vista de tabla con 8 columnas: nombre, artista, album, titulo, anio, pista, genero, tamano
- Panel de detalle con campos editables para todos los metadatos
- Visualizacion y eliminacion de caratula de album
- Renombrado de archivos desde el panel de detalle
- Edicion directa en celdas de la tabla
- Seleccion por filas (highlight azul) con Ctrl+click y Shift+click
- Boton "Select All" / "Deselect All" toggle
- Edicion batch: solo modifica los campos que realmente tocas
- Soporte para ID3v1, ID3v1.1 e ID3v2.3
- Codificacion de texto: ISO-8859-1, UTF-16, UTF-8
- Guardado de imagenes APIC (JPEG/PNG)
- Ordenamiento por columnas (alfabetico para texto, numerico para anio/pista/tamano)

### Reproductor MP3
- Play / Pause toggle
- Next / Previous para navegar la lista
- Barra de seek (slider) con posicion actual / duracion
- Doble-click en tabla para reproducir
- Auto-seleccion de fila en reproduccion

## Requisitos

- Linux (tested on Ubuntu/Debian)
- g++ con soporte C++17
- Qt5 development libraries (Core, Gui, Widgets, Multimedia)
- MOC (Qt Meta Object Compiler)
- Plugins multimedia de Qt5

```bash
# Debian/Ubuntu
sudo apt install g++ make qtbase5-dev qt5-qmake qtmultimedia5-dev libqt5multimedia5-plugins
```

## Compilacion

```bash
make          # Compilar
make run      # Compilar y ejecutar
make clean    # Limpiar objetos y binario
make distclean # Limpiar todo (obj + bin)
```

## Estructura del proyecto

```
.
├── bin/                    # Binarios compilados
├── docs/                   # Documentacion
│   ├── API.md
│   ├── ARCHITECTURE.md
│   ├── BUILD.md
│   ├── CHANGELOG.md
│   └── TODO.md
├── include/                # Headers (.h)
│   ├── id3tag.h
│   ├── mainwindow.h
│   ├── mp3file.h
│   ├── mp3player.h
│   └── mp3tablemodel.h
├── obj/                    # Objetos compilados
├── src/                    # Fuentes (.cpp)
│   ├── id3tag.cpp
│   ├── main.cpp
│   ├── mainwindow.cpp
│   ├── mp3file.cpp
│   ├── mp3player.cpp
│   └── mp3tablemodel.cpp
├── Makefile
├── README.md
└── VERSION
```

## Ramas

| Rama | Descripcion | Tags |
|------|-------------|------|
| `main` | Editor de metadatos | v1.0.0 - v1.4.1 |
| `player` | Editor + reproductor MP3 | v2.0.0 - v2.0.1 |

## Uso

1. Ejecutar `./bin/MetadataMP3`
2. Clic en **Add Files** o **Add Folder** para cargar archivos MP3
3. Seleccionar archivo(s) en la tabla (click, Ctrl+click, Shift+click)
4. Modificar campos en el panel de detalle
5. Para renombrar, escribir el nombre en "New Filename"
6. Para eliminar caratula, clic en **Remove Art** (aplica a seleccionados)
7. Clic en **Apply Changes** para guardar
8. Para reproducir: doble-click en la tabla o seleccionar + boton Play

## Licencia

Ver archivo LICENSE (si existe).
