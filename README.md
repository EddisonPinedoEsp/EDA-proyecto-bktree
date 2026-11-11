# BK-Tree sobre el diccionario del español

Implementa y evalúa un BK-Tree (Burkhard y Keller, 1973) con distancia de Levenshtein sobre palabras del español, y estudia cómo la distribución de distancias y la estrategia de construcción afectan el porcentaje de nodos examinados.


## Sobre el proyecto

Un **BK-Tree** es un árbol para buscar palabras "parecidas" (por ejemplo, para corregir ortografía). Cada nodo guarda una palabra y sus hijos cuelgan de aristas etiquetadas con la distancia de Levenshtein (mínimo de inserciones, eliminaciones y sustituciones para pasar de una palabra a otra). Al buscar, la desigualdad triangular permite descartar ramas enteras sin visitarlas. Qué tanto se poda depende de cómo estén repartidas las distancias entre las palabras.

El proyecto responde a la pregunta: **¿cómo se relacionan la distribución de distancias de Levenshtein y la estrategia de construcción de un BK-Tree con el porcentaje de nodos examinados en las consultas?** Para ello:

1. Implementa la distancia de Levenshtein, una búsqueda exhaustiva (línea base y oráculo de correctitud) y un BK-Tree convencional, con tres consultas: por radio, clase 1 (una palabra a distancia mínima) y clase 2 (todas las de distancia mínima).
2. Caracteriza las distancias y la estructura de los árboles sobre un diccionario del español.
3. Propone y compara una estrategia alternativa de construcción mediante selección de pivotes.


## Estructura

```
config.json          parámetros de preparación de datos (normalización, subconjuntos, semilla)
data/raw/            datos originales (CORPES, frecuencias)
data/processed/      diccionario limpio y subconjuntos (palabra,frecuencia)
docs/informe/        propuesta de investigación (LaTeX)
src/scripts/         preparación de datos (Python)
src/core/            UTF-8, normalización, Levenshtein, tipos comunes
src/baseline/        búsqueda exhaustiva (línea base y oráculo de correctitud)
src/bktree/          BK-Tree: inserción, radio, clase 1 y clase 2, métricas estructurales
src/builders/        órdenes de inserción (original, aleatorio con semilla, alfabético)
src/experiments/     ejecutable de caracterización (Fase 4)
tests/               pruebas unitarias y de integración
```

## Datos

### Fuentes (`data/raw/`)

| Archivo | Contenido | 
|---|---|
| `frecuencia_elementos_corpes_1_5.txt` | Lista de frecuencias de elementos del CORPES (corpus de la RAE), versión 1.5: ~3,1 millones de líneas tabulares (elemento, lema, etiqueta gramatical, frecuencia absoluta, frecuencias normalizadas). Incluye signos de puntuación y números. |
| `palabras_frecuencias.csv` | Lista `palabra,frecuencia` de ~50 000 filas. | 

### Procesamiento (`src/scripts/prepare_data.py`)

La política está en `config.json` y se aplica una sola vez, de modo que el árbol, las consultas y la búsqueda exhaustiva usan exactamente el mismo conjunto:

1. **Lectura:** de cada fuente se toma la palabra y su frecuencia absoluta. Las filas sin frecuencia válida se descartan.
2. **Normalización:** Unicode NFC y minúsculas. **Se conservan las tildes** (`a` y `á` son letras distintas) y la `ñ`; no se eliminan acentos.
3. **Validación:** solo se aceptan palabras formadas por letras del español (`a-z`, `á é í ó ú ü ñ`), de 1 a 30 caracteres. Se descartan signos de puntuación, números y símbolos.
4. **Deduplicación:** tras normalizar, las entradas repetidas (otras mayúsculas, distintas etiquetas gramaticales, aparición en ambas fuentes) se fusionan en una sola palabra y **sus frecuencias se suman**.
5. **Orden y salida:** el resultado se ordena por frecuencia descendente y se guarda en `data/processed/`.

Resultado de la última ejecución (`data/processed/stats_summary.json`):

| Entradas leídas | Descartadas por inválidas | Duplicados fusionados | Palabras únicas finales |
|---|---|---|---|
| 3 164 069 | 1 453 215 | 601 504 | **1 109 350** |

Las palabras tienen entre 1 y 30 letras; la longitud más común es 8 (152 962 palabras) y las de 7 a 9 letras son cerca del 40 % del total.

### Archivos procesados (`data/processed/`)

