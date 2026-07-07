# Guia de Compilacion

## Requisitos del sistema

- **Sistema operativo**: Linux (Ubuntu/Debian, Fedora, Arch)
- **Compilador**: g++ con soporte C++17
- **Qt5**: desarrollo (headers + libs + moc + multimedia)

## Instalar dependencias

### Debian/Ubuntu

```bash
sudo apt update
sudo apt install g++ make qtbase5-dev qt5-qmake qtmultimedia5-dev libqt5multimedia5-plugins
```

### Fedora

```bash
sudo dnf install gcc-c++ make qt5-qtbase-devel qt5-qttools-devel qt5-qtmultimedia-devel
```

### Arch Linux

```bash
sudo pacman -S base-devel qt5-base qt5-tools qt5-multimedia
```

## Compilar

```bash
make            # Compilar el binario
make run        # Compilar y ejecutar
make clean      # Eliminar archivos objeto + binario
make distclean  # Eliminar obj/ y bin/
```

## Estructura de archivos generada

```
obj/                    # Archivos objeto y dependencias
├── id3tag.o
├── main.o
├── mainwindow.o
├── mp3file.o
├── mp3player.o
├── mp3tablemodel.o
├── moc_mainwindow.cpp  # Generado por MOC
├── moc_mainwindow.o
├── moc_mp3tablemodel.cpp
├── moc_mp3tablemodel.o
├── moc_mp3player.cpp   # Generado por MOC
├── moc_mp3player.o
├── *.d                 # Dependencias automaticas
bin/
└── MetadataMP3         # Binario ejecutable
```

## Makefile - Variables

| Variable | Valor default | Descripcion |
|----------|---------------|-------------|
| `CXX` | `g++` | Compilador |
| `CXXFLAGS` | `-std=c++17 -fPIC -Wall -Wextra -O2` | Flags de compilacion |
| `LDFLAGS` | `-lQt5Core -lQt5Gui -lQt5Widgets -lQt5Multimedia` | Librerias enlazadas |
| `INCLUDES` | `-Iinclude -I/usr/include/...` | Rutas de headers |
| `MOC_HDR` | `mainwindow.h mp3tablemodel.h mp3player.h` | Headers que requieren MOC |

## Agregar nueva clase con signals/slots

1. Crear `include/miclase.h` con `Q_OBJECT`
2. Crear `src/miclase.cpp`
3. Agregar `miclase.h` a `MOC_HDR` en Makefile
4. Los sources se detectan automaticamente via `wildcard $(SRC)/*.cpp`

## Solucion de problemas

### "moc: command not found"

```bash
which moc
sudo apt install qt5-qmake qtbase5-dev
```

### "Qt5Multimedia not found"

```bash
sudo apt install qtmultimedia5-dev libqt5multimedia5-plugins
```

### Errores de headers movidos

Los headers estan en `include/`, no en `src/`. El Makefile usa `-Iinclude`.
