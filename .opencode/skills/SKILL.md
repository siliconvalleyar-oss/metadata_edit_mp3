# MetadataMP3 - Skill

## Contexto del Proyecto

MetadataMP3 es un editor grafico de metadatos ID3 para archivos MP3 con reproductor integrado. Qt5/C++17. Makefile build system.

## Ramas

- `main` — Editor de metadatos (v1.4.1)
- `player` — Editor + reproductor MP3 (v2.0.1)

## Arquitectura

```
src/main.cpp              → Entry point
src/mainwindow.cpp        → UI principal (toolbar, player bar, splitter, tabla, panel detalle)
src/mp3tablemodel.cpp     → QAbstractTableModel (8 columnas, sort)
src/mp3file.cpp           → Modelo de dominio (metadata + backup + rename)
src/id3tag.cpp            → Parser/escritor ID3v1/v2 (binario)
src/mp3player.cpp         → Wrapper QMediaPlayer (play/pause/next/prev/seek)
```

## Convenciones

- Headers en `include/`, sources en `src/`
- MOC_HDR en Makefile: `mainwindow.h mp3tablemodel.h mp3player.h`
- Nueva clase con Q_OBJECT: agregar header a MOC_HDR
- Tags git en cada push: bump VERSION, commit, `git tag -a vX.Y.Z`, push with --tags
- Sin QCheckBox para seleccion — usar QItemSelectionModel (highlight azul)
- Batch edit: cada campo tracking independiente via m_editedFields

## Dependencias

```bash
sudo apt install g++ make qtbase5-dev qt5-qmake qtmultimedia5-dev libqt5multimedia5-plugins
```

## Build

```bash
make clean && make    # Compilar
make run              # Compilar + ejecutar
```

## Bugs Conocidos / Cuidados

- `editingFinished` solo dispara en Enter/loss-of-focus → usar commitPendingEdits() antes de save
- ID3v1 genre es un solo byte (0-255) — no hay lookup de nombres
- Audio data que termine en "TAG" seria confundido con ID3v1 (riesgo teorico)
- `commitPendingEdits()` solo aplica campos en m_editedFields (diseñado para batch selectivo)
