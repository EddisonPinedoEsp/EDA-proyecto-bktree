import re
import csv
import json
import unicodedata
from pathlib import Path

def clean_word(
    word: str,
    lowercase: bool = True,
    strip_accents: bool = False,
) -> str:
    """Normaliza una palabra según la política configurada."""
    word = unicodedata.normalize("NFC", word.strip())
    if lowercase:
        word = word.lower()
    if strip_accents:
        word = "".join(
            char
            for char in unicodedata.normalize("NFD", word)
            if unicodedata.category(char) != "Mn"
        )
        word = unicodedata.normalize("NFC", word)
    return word

def is_valid_word(
    word: str,
    min_word_length: int = 1,
    max_word_length: int | None = None,
    allow_spanish_chars: bool = True,
) -> bool:
    """Valida que la entrada contenga únicamente caracteres alfabéticos en español."""
    # Permite letras, tildes en español y la ñ. Excluye signos ortográficos (, . : ? ! " - —)
    if len(word) < min_word_length or (
        max_word_length is not None and len(word) > max_word_length
    ):
        return False
    pattern = r"^[a-záéíóúüñ]+$" if allow_spanish_chars else r"^[a-z]+$"
    return bool(re.fullmatch(pattern, word, re.IGNORECASE))

def process_corpes_file(
    filepath: str,
    normalization: dict,
):
    """Procesa el archivo tabular de CORPES 1.5."""
    data = {}
    total_raw = 0
    discarded = 0
    
    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        # Saltar encabezados o líneas vacías
        for line in f:
            parts = line.strip().split()
            if not parts or parts[0] == "Elemento":
                continue
            
            total_raw += 1
            raw_word = parts[0]
            
            # Intentar parsear la frecuencia absoluta (columna 3 o 4 según formato)
            try:
                freq = int(parts[3])
            except (IndexError, ValueError):
                discarded += 1
                continue

            cleaned = clean_word(
                raw_word,
                lowercase=normalization["lowercase"],
                strip_accents=normalization["strip_accents"],
            )
            if is_valid_word(
                cleaned,
                min_word_length=normalization["min_word_length"],
                max_word_length=normalization["max_word_length"],
                allow_spanish_chars=normalization["allow_spanish_chars"],
            ):
                data[cleaned] = data.get(cleaned, 0) + freq
            else:
                discarded += 1

    return data, total_raw, discarded

def process_csv_file(filepath: str, normalization: dict):
    """Procesa el archivo CSV (palabra,frecuencia)."""
    data = {}
    total_raw = 0
    discarded = 0
    
    with open(filepath, 'r', encoding='utf-8') as f:
        reader = csv.reader(f)
        header = next(reader, None)
        for row in reader:
            if not row:
                continue
            total_raw += 1
            if len(row) < 2:
                discarded += 1
                continue
            raw_word, freq_str = row[0], row[1]
            try:
                freq = int(freq_str)
            except ValueError:
                discarded += 1
                continue

            cleaned = clean_word(
                raw_word,
                lowercase=normalization["lowercase"],
                strip_accents=normalization["strip_accents"],
            )
            if is_valid_word(
                cleaned,
                min_word_length=normalization["min_word_length"],
                max_word_length=normalization["max_word_length"],
                allow_spanish_chars=normalization["allow_spanish_chars"],
            ):
                data[cleaned] = data.get(cleaned, 0) + freq
            else:
                discarded += 1

    return data, total_raw, discarded

def main():
    project_root = Path(__file__).resolve().parents[2]
    config_path = project_root / "config.json"
    with config_path.open("r", encoding="utf-8") as f:
        config = json.load(f)

    normalization = config.get("normalization", {})
    raw_corpes = project_root / "data/raw/frecuencia_elementos_corpes_1_5.txt"
    raw_csv = project_root / "data/raw/palabras_frecuencias.csv"
    
    dict_combined = {}
    total_raw_entries = 0
    total_discarded_entries = 0

    # 1. Cargar y procesar CORPES
    if raw_corpes.exists():
        corpes_data, raw_cnt, disc_cnt = process_corpes_file(
            raw_corpes,
            normalization,
        )
        total_raw_entries += raw_cnt
        total_discarded_entries += disc_cnt
        for k, v in corpes_data.items():
            dict_combined[k] = dict_combined.get(k, 0) + v

    # 2. Cargar y procesar CSV
    if raw_csv.exists():
        csv_data, raw_cnt, disc_cnt = process_csv_file(raw_csv, normalization)
        total_raw_entries += raw_cnt
        total_discarded_entries += disc_cnt
        for k, v in csv_data.items():
            dict_combined[k] = dict_combined.get(k, 0) + v

    # 3. Ordenar por frecuencia descendente
    sorted_words = sorted(dict_combined.items(), key=lambda x: x[1], reverse=True)
    
    # 4. Estadísticas
    final_count = len(sorted_words)
    duplicates_removed = total_raw_entries - total_discarded_entries - final_count
    
    length_distribution = {}
    for word, _ in sorted_words:
        length = len(word)
        length_distribution[length] = length_distribution.get(length, 0) + 1

    stats = {
        "entradas_totales_raw": total_raw_entries,
        "entradas_descartadas_invalidas": total_discarded_entries,
        "duplicados_fusionados": duplicates_removed,
        "palabras_unicas_finales": final_count,
        "distribucion_longitudes": dict(sorted(length_distribution.items()))
    }

    # 5. Guardar el conjunto limpio principal
    output_dir = project_root / "data/processed"
    output_dir.joinpath("subsets").mkdir(parents=True, exist_ok=True)
    full_output = output_dir / "dictionary_full.csv"
    with full_output.open("w", encoding="utf-8", newline="") as f:
        writer = csv.writer(f, delimiter=",")
        writer.writerow(["palabra", "frecuencia"])
        writer.writerows(sorted_words)

    # 6. Generar subconjuntos anidados según la configuración
    for size in config.get("subsets", []):
        if size <= final_count:
            subset_file = output_dir / "subsets" / f"dict_{size}.csv"
            with subset_file.open("w", encoding="utf-8", newline="") as f:
                writer = csv.writer(f, delimiter=",")
                writer.writerow(["palabra", "frecuencia"])
                writer.writerows(sorted_words[:size])

    # Guardar reporte de estadísticas
    with (output_dir / "stats_summary.json").open("w", encoding="utf-8") as f:
        json.dump(stats, f, indent=4, ensure_ascii=False)

    print("=== FASE 1 COMPLETADA ===")
    print(f"Total procesado: {total_raw_entries}")
    print(f"Descartados (signos/números): {total_discarded_entries}")
    print(f"Duplicados fusionados: {duplicates_removed}")
    print(f"Palabras finales únicas: {final_count}")
    print(f"Archivos guardados en 'data/processed/'")

if __name__ == "__main__":
    main()