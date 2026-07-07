# Arquitectura del Proyecto

## Vision general

MetadataMP3 es una aplicacion Qt5 de escritorio que permite leer y modificar metadatos ID3 en archivos MP3, con reproductor de audio integrado. Sigue el patron MVC de Qt.

## Capas

```
┌─────────────────────────────────────────┐
│              main.cpp                   │  Entrada: crea QApplication y MainWindow
├─────────────────────────────────────────┤
│           MainWindow (Vista)            │  UI: tabla + panel detalle + toolbar + player
│  ┌──────────────┬───────────────────┐   │
│  │  QTableView  │  Panel Derecho    │   │
│  │  (tabla MP3) │  (edicion campos) │   │
│  └──────────────┴───────────────────┘   │
│  ┌──────────────────────────────────┐   │
│  │  Player Bar (prev/play/next/seek)│   │
│  └──────────────────────────────────┘   │
├─────────────────────────────────────────┤
│        MP3TableModel (Modelo)           │  Modelo Qt: puente entre vista y datos
├─────────────────────────────────────────┤
│           MP3File (Dominio)             │  Modelo de negocio: metadata de un archivo
├─────────────────────────────────────────┤
│           ID3Tag (Persistencia)         │  Lectura/escritura de tags ID3v1/v2
├─────────────────────────────────────────┤
│           Mp3Player (Audio)             │  Reproduccion via QMediaPlayer
└─────────────────────────────────────────┘
```

## Clases

### MainWindow (`include/mainwindow.h`, `src/mainwindow.cpp`)

Ventana principal. Responsabilidades:
- Construccion de la interfaz (toolbar, player bar, splitter, tabla, panel de detalle)
- Coordinacion entre vista, modelo y reproductor
- Seleccion de archivos y carpetas via `QFileDialog`
- Aplicacion de cambios (guardar en disco) con barra de progreso
- Edicion batch por campos (solo modifica campos tocados)
- Navegacion del reproductor (play/pause, next, prev, seek)

### MP3TableModel (`include/mp3tablemodel.h`, `src/mp3tablemodel.cpp`)

Modelo de tabla Qt. Responsabilidades:
- Almacenar la lista de objetos `MP3File*`
- Exponer datos al `QTableView` via `data()` / `headerData()`
- Soporte de edicion in-place via `setData()`
- Ordenamiento por columnas via `sort()`

Columnas (8):
| Indice | Nombre  | Editable | Orden    |
|--------|---------|----------|----------|
| 0      | Filename| Si       | Alfabetico|
| 1      | Artist  | Si       | Alfabetico|
| 2      | Album   | Si       | Alfabetico|
| 3      | Title   | Si       | Alfabetico|
| 4      | Year    | Si       | Numerico  |
| 5      | Track   | Si       | Numerico  |
| 6      | Genre   | Si       | Alfabetico|
| 7      | Size    | No       | Numerico  |

### MP3File (`include/mp3file.h`, `src/mp3file.cpp`)

Modelo de dominio. Responsabilidades:
- Contener los metadatos de un archivo MP3
- Coordina lectura/escritura via `ID3Tag`
- Gestion de estado (modificado, renombrado)
- Backup de valores originales para discard
- Renombrado de archivo en disco

### ID3Tag (`include/id3tag.h`, `src/id3tag.cpp`)

Capa de persistencia. Responsabilidades:
- Lectura de ID3v1 (128 bytes al final del archivo)
- Lectura de ID3v2 (cabecera al inicio, frames parseados)
- Escritura unificada: `save()` lee archivo completo, elimina tags viejos, escribe [v2] + [audio] + [v1]
- Parseo de frames: TPE1, TALB, TIT2, TDRC, TCON, TRCK, TCOM, COMM, APIC
- Decodificacion de texto: ISO-8859-1, UTF-16 (LE/BE con BOM), UTF-8
- Sinc-safe integer conversion para ID3v2.4+

### Mp3Player (`include/mp3player.h`, `src/mp3player.cpp`)

Reproductor de audio. Responsabilidades:
- Wrap de `QMediaPlayer` con interfaz simplificada
- Gestion de playlist (lista de MP3File*)
- Play, pause, stop, next, previous
- Seek (setPosition)
- Senales: stateChanged, positionChanged, durationChanged, currentFileChanged

## Flujo de datos

### Carga de archivo
```
MainWindow::loadFile()
  → new MP3File(path)
    → MP3File::load()
      → ID3Tag::load(path)
        → readID3v1()     // lee 128 bytes al final
        → readID3v2()     // lee cabecera + frames al inicio
          → parseID3v2Frames()
            → decodeText() para cada frame
      ← rellena campos de MP3File + backup originales
  → MP3TableModel::addFile()
  → refreshPlayerPlaylist()
```

### Guardado de cambios
```
MainWindow::onApply()
  → commitPendingEdits()    // solo campos modificados (m_editedFields)
  → QProgressDialog         // barra de progreso modal
  → MP3File::save() por cada archivo seleccionado
    → ID3Tag::save(path)
      → readAll() + strip old tags
      → buildID3v2Tag()     // reconstruye tag v2 (sin APIC si removeArt)
      → write [v2] + [audio] + [v1]
    → QFile::rename() si hay renombrado
  → refreshPlayerPlaylist()
```

### Reproduccion
```
Doble-click en tabla / boton Play
  → Mp3Player::playFile(MP3File*)
    → QMediaPlayer::setMedia(QUrl::fromLocalFile(path))
    → QMediaPlayer::play()
  → positionChanged → update slider + time label
  → currentFileChanged → selectRow() en tabla
```

## Dependencias externas

- **Qt5Core**: contenedores, archivos, codecs
- **Qt5Gui**: QImage para caratula de album
- **Qt5Widgets**: toda la interfaz grafica
- **Qt5Multimedia**: QMediaPlayer para reproduccion de audio
- **MOC**: compilador de meta-objetos para signals/slots
