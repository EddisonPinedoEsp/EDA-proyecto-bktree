#include "builders/orden.hpp"
#include "check.hpp"
#include "core/dictionary.hpp"

#include <cstdio>
#include <fstream>
#include <iostream>

using namespace std;

int main() {
  // Árbol pequeño inspeccionable a mano (orden de inserción: casa, caso, cosa, mesa):
  //   casa
  //    ├─1─ caso
  //    │     └─2─ cosa      d(cosa,casa)=1 -> baja a caso; d(cosa,caso)=2
  //    └─2─ mesa            d(mesa,casa)=2
  BKTree t;
  construir(t, {{"casa", 4}, {"caso", 3}, {"cosa", 2}, {"mesa", 1}});
  EstructuraArbol e = t.analizarEstructura();
  CHECK(e.nodos == 4);
  CHECK(e.hojas == 2);
  CHECK(e.profundidadMaxima == 2);
  CHECK(e.profundidadMedia == 1.0);  // (0+1+2+1)/4
  CHECK(e.gradoMaximo == 2);
  CHECK(e.gradoMedioInternos == 1.5);  // casa:2, caso:1
  CHECK(e.histGrados.at(0) == 2 && e.histGrados.at(1) == 1 && e.histGrados.at(2) == 1);
  CHECK(e.histEtiquetas.at(1) == 1 && e.histEtiquetas.at(2) == 2);
  CHECK(e.subarbolesRaiz.size() == 2);
  CHECK(e.subarbolesRaiz[0] == make_pair(1, size_t(2)));
  CHECK(e.subarbolesRaiz[1] == make_pair(2, size_t(1)));

  // Árbol vacío.
  BKTree vacio;
  CHECK(vacio.analizarEstructura().nodos == 0);

  // Órdenes: mismo conjunto; aleatorio reproducible con la misma semilla.
  vector<InfoPalabra> v = {{"b", 1}, {"a", 2}, {"d", 3}, {"c", 4}, {"e", 5}};
  CHECK(ordenar(v, OrdenInsercion::Original, 0)[0].palabra == "b");
  CHECK(ordenar(v, OrdenInsercion::Alfabetico, 0)[0].palabra == "a");
  auto r1 = ordenar(v, OrdenInsercion::Aleatorio, 7), r2 = ordenar(v, OrdenInsercion::Aleatorio, 7);
  bool igual = true;
  for (size_t i = 0; i < v.size(); ++i) igual &= r1[i].palabra == r2[i].palabra;
  CHECK(igual && r1.size() == v.size());
  // El árbol resultante contiene el mismo conjunto sea cual sea el orden.
  BKTree ta, tb;
  construir(ta, ordenar(v, OrdenInsercion::Original, 0));
  construir(tb, ordenar(v, OrdenInsercion::Aleatorio, 3));
  CHECK(ta.tamano() == tb.tamano() && ta.tamano() == 5);

  // Carga de diccionario: encabezado, mayúsculas, vacías y duplicados.
  const char *ruta = "test_dic.csv";
  { ofstream f(ruta); f << "palabra,frecuencia\nCasa,10\n  niño ,5\n,3\ncasa,1\nx,abc\nmesa,2\n"; }
  auto d = cargarDiccionario(ruta);
  CHECK(d.size() == 3);
  CHECK(d[0].palabra == "casa" && d[0].frecuencia == 10);
  CHECK(d[1].palabra == "niño" && d[2].palabra == "mesa");
  CHECK(cargarDiccionario(ruta, 2).size() == 2);
  remove(ruta);
  bool lanzo = false;
  try { cargarDiccionario("no_existe.csv"); } catch (...) { lanzo = true; }
  CHECK(lanzo);
  return resumen("test_estructura");
}
