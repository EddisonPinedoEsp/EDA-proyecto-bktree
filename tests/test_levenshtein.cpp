#include "check.hpp"
#include "core/levenshtein.hpp"
#include "core/text_utils.hpp"

#include <algorithm>
#include <random>
#include <string>
#include <vector>

// Oráculo independiente: matriz completa (sin optimización de memoria).
static int referencia(const std::u32string &a, const std::u32string &b) {
  std::vector<std::vector<int>> D(a.size() + 1, std::vector<int>(b.size() + 1));
  for (size_t i = 0; i <= a.size(); ++i) D[i][0] = (int)i;
  for (size_t j = 0; j <= b.size(); ++j) D[0][j] = (int)j;
  for (size_t i = 1; i <= a.size(); ++i)
    for (size_t j = 1; j <= b.size(); ++j)
      D[i][j] = std::min({D[i - 1][j] + 1, D[i][j - 1] + 1,
                          D[i - 1][j - 1] + (a[i - 1] != b[j - 1])});
  return D[a.size()][b.size()];
}

int main() {
  // Casos manuales.
  CHECK(distanciaLevenshtein("", "") == 0);
  CHECK(distanciaLevenshtein("", "abc") == 3);
  CHECK(distanciaLevenshtein("abc", "") == 3);
  CHECK(distanciaLevenshtein("casa", "casa") == 0);
  CHECK(distanciaLevenshtein("a", "b") == 1);
  CHECK(distanciaLevenshtein("casa", "caso") == 1);   // sustitución
  CHECK(distanciaLevenshtein("casa", "casas") == 1);  // inserción
  CHECK(distanciaLevenshtein("casas", "casa") == 1);  // eliminación
  CHECK(distanciaLevenshtein("kitten", "sitting") == 3);
  CHECK(distanciaLevenshtein("sunday", "saturday") == 3);
  // Acentos: a y á son distintos; ñ cuenta como un solo carácter.
  CHECK(distanciaLevenshtein("cancion", "canción") == 1);
  CHECK(distanciaLevenshtein("año", "ano") == 1);
  CHECK(distanciaLevenshtein("niño", "nino") == 1);
  CHECK(distanciaLevenshtein("canción", "canción") == 0);
  // Normalizada.
  CHECK(distanciaLevenshteinNormalizada("", "") == 0.0);
  CHECK(distanciaLevenshteinNormalizada("abc", "") == 1.0);
  CHECK(distanciaLevenshteinNormalizada("casa", "caso") == 0.25);
  // Minúsculas y recorte.
  CHECK(aMinusculasEspanolUtf8Basico("ÁÉÍÓÚÜÑ Casa") == "áéíóúüñ casa");
  CHECK(recortar("  hola \n") == "hola");
  CHECK(recortar(" \t ").empty());

  // Propiedades métricas y equivalencia con el oráculo (semilla fija).
  std::mt19937 rng(42);
  const std::u32string alfabeto = U"abcáñ";
  auto aleatoria = [&] {
    std::u32string s;
    int n = rng() % 8;
    for (int i = 0; i < n; ++i) s.push_back(alfabeto[rng() % alfabeto.size()]);
    return s;
  };
  for (int it = 0; it < 3000; ++it) {
    auto a = aleatoria(), b = aleatoria(), c = aleatoria();
    int ab = distanciaLevenshtein(a, b), ba = distanciaLevenshtein(b, a);
    int bc = distanciaLevenshtein(b, c), ac = distanciaLevenshtein(a, c);
    CHECK(ab == referencia(a, b));
    CHECK(ab >= 0);
    CHECK(ab == ba);
    CHECK((ab == 0) == (a == b));
    CHECK(ac <= ab + bc);
  }
  return resumen("test_levenshtein");
}
