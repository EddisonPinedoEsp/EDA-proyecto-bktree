#include "bktree.hpp"

#include "../core/levenshtein.hpp"
#include "../core/text_utils.hpp"

#include <algorithm>
#include <climits>
#include <unordered_map>
#include <vector>
#include <iostream>

using namespace std;

struct BKTree::Nodo {
  string palabra;
  u32string codepoints;
  long long frecuencia;
  unordered_map<int, Nodo *> hijos;

  Nodo(const string &p, long long f)
      : palabra(p), codepoints(utf8ACodepoints(p)), frecuencia(f) {}
};

BKTree::~BKTree() {
  // Iterativo: evita desbordar la pila en árboles profundos.
  vector<Nodo *> pila;
  if (raiz_) pila.push_back(raiz_);
  while (!pila.empty()) {
    Nodo *n = pila.back();
    pila.pop_back();
    for (auto &h : n->hijos) pila.push_back(h.second);
    delete n;
  }
}

bool BKTree::insertar(const string &palabra, long long frecuencia) {
  if (!raiz_) {
    raiz_ = new Nodo(palabra, frecuencia);
    ++tamano_;
    return true;
  }
  const u32string cp = utf8ACodepoints(palabra);
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

void BKTree::buscarRadio(const Nodo *nodo, const u32string &q, int radio,
                         vector<ResultadoBusqueda> &res,
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
void BKTree::buscarMinimo(const Nodo *nodo, const u32string &q,
                          int &mejor, vector<ResultadoBusqueda> &res,
                          EstadisticasBusqueda *est) const {
  int d = distanciaLevenshtein(q, nodo->codepoints);
  if (est) { ++est->distancias; ++est->nodosExaminados; }
  if (d < mejor) { mejor = d; res.clear(); }
  if (d == mejor) res.push_back({nodo->palabra, nodo->frecuencia, d});

  for (const auto &h : nodo->hijos)
    if (h.first >= d - mejor && h.first <= d + mejor)
      buscarMinimo(h.second, q, mejor, res, est);
}

vector<ResultadoBusqueda>
BKTree::busquedaRadio(const string &consulta, int radio,
                      EstadisticasBusqueda *est) const {
  vector<ResultadoBusqueda> res;
  if (raiz_ && radio >= 0)
    buscarRadio(raiz_, utf8ACodepoints(consulta), radio, res, est);
  sort(res.begin(), res.end(), mejorResultado);
  return res;
}

vector<ResultadoBusqueda>
BKTree::claseDos(const string &consulta, EstadisticasBusqueda *est) const {
  vector<ResultadoBusqueda> res;
  int mejor = INT_MAX / 2;  // evita desbordar d + mejor
  if (raiz_) buscarMinimo(raiz_, utf8ACodepoints(consulta), mejor, res, est);
  sort(res.begin(), res.end(), mejorResultado);
  return res;
}

optional<ResultadoBusqueda>
BKTree::claseUno(const string &consulta, EstadisticasBusqueda *est) const {
  auto res = claseDos(consulta, est);
  if (res.empty()) return nullopt;
  return res.front();
}

EstructuraArbol BKTree::analizarEstructura() const {
  EstructuraArbol e;
  if (!raiz_) return e;

  long long sumaProf = 0, sumaHijosInternos = 0, internos = 0;
  std::vector<std::pair<const Nodo *, int>> pila{{raiz_, 0}};
  while (!pila.empty()) {
    auto [n, prof] = pila.back();
    pila.pop_back();
    ++e.nodos;
    sumaProf += prof;
    e.profundidadMaxima = std::max(e.profundidadMaxima, prof);
    int grado = static_cast<int>(n->hijos.size());
    ++e.histGrados[grado];
    e.gradoMaximo = std::max(e.gradoMaximo, grado);
    if (grado == 0) {
      ++e.hojas;
    } else {
      ++internos;
      sumaHijosInternos += grado;
    }
    for (const auto &h : n->hijos) {
      ++e.histEtiquetas[h.first];
      pila.push_back({h.second, prof + 1});
    }
  }
  e.profundidadMedia = static_cast<double>(sumaProf) / e.nodos;
  if (internos) e.gradoMedioInternos = static_cast<double>(sumaHijosInternos) / internos;

  for (const auto &h : raiz_->hijos) {
    size_t tam = 0;
    std::vector<const Nodo *> st{h.second};
    while (!st.empty()) {
      const Nodo *n = st.back();
      st.pop_back();
      ++tam;
      for (const auto &c : n->hijos) st.push_back(c.second);
    }
    e.subarbolesRaiz.push_back({h.first, tam});
  }
  std::sort(e.subarbolesRaiz.begin(), e.subarbolesRaiz.end());
  return e;
}
