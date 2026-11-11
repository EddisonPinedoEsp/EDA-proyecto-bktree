#ifndef BKTREE_BK_TREE_HPP
#define BKTREE_BK_TREE_HPP

#include "../core/types.hpp"

#include <map>
#include <optional>
#include <string>
#include <vector>
#include <iostream>

using namespace std;

// Métricas estructurales (profundidad de la raíz = 0).
struct EstructuraArbol {
  size_t nodos = 0;
  size_t hojas = 0;
  int profundidadMaxima = 0;
  double profundidadMedia = 0;       // media sobre todos los nodos
  int gradoMaximo = 0;               // máx. de hijos de un nodo
  double gradoMedioInternos = 0;     // media de hijos en nodos no hoja
  std::map<int, size_t> histGrados;     // nº de hijos -> nº de nodos
  std::map<int, size_t> histEtiquetas;  // etiqueta de arista -> nº de aristas
  // (etiqueta, tamaño del subárbol) de cada hijo de la raíz, por etiqueta.
  std::vector<std::pair<int, size_t>> subarbolesRaiz;
};

// BK-Tree (File Structure 1 / Search Algorithm 1 de Burkhard y Keller, 1973)
// sobre distancia de Levenshtein. El orden de inserción define el árbol.
class BKTree {
public:
  BKTree() = default;
  ~BKTree();
  BKTree(const BKTree &) = delete;
  BKTree &operator=(const BKTree &) = delete;

  // Ignora palabras ya presentes (distancia 0 al pivote). true si se agregó.
  bool insertar(const string &palabra, long long frecuencia);
  size_t tamano() const { return tamano_; }

  vector<ResultadoBusqueda>
  busquedaRadio(const string &consulta, int radio,
                EstadisticasBusqueda *est = nullptr) const;
  optional<ResultadoBusqueda>
  claseUno(const string &consulta,
           EstadisticasBusqueda *est = nullptr) const;
  vector<ResultadoBusqueda>
  claseDos(const string &consulta,
           EstadisticasBusqueda *est = nullptr) const;

  EstructuraArbol analizarEstructura() const;

private:
  struct Nodo;
  Nodo *raiz_ = nullptr;
  size_t tamano_ = 0;

  void buscarRadio(const Nodo *nodo, const u32string &q, int radio,
                   vector<ResultadoBusqueda> &res,
                   EstadisticasBusqueda *est) const;
  void buscarMinimo(const Nodo *nodo, const u32string &q, int &mejor,
                    vector<ResultadoBusqueda> &res,
                    EstadisticasBusqueda *est) const;
};

#endif
