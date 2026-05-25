# SpraySheetCut

Recortador manual de spritesheets imperfectos. Ligero, en C++ con [raylib](https://www.raylib.com/).

Cargás una imagen, dibujás rectángulos sobre los sprites que querés, le das guardar y obtenés cada sprite como PNG numerado. Podés guardar el layout de recortes como `.cuts` para reutilizarlo en otras imágenes con la misma disposición.

## Por qué existe

Los spritesheets generados por IA, exportados de animaciones viejas o sacados de packs random rara vez quedan en una grilla perfecta. Los recortadores automáticos fallan. Esto es manual a propósito: tú decidís dónde está cada sprite.

## Características

- Carga PNG / JPG por argumento o arrastrando a la ventana.
- Dibujá rectángulos arrastrando el ratón. Sin grilla, sin tamaño fijo.
- Zoom con la rueda, paneo con botón medio o `Espacio + click izquierdo`.
- Seleccioná un recorte y borralo con `Supr`. Deshacer con `Z`. Limpiar todos con `C`.
- Guardá los recortes como PNGs numerados (`sprite_001.png`, `sprite_002.png`, ...) en una carpeta `output/` junto a la imagen original.
- Exportá la disposición de recortes a un archivo `.cuts` y arrastralo sobre otra imagen para reutilizarlo.
- Si abrís una imagen y al lado hay un `<imagen>.cuts`, se aplica automáticamente.

## Requisitos

- Linux (probado en Linux Mint / Ubuntu).
- `g++`, `make`, `git`, `cmake`.
- raylib 5.x (el script `install_raylib.sh` lo instala por vos).

## Instalación

```bash
git clone https://github.com/<tu-usuario>/spraysheetcut.git
cd spraysheetcut
./install_raylib.sh   # solo la primera vez. Requiere sudo.
./build.sh
```

Esto deja un binario `./spraysheetcut` en la carpeta.

## Uso

```bash
./spraysheetcut                       # abre vacío, arrastrá una imagen
./spraysheetcut spritesheet.png       # abre con imagen cargada
```

### Controles

| Tecla / acción | Qué hace |
|---|---|
| Arrastrar imagen a ventana | Cargar imagen |
| Arrastrar `.cuts` a ventana | Aplicar layout guardado |
| Click izq + arrastrar | Crear recorte rectangular |
| Click izq sobre recorte | Seleccionar (queda amarillo) |
| Rueda del ratón | Zoom (centrado en el cursor) |
| Botón medio del ratón | Paneo |
| `Espacio` + click izq | Paneo alternativo |
| `R` | Resetear vista (zoom 1x, centrado) |
| `Z` | Deshacer último recorte |
| `Supr` | Borrar recorte seleccionado |
| `C` | Borrar todos los recortes |
| `E` | Exportar layout como `<imagen>.cuts` |
| `S` | Guardar todos los recortes como PNG |

### Salida

Al pulsar `S`, los recortes se exportan a una carpeta `output/` **junto a la imagen cargada**:

```
mi_carpeta/
├── spritesheet.png
└── output/
    ├── sprite_001.png
    ├── sprite_002.png
    └── ...
```

La numeración sigue el orden en que dibujaste los rectángulos.

## Formato `.cuts`

Texto plano editable a mano. Cada línea es un rectángulo en píxeles de la imagen original:

```
# spraysheetcut layout v1
12 34 64 64
80 34 64 64
148 34 64 64
```

Columnas: `x y ancho alto`. Comentarios con `#`.

## Flujo recomendado para reciclar layouts

1. Cargá `personaje_run.png`.
2. Marcá los frames con el ratón.
3. `E` → se crea `personaje_run.cuts`.
4. Cargá `personaje_idle.png` (mismo grid).
5. Arrastrá `personaje_run.cuts` a la ventana → mismos recortes aplicados.
6. `S` → exporta los PNGs.

## Limitaciones conocidas

- No hay redimensión / mover recortes ya creados. Si te equivocaste, borrá y volvé a hacerlo.
- No hay selector de archivos: se usa arrastrar y soltar o argumento de línea de comandos.
- Probado solo en Linux. Debería compilar en otros sistemas con raylib pero no está testeado.

## Licencia

MIT.
