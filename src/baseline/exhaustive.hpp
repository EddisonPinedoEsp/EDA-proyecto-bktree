#ifndef BKTREE_EXHAUSTIVE_HPP
#define BKTREE_EXHAUSTIVE_HPP

#include "../core/types.hpp"

#include <optional>
#include <string>
#include <unordered_set>
#include <vector>
#include <iostream>

using namespace std;

// Búsqueda exhaustiva: línea base de rendimiento y oráculo de correctitud.
// Mismo conjunto (sin duplicados) y misma política de desempate que el BK-Tree.
class BusquedaExhaustiva {
public:
  // Ignora palabras ya presentes. Devuelve true si se agregó.
  bool insertar(const string &palabra, long long frecuencia);
  size_t tamano() const { return palabras_.size(); }

  // Todas las palabras con d(q,w) <= radio, ordenadas con mejorResultado.
  vector<ResultadoBusqueda>
  busquedaRadio(const string &consulta, int radio,
                EstadisticasBusqueda *est = nullptr) const;
  // Clase 1: una palabra a distancia mínima (la primera según el desempate).
  optional<ResultadoBusqueda>
  claseUno(const string &consulta,
           EstadisticasBusqueda *est = nullptr) const;
  // Clase 2: todas las palabras a distancia mínima.
  vector<ResultadoBusqueda>
  claseDos(const string &consulta,
           EstadisticasBusqueda *est = nullptr) const;

private:
  struct Entrada {
    string palabra;
    u32string codepoints;
    long long frecuencia;
  };
  vector<Entrada> palabras_;
  unordered_set<string> vistas_;
};

#endif
