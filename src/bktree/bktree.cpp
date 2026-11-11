#include "bktree.hpp"

#include "../core/levenshtein.hpp"
#include "../core/text_utils.hpp"

#include <algorithm>
#include <climits>
#include <unordered_map>
#include <vector>

struct BKTree::Nodo {
  std::string palabra;
  std::u32string codepoints;
  long long frecuencia;
  std::unordered_map<int, Nodo *> hijos;

  Nodo(const std::string &p, long long f)
      : palabra(p), codepoints(utf8ACodepoints(p)), frecuencia(f) {}
};

BKTree::~BKTree() {
  // Iterativo: evita desbordar la pila en árboles profundos.
  std::vector<Nodo *> pila;
  if (raiz_) pila.push_back(raiz_);
  while (!pila.empty()) {
    Nodo *n = pila.back();
    pila.pop_back();
    for (auto &h : n->hijos) pila.push_back(h.second);
    delete n;
  }
}

bool BKTree::insertar(const std::string &palabra, long long frecuencia) {
  if (!raiz_) {
    raiz_ = new Nodo(palabra, frecuencia);
    ++tamano_;
    return true;
  }
  const std::u32string cp = utf8ACodepoints(palabra);
  Nodo *actual = raiz_;
  while (true) {
    int d = distanciaLevenshtein(cp, actual->codepoints);
    if (d == 0) return false;  // duplicado
    auto it = actual->hijos.find(d);
    if (it == actual->hijos.end()) {
      actual->hijos[d] = new Nodo(palabra, frecuencia);
      ++tamano_;
      return true;
    }
    actual = it->second;
  }
}

void BKTree::buscarRadio(const Nodo *nodo, const std::u32string &q, int radio,
                         std::vector<ResultadoBusqueda> &res,
                         EstadisticasBusqueda *est) const {
  int d = distanciaLevenshtein(q, nodo->codepoints);
  if (est) { ++est->distancias; ++est->nodosExaminados; }
  if (d <= radio) res.push_back({nodo->palabra, nodo->frecuencia, d});

  for (const auto &h : nodo->hijos)
    if (h.first >= d - radio && h.first <= d + radio)
      buscarRadio(h.second, q, radio, res, est);
}

// Radio dinámico: `mejor` baja cuando se hallan candidatos más cercanos, y el
// intervalo de poda se evalúa con el valor vigente en cada hijo.
void BKTree::buscarMinimo(const Nodo *nodo, const std::u32string &q,
                          int &mejor, std::vector<ResultadoBusqueda> &res,
                          EstadisticasBusqueda *est) const {
  int d = distanciaLevenshtein(q, nodo->codepoints);
  if (est) { ++est->distancias; ++est->nodosExaminados; }
  if (d < mejor) { mejor = d; res.clear(); }
  if (d == mejor) res.push_back({nodo->palabra, nodo->frecuencia, d});

  for (const auto &h : nodo->hijos)
    if (h.first >= d - mejor && h.first <= d + mejor)
      buscarMinimo(h.second, q, mejor, res, est);
}

std::vector<ResultadoBusqueda>
BKTree::busquedaRadio(const std::string &consulta, int radio,
                      EstadisticasBusqueda *est) const {
  std::vector<ResultadoBusqueda> res;
  if (raiz_ && radio >= 0)
    buscarRadio(raiz_, utf8ACodepoints(consulta), radio, res, est);
  std::sort(res.begin(), res.end(), mejorResultado);
  return res;
}

std::vector<ResultadoBusqueda>
BKTree::claseDos(const std::string &consulta, EstadisticasBusqueda *est) const {
  std::vector<ResultadoBusqueda> res;
  int mejor = INT_MAX / 2;  // evita desbordar d + mejor
  if (raiz_) buscarMinimo(raiz_, utf8ACodepoints(consulta), mejor, res, est);
  std::sort(res.begin(), res.end(), mejorResultado);
  return res;
}

std::optional<ResultadoBusqueda>
BKTree::claseUno(const std::string &consulta, EstadisticasBusqueda *est) const {
  auto res = claseDos(consulta, est);
  if (res.empty()) return std::nullopt;
  return res.front();
}
