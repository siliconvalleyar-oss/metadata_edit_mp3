# API Reference

## MP3File

Modelo de dominio que representa un archivo MP3 con sus metadatos.

### Constructor

```cpp
MP3File();
explicit MP3File(const QString &filePath);
```

### Metodos principales

| Metodo | Retorno | Descripcion |
|--------|---------|-------------|
| `load()` | `bool` | Lee metadatos del disco via ID3Tag + crea backup |
| `save()` | `bool` | Escribe metadatos al disco + renombra si aplica |
| `discardChanges()` | `void` | Restaura valores originales del backup |

### Getters (inline)

| Metodo | Retorno | Campo ID3 |
|--------|---------|-----------|
| `filePath()` | `QString` | Ruta completa |
| `fileName()` | `QString` | Nombre del archivo |
| `fileSize()` | `qint64` | Tamano en bytes |
| `lastModified()` | `QDateTime` | Fecha de modificacion |
| `title()` | `QString` | TIT2 |
| `artist()` | `QString` | TPE1 |
| `album()` | `QString` | TALB |
| `year()` | `QString` | TDRC/TYER |
| `genre()` | `QString` | TCON |
| `track()` | `QString` | TRCK |
| `comment()` | `QString` | COMM |
| `composer()` | `QString` | TCOM |
| `albumArt()` | `QByteArray` | APIC (imagen raw) |
| `hasAlbumArt()` | `bool` | `!albumArt().isEmpty()` |
| `isModified()` | `bool` | Flag de modificacion |
| `newFileName()` | `QString` | Nombre para renombrado |
| `removeArtFlag()` | `bool` | True si se solicito eliminar imagen |

### Setters

Cada setter marca `m_modified = true`:

```cpp
void setArtist(const QString &v);
void setAlbum(const QString &v);
void setTitle(const QString &v);
void setYear(const QString &v);
void setGenre(const QString &v);
void setTrack(const QString &v);
void setComment(const QString &v);
void setComposer(const QString &v);
void setAlbumArt(const QByteArray &v);
void removeAlbumArt();              // limpia imagen + set m_removeArt=true
void setNewFileName(const QString &v);  // m_renameFile = !v.isEmpty()
void setModified(bool v);
```

---

## MP3TableModel

Modelo Qt para `QAbstractTableModel`. 8 columnas, sin checkbox.

### Enum Column

```cpp
enum Column {
    ColFileName = 0,
    ColArtist, ColAlbum, ColTitle,
    ColYear, ColTrack, ColGenre, ColSize,
    ColumnCount  // = 8
};
```

### Metodos principales

| Metodo | Retorno | Descripcion |
|--------|---------|-------------|
| `addFile(MP3File*)` | `void` | Agrega archivo al modelo |
| `removeFile(int row)` | `void` | Elimina archivo en la fila |
| `clear()` | `void` | Elimina todos los archivos |
| `fileAt(int row)` | `MP3File*` | Obtiene archivo en fila |
| `allFiles()` | `QVector<MP3File*>` | Todos los archivos |
| `sort(int column, Qt::SortOrder)` | `void` | Ordena por columna |

### Overrides de QAbstractTableModel

- `rowCount()` / `columnCount()` - dimensiones
- `data()` - datos para DisplayRole, EditRole, ForegroundRole, ToolTipRole
- `headerData()` - encabezados de columna
- `setData()` - edicion in-place
- `flags()` - habilita editable (excepto Size)
- `sort()` - ordenamiento alfabetico/numerico por columna

---

## ID3Tag

Capa de lectura/escritura de tags ID3 en archivos MP3.

### Metodos principales

| Metodo | Retorno | Descripcion |
|--------|---------|-------------|
| `load(const QString&)` | `bool` | Lee ID3v1 y ID3v2 del archivo |
| `save(const QString&)` | `bool` | Reescribe archivo: [v2] + [audio] + [v1] |

### Frames ID3v2 soportados

| Frame | Campo |
|-------|-------|
| TPE1 | Artista |
| TALB | Album |
| TIT2 | Titulo |
| TDRC / TYER | Anio |
| TCON | Genero |
| TRCK | Pista |
| TCOM | Compositor |
| COMM | Comentario |
| APIC | Caratula de album |

### Codificaciones soportadas

- `0x00`: ISO-8859-1
- `0x01`: UTF-16 con BOM
- `0x02`: UTF-16BE
- `0x03`: UTF-8

---

## Mp3Player

Reproductor de audio basado en QMediaPlayer.

### Metodos principales

| Metodo | Retorno | Descripcion |
|--------|---------|-------------|
| `setFiles(QVector<MP3File*>)` | `void` | Actualiza la playlist |
| `playFile(MP3File*)` | `void` | Reproduce un archivo especifico |
| `play()` | `void` | Play / reanuda |
| `pause()` | `void` | Pausa |
| `stop()` | `void` | Detiene |
| `next()` | `void` | Siguiente tema |
| `previous()` | `void` | Tema anterior |
| `setPosition(qint64 ms)` | `void` | Seek a posicion |
| `state()` | `QMediaPlayer::State` | Estado actual |
| `position()` | `qint64` | Posicion en ms |
| `duration()` | `qint64` | Duracion en ms |
| `currentFile()` | `MP3File*` | Archivo reproduciendose |

### Senales

```cpp
void stateChanged(QMediaPlayer::State state);
void positionChanged(qint64 position);
void durationChanged(qint64 duration);
void currentFileChanged(MP3File *file);
```

---

## MainWindow

Ventana principal. Slots principales:

| Slot | Descripcion |
|------|-------------|
| `onAddFiles()` | Abre dialogo para seleccionar archivos MP3 |
| `onAddFolder()` | Abre dialogo para seleccionar carpeta |
| `onClearAll()` | Limpia todos los archivos (sin confirmacion) |
| `onApply()` | Guarda cambios de archivos seleccionados con progress bar |
| `onDiscardChanges()` | Revertir cambios a valores originales |
| `onToggleSelectAll()` | Toggle select all / deselect all |
| `onPlayPause()` | Play / Pause del reproductor |
| `onNext()` / `onPrevious()` | Navegar playlist |
| `onRemoveArt()` | Eliminar caratula de archivos seleccionados |
| `commitPendingEdits()` | Aplica solo campos modificados (m_editedFields) |
