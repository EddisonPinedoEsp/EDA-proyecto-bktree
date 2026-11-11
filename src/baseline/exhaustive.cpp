#include "exhaustive.hpp"

#include "../core/levenshtein.hpp"
#include "../core/text_utils.hpp"

#include <algorithm>
#include <climits>
#include <unordered_set>
#include <iostream>

using namespace std;

bool BusquedaExhaustiva::insertar(const string &palabra,
                                  long long frecuencia) {
  if (!vistas_.insert(palabra).second) return false;
  palabras_.push_back({palabra, utf8ACodepoints(palabra), frecuencia});
  return true;
}

vector<ResultadoBusqueda>
BusquedaExhaustiva::busquedaRadio(const string &consulta, int radio,
                                  EstadisticasBusqueda *est) const {
  const u32string q = utf8ACodepoints(consulta);
  vector<ResultadoBusqueda> res;
  for (const auto &e : palabras_) {
    int d = distanciaLevenshtein(q, e.codepoints);
    if (est) { ++est->distancias; ++est->nodosExaminados; }
    if (d <= radio) res.push_back({e.palabra, e.frecuencia, d});
  }
  sort(res.begin(), res.end(), mejorResultado);
  return res;
}

vector<ResultadoBusqueda>
BusquedaExhaustiva::claseDos(const string &consulta,
                             EstadisticasBusqueda *est) const {
  const u32string q = utf8ACodepoints(consulta);
  vector<ResultadoBusqueda> res;
  int mejor = INT_MAX;
  for (const auto &e : palabras_) {
    int d = distanciaLevenshtein(q, e.codepoints);
    if (est) { ++est->distancias; ++est->nodosExaminados; }
    if (d < mejor) { mejor = d; res.clear(); }
    if (d == mejor) res.push_back({e.palabra, e.frecuencia, d});
  }
  sort(res.begin(), res.end(), mejorResultado);
  return res;
}

optional<ResultadoBusqueda>
BusquedaExhaustiva::claseUno(const string &consulta,
                             EstadisticasBusqueda *est) const {
  auto res = claseDos(consulta, est);
  if (res.empty()) return nullopt;
  return res.front();
}
