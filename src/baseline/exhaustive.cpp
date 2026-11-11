#include "exhaustive.hpp"

#include "../core/levenshtein.hpp"
#include "../core/text_utils.hpp"

#include <algorithm>
#include <climits>
#include <unordered_set>

bool BusquedaExhaustiva::insertar(const std::string &palabra,
                                  long long frecuencia) {
  if (!vistas_.insert(palabra).second) return false;
  palabras_.push_back({palabra, utf8ACodepoints(palabra), frecuencia});
  return true;
}

std::vector<ResultadoBusqueda>
BusquedaExhaustiva::busquedaRadio(const std::string &consulta, int radio,
                                  EstadisticasBusqueda *est) const {
  const std::u32string q = utf8ACodepoints(consulta);
  std::vector<ResultadoBusqueda> res;
  for (const auto &e : palabras_) {
    int d = distanciaLevenshtein(q, e.codepoints);
    if (est) { ++est->distancias; ++est->nodosExaminados; }
    if (d <= radio) res.push_back({e.palabra, e.frecuencia, d});
  }
  std::sort(res.begin(), res.end(), mejorResultado);
  return res;
}

std::vector<ResultadoBusqueda>
BusquedaExhaustiva::claseDos(const std::string &consulta,
                             EstadisticasBusqueda *est) const {
  const std::u32string q = utf8ACodepoints(consulta);
  std::vector<ResultadoBusqueda> res;
  int mejor = INT_MAX;
  for (const auto &e : palabras_) {
    int d = distanciaLevenshtein(q, e.codepoints);
    if (est) { ++est->distancias; ++est->nodosExaminados; }
    if (d < mejor) { mejor = d; res.clear(); }
    if (d == mejor) res.push_back({e.palabra, e.frecuencia, d});
  }
  std::sort(res.begin(), res.end(), mejorResultado);
  return res;
}

std::optional<ResultadoBusqueda>
BusquedaExhaustiva::claseUno(const std::string &consulta,
                             EstadisticasBusqueda *est) const {
  auto res = claseDos(consulta, est);
  if (res.empty()) return std::nullopt;
  return res.front();
}