- `dictionary_full.csv`: el diccionario completo (`palabra,frecuencia`, con encabezado).
- `subsets/dict_N.csv` para N = 1 000, 5 000, 10 000, 50 000, 100 000, 500 000 y 1 000 000: las **N palabras más frecuentes**. Son subconjuntos **anidados** (cada uno contiene al anterior), pensados para medir cómo crece el costo con el tamaño del diccionario.
- `stats_summary.json`: estadísticas de la preparación.

Consecuencia a tener en cuenta: como los subconjuntos se forman por frecuencia, los pequeños están formados por palabras cortas y muy usadas (`de`, `la`, `que`...), y las distancias entre ellas no son representativas del diccionario completo. El cargador de C++ (`src/core/dictionary.cpp`) lee estos CSV aplicando de nuevo la misma normalización (es idempotente).

## Requisitos

- Compilador C++17 (g++ o clang++) y CMake >= 3.14
- Python 3 (solo para preparar los datos)

## Compilar y probar

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Preparar los datos

```bash
python3 src/scripts/prepare_data.py
```

Genera `data/processed/dictionary_full.csv` y los subconjuntos en `data/processed/subsets/` según `config.json`.

## Caracterización

### Qué es

La **caracterización** es la descripción empírica, antes de proponer ninguna mejora, de dos cosas: (1) cómo se distribuyen las distancias de Levenshtein entre las palabras del diccionario y (2) qué forma tienen los BK-Trees que resultan de construirlo (profundidad, número de hijos por nodo, tamaño de los subárboles, reparto de las etiquetas de las aristas). Se mide sobre muestras con semillas fijas, de modo que los resultados son reproducibles.

### Para qué sirve en el proyecto

El BK-Tree poda ramas usando la desigualdad triangular, y esa poda funciona mejor o peor según cómo estén repartidas las distancias. Si casi todos los pares de palabras están a distancias parecidas, pocas ramas se descartan y la búsqueda se acerca a revisar todo el diccionario. La caracterización es la base de la pregunta de investigación: relaciona la distribución de distancias y la forma del árbol con el porcentaje de nodos examinados en las consultas. Además, fija la línea base (construcción por orden de inserción) con la que se compararán después las estrategias alternativas de selección de pivotes (Fase 5 del plan).

### Cómo ejecutarla

Desde la raíz del repositorio, con el proyecto ya compilado (ver "Compilar y probar") y los datos preparados:

```bash
./build/caracterizacion tamanos=1000,5000,10000 semillas=5 pares=200000 consultas=300 radios=1,2,3 salida=resultados/caracterizacion
```

Todos los parámetros son opcionales y se pasan como `clave=valor`:

| Parámetro | Valor por defecto | Significado |
|---|---|---|
| `dir` | `data/processed/subsets` | Carpeta con los `dict_N.csv` |
| `tamanos` | `1000,5000,10000` | Tamaños de diccionario a evaluar |
| `semillas` | `5` | Semillas (1..K) del orden aleatorio por tamaño |
| `pares` | `200000` | Pares aleatorios para la distribución de distancias |
| `consultas` | `300` | Consultas por tipo (presente, 1 error, 2 errores) |
| `radios` | `1,2,3` | Radios de la búsqueda por radio (se mide además clase 2) |
| `salida` | `resultados/caracterizacion` | Carpeta de salida |
| `verificar` | `1` | Compara contra la búsqueda exhaustiva (primeras 100 consultas de cada tipo) |

Con los valores del ejemplo tarda unos 40 s (casi todo es la verificación). Para un tamaño grande conviene usar `verificar=0`. Las semillas están fijas (muestreo de pares y consultas: 42, orden aleatorio: 1..K), así que repetir el comando da los mismos CSV, salvo la columna de tiempos.

Variantes de construcción por tamaño: orden `original` (frecuencia descendente, el de los archivos), `alfabetico` y `aleatorio` con cada semilla.

### Archivos de salida (`resultados/caracterizacion/`)

| Archivo | Contenido |
|---|---|
| `distancias_pares.csv`, `resumen_distancias.csv` | Histograma y estadísticas (media, mediana, desviación, cuantiles, mín., máx.) de d(a,b) entre pares **aleatorios** |
| `estructura.csv` | Nodos, hojas, profundidad máx. y media, grados, hijos de la raíz, tiempo de construcción |
| `hist_etiquetas.csv` | Distribución de etiquetas de aristas nodo→hijo (distinta de la anterior) |
| `hist_grados.csv` | Distribución del número de hijos por nodo |
| `subarboles_raiz.csv` | Tamaño del subárbol de cada hijo de la raíz |
| `consultas.csv` | Distancias calculadas y % de nodos examinados (media, desviación, mín., máx.) por tipo de consulta y radio; columna `correcto` = coincide con la búsqueda exhaustiva |

