#ifndef BKTREE_BK_TREE_HPP
#define BKTREE_BK_TREE_HPP

#include "../core/types.hpp"

#include <optional>
#include <string>
#include <vector>

// BK-Tree (File Structure 1 / Search Algorithm 1 de Burkhard y Keller, 1973)
// sobre distancia de Levenshtein. El orden de inserción define el árbol.
class BKTree {
public:
  BKTree() = default;
  ~BKTree();
  BKTree(const BKTree &) = delete;
  BKTree &operator=(const BKTree &) = delete;

  // Ignora palabras ya presentes (distancia 0 al pivote). true si se agregó.
  bool insertar(const std::string &palabra, long long frecuencia);
  size_t tamano() const { return tamano_; }

  std::vector<ResultadoBusqueda>
  busquedaRadio(const std::string &consulta, int radio,
                EstadisticasBusqueda *est = nullptr) const;
  std::optional<ResultadoBusqueda>
  claseUno(const std::string &consulta,
           EstadisticasBusqueda *est = nullptr) const;
  std::vector<ResultadoBusqueda>
  claseDos(const std::string &consulta,
           EstadisticasBusqueda *est = nullptr) const;

private:
  struct Nodo;
  Nodo *raiz_ = nullptr;
  size_t tamano_ = 0;

  void buscarRadio(const Nodo *nodo, const std::u32string &q, int radio,
                   std::vector<ResultadoBusqueda> &res,
                   EstadisticasBusqueda *est) const;
  void buscarMinimo(const Nodo *nodo, const std::u32string &q, int &mejor,
                    std::vector<ResultadoBusqueda> &res,
                    EstadisticasBusqueda *est) const;
};

#endif
