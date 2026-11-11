# BK-Tree sobre el diccionario del español

Implementa y evalúa un BK-Tree (Burkhard y Keller, 1973) con distancia de Levenshtein sobre palabras del español, y estudia cómo la distribución de distancias y la estrategia de construcción afectan el porcentaje de nodos examinados.


## Estructura

```
config.json          parámetros de preparación de datos (normalización, subconjuntos, semilla)
data/raw/            datos originales (CORPES, frecuencias)
data/processed/      diccionario limpio y subconjuntos (palabra,frecuencia)
docs/informe/        propuesta de investigación (LaTeX)
src/scripts/         preparación de datos (Python)
src/core/            UTF-8, normalización, Levenshtein, tipos comunes
src/baseline/        búsqueda exhaustiva (línea base y oráculo de correctitud)
src/bktree/          BK-Tree: inserción, radio, clase 1 y clase 2
tests/               pruebas unitarias y de integración
```

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

## Operaciones de consulta

- **Radio:** todas las palabras con distancia <= r.
- **Clase 1:** una palabra a distancia mínima.
- **Clase 2:** todas las palabras a distancia mínima.

Desempate (igual en BK-Tree y búsqueda exhaustiva): menor distancia, luego mayor frecuencia, luego orden alfabético. Cada consulta puede devolver contadores de distancias calculadas y nodos examinados (`EstadisticasBusqueda`).
