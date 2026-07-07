# Changelog

## [2.0.1] - 2026-07-06 (rama player)

### Fixed
- Remove Art ahora aplica a todos los archivos seleccionados, no solo al actual

## [2.0.0] - 2026-07-06 (rama player)

### Added
- Reproductor MP3 integrado via QMediaPlayer
- Player bar: prev, play/pause, next, seek slider, time label
- Doble-click en tabla para reproducir archivo
- Navegacion previous/next por la lista de temas
- Seek slider con posicion actual / duracion (MM:SS)
- Auto-seleccion de fila en reproduccion
- Nueva dependencia: Qt5Multimedia

## [1.4.1] - 2026-07-06

### Fixed
- Boton "Remove Selected" eliminado del toolbar

## [1.4.0] - 2026-07-06

### Added
- Ordenamiento por columnas (click en headers)
- Filename/Artist/Album/Title/Genre: orden alfabetico
- Year/Track/Size: orden numerico
- Ascending/descending toggle via flechas

## [1.3.0] - 2026-07-06

### Fixed
- Batch edit ahora solo modifica los campos realmente tocados
- Antes: editar Album sobreescribia Artist/Title/etc en todos los archivos
- Ahora: cada campo tiene tracking independiente via m_editedFields

## [1.2.1] - 2026-07-06

### Fixed
- "Clear All" ya no pide confirmacion

## [1.2.0] - 2026-07-06

### Changed
- Seleccion por filas (highlight azul) en vez de checkboxes
- Boton toggle "Select All" / "Deselect All" en toolbar
- Eliminada columna de checkbox de la tabla
- Eliminado m_selected/selected()/setSelected() de MP3File
- Seleccion via QItemSelectionModel (Ctrl+click, Shift+click, drag)

## [1.1.1] - 2026-07-06

### Fixed
- Tags ID3v1 duplicados al guardar (consolidado save en funcion unica)
- "Apply Changes" no escribia metadatos (commitPendingEdits antes de save)
- UI se colgaba al guardar (QProgressDialog + processEvents)
- make clean ahora tambien elimina el binario

## [1.1.0] - 2026-07-06

### Added
- Boton "Discard Changes" para revertir modificaciones
- Edicion batch: campos del panel aplican a todos los archivos seleccionados

### Fixed
- Propagacion de ediciones del panel de detalle a archivos seleccionados

## [1.0.0] - 2026-07-02

### Added
- Editor grafico de metadatos ID3 para archivos MP3
- Carga de archivos individuales y carpetas (recursivo)
- Vista de tabla con seleccion multiple y ordenamiento
- Panel de detalle con edicion de todos los campos ID3
- Soporte ID3v1, ID3v1.1 e ID3v2.3
- Decodificacion de texto ISO-8859-1, UTF-16, UTF-8
- Visualizacion y eliminacion de caratula de album (APIC frame)
- Renombrado de archivos desde el editor
- Edicion in-place en celdas de la tabla
- Checkbox de seleccion por archivo con "Select All"
- Indicador visual de archivos modificados (texto azul)
- Barra de estado con conteo de archivos, seleccionados y modificados
- Makefile con estructura organizada (src/, include/, obj/, bin/)
