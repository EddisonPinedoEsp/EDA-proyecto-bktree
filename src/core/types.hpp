#ifndef BKTREE_TYPES_HPP
#define BKTREE_TYPES_HPP

#include <string>
#include <iostream>

using namespace std;

struct InfoPalabra {
  string palabra;
  long long frecuencia;
};

struct ResultadoBusqueda {
  string palabra;
  long long frecuencia;
  int distancia;
};

// Contadores explícitos de costo. No influyen en el resultado de las consultas.
//  - distancias:      llamadas a la función de distancia consulta-pivote.
//  - nodosExaminados: palabras cuya distancia a la consulta fue calculada
//                     (definición de "nodo examinado" del documento técnico).
struct EstadisticasBusqueda {
  long long distancias = 0;
  long long nodosExaminados = 0;
};

// Política de desempate única para TODAS las estructuras (BK-Tree y búsqueda
// exhaustiva): menor distancia, luego mayor frecuencia, luego orden
// alfabético de la palabra.
inline bool mejorResultado(const ResultadoBusqueda &a,
                           const ResultadoBusqueda &b) {
  if (a.distancia != b.distancia) return a.distancia < b.distancia;
  if (a.frecuencia != b.frecuencia) return a.frecuencia > b.frecuencia;
  return a.palabra < b.palabra;
}

#endif
