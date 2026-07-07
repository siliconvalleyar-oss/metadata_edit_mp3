# TODO - Escalar el Proyecto

## Prioridad Alta

### Rama player - Siguiente pasos
- [ ] Barra de volumen (QSlider horizontal con icono de altavoz)
- [ ] Modo shuffle (reproduccion aleatoria)
- [ ] Modo repeat (repetir tema / repetir lista)
- [ ] Mostrar nombre del tema reproduciendose en el player bar
- [ ] Atajos de teclado: Space=play/pause, Left/Right=seek, Up/Down=next/prev
- [ ] QueuedConnection para signals del player (evitar lag en UI)

### Rama main - Mejoras editor
- [ ] Undo/Redo con QUndoStack (historial de ediciones en memoria)
- [ ] Drag & drop de archivos MP3 a la tabla
- [ ] Exportar lista de metadatos a CSV
- [ ] Buscar/filtrar archivos por nombre o artista (QLineEdit + QSortFilterProxyModel)

## Prioridad Media

### S跨平台
- [ ] Soporte Windows (probar compilacion con MSVC/MinGW)
- [ ] Soporte macOS (probar con homebrew Qt5)
- [ ] CMakeLists.txt como alternativa al Makefile (mejor IDE support)

### Rendimiento
- [ ] Carga lazy de album art (cargar imagen solo cuando se selecciona la fila)
- [ ] Hilos para guardado (evitar blockeo del UI con muchos archivos)
- [ ] QThumbnail para vista previa rapida de imagenes grandes

### Metadatos
- [ ] Soporte ID3v2.4 (actualmente solo v2.3)
- [ ] Soporte para mas frames: TSOP (orden de album), TPOS (disco), USLT (letras)
- [ ] Editor de genero con lista predefinida (ID3 genre numbers)
- [ ] Campo "BPM" (TBPM frame)

## Prioridad Baja

### UI/UX
- [ ] Tema oscuro (dark mode) con QSS
- [ ] Iconos personalizados en vez de Unicode (usar FontAwesome o SVG)
- [ ] Tooltip con caratula de album al hover sobre la tabla
- [ ] Barra de progreso de carga de archivos (para carpetas grandes)
- [ ] Guardar/restaurar estado de la ventana (tamanio, columnas, etc.)
- [ ] Dialogo "About" con info de version y licencia

### Testing
- [ ] Tests unitarios para ID3Tag (leer/escribir/roundtrip)
- [ ] Tests unitarios para MP3File (load/save/discard)
- [ ] Tests de integracion para MainWindow
- [ ] Fuzzing con archivos MP3 corruptos

### Empaquetado
- [ ] Script de instalacion (make install)
- [ ] Paquete .deb
- [ ] Paquete AppImage
- [ ] Icono .desktop para lanzadores de Linux
- [ ] Metadata MIME type para asociar .mp3

## ideas Futuras

- [ ] Soporte OGG/Vorbis (vorbis comments)
- [ ] Soporte FLAC (Vorbis comments)
- [ ] Soporte M4A/AAC (MP4 tags)
- [ ] Edicion de letras embebidas (USLT/SYLT)
- [ ] Conversion de formatos (MP3 → OGG, etc.)
- [ ] Plugin system para formatos adicionales
- [ ] Modo consola/CLI para edicion batch sin UI
