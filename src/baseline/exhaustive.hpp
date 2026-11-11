#ifndef BKTREE_EXHAUSTIVE_HPP
#define BKTREE_EXHAUSTIVE_HPP

#include "../core/types.hpp"

#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

// Búsqueda exhaustiva: línea base de rendimiento y oráculo de correctitud.
// Mismo conjunto (sin duplicados) y misma política de desempate que el BK-Tree.
class BusquedaExhaustiva {
public:
  // Ignora palabras ya presentes. Devuelve true si se agregó.
  bool insertar(const std::string &palabra, long long frecuencia);
  size_t tamano() const { return palabras_.size(); }

  // Todas las palabras con d(q,w) <= radio, ordenadas con mejorResultado.
  std::vector<ResultadoBusqueda>
  busquedaRadio(const std::string &consulta, int radio,
                EstadisticasBusqueda *est = nullptr) const;
  // Clase 1: una palabra a distancia mínima (la primera según el desempate).
  std::optional<ResultadoBusqueda>
  claseUno(const std::string &consulta,
           EstadisticasBusqueda *est = nullptr) const;
  // Clase 2: todas las palabras a distancia mínima.
  std::vector<ResultadoBusqueda>
  claseDos(const std::string &consulta,
           EstadisticasBusqueda *est = nullptr) const;

private:
  struct Entrada {
    std::string palabra;
    std::u32string codepoints;
    long long frecuencia;
  };
  std::vector<Entrada> palabras_;
  std::unordered_set<std::string> vistas_;
};

#endif