### Resultados de la ejecución de ejemplo

La verificación contra la búsqueda exhaustiva fue **OK** en todas las variantes. Todo lo siguiente sale de los CSV de esa corrida.

**Distancia de Levenshtein entre pares aleatorios** (200 000 pares por tamaño, muestreo, no cálculo exhaustivo):

| Tamaño | Media | Mediana | Desv. | p5 | p95 | Mín. | Máx. |
|---|---|---|---|---|---|---|---|
| 1 000 | 5,85 | 6 | 1,73 | 3 | 9 | 1 | 13 |
| 5 000 | 6,95 | 7 | 1,93 | 4 | 10 | 1 | 16 |
| 10 000 | 7,30 | 7 | 1,96 | 4 | 11 | 1 | 18 |

Para 10 000 palabras, el porcentaje de pares por distancia es: d=3: 1,1 %, d=4: 4,5 %, d=5: 11,6 %, d=6: 18,4 %, d=7: 21,4 %, d=8: 18,0 %, d=9: 12,1 %, d=10: 6,7 %; el resto (d=2 y d>=11) suma 6,2 %. Es decir, **las distancias se concentran alrededor de 6–8, no en 1–2**: la hipótesis de la propuesta (agrupamiento en distancias pequeñas) no se observa entre pares aleatorios de estos subconjuntos.

**Estructura del árbol** (media sobre las semillas del orden aleatorio; los órdenes `original` y `alfabetico` tienen una sola construcción):

| Tamaño | Orden | Prof. máx. | Prof. media | Grado máx. | Mayor subárbol de la raíz (% de nodos) |
|---|---|---|---|---|---|
| 1 000 | original | 13 | 4,61 | 12 | 23 % |
| 1 000 | alfabético | 11 | 5,02 | 12 | 21 % |
| 1 000 | aleatorio | 9,4 (9–10) | 4,78 | 11,4 | 36 % |
| 5 000 | original | 21 | 5,60 | 15 | 19 % |
| 5 000 | alfabético | 21 | 6,11 | 16 | 18 % |
| 5 000 | aleatorio | 13,2 (11–16) | 5,67 | 14,2 | 30 % |
| 10 000 | original | 25 | 6,02 | 17 | 19 % |
| 10 000 | alfabético | 25 | 6,58 | 18 | 17 % |
| 10 000 | aleatorio | 16,8 (14–23) | 6,03 | 16,6 | 29 % |

La profundidad máxima depende del orden de inserción (25 con orden original o alfabético, 14–23 con orden aleatorio) pero la profundidad media casi no cambia.

**Costo de consulta** (orden aleatorio, media de las 5 semillas; % de nodos del árbol cuya distancia a la consulta se calcula, 300 consultas por tipo):

| Tamaño | Tipo de consulta | Radio 1 | Radio 2 | Clase 2 |
|---|---|---|---|---|
| 1 000 | presente | 16,1 % | 48,4 % | 31,3 % |
| 1 000 | 1 error | 17,2 % | 50,6 % | 42,0 % |
| 1 000 | 2 errores | 17,4 % | 49,8 % | 55,2 % |
| 10 000 | presente | 6,2 % | 30,5 % | 20,4 % |
| 10 000 | 1 error | 6,1 % | 29,6 % | 28,1 % |
| 10 000 | 2 errores | 6,2 % | 29,5 % | 38,0 % |

Con 10 000 palabras, una consulta de radio 1 calcula unas 620 distancias frente a las 10 000 de la búsqueda exhaustiva; con radio 2 calcula unas 3 000. El porcentaje examinado baja al crecer el diccionario, pero sube con el radio y con el número de errores de la consulta (en clase 2).

**Cautelas para interpretar:** los subconjuntos son anidados por frecuencia (los tamaños pequeños tienen palabras más cortas y frecuentes, lo que influye en la distancia media); las consultas "presente" en clase 2 incluyen la propia palabra a distancia 0; y son resultados de una sola corrida, aún sin los tamaños grandes (50 000 a 1 000 000).

## Operaciones de consulta

- **Radio:** todas las palabras con distancia <= r.
- **Clase 1:** una palabra a distancia mínima.
- **Clase 2:** todas las palabras a distancia mínima.

Desempate (igual en BK-Tree y búsqueda exhaustiva): menor distancia, luego mayor frecuencia, luego orden alfabético. Cada consulta puede devolver contadores de distancias calculadas y nodos examinados (`EstadisticasBusqueda`).
