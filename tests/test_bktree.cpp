#include "baseline/exhaustive.hpp"
#include "bktree/bktree.hpp"
#include "check.hpp"
#include "core/levenshtein.hpp"

#include <random>
#include <string>
#include <vector>

static bool iguales(const std::vector<ResultadoBusqueda> &a,
                    const std::vector<ResultadoBusqueda> &b) {
  if (a.size() != b.size()) return false;
  for (size_t i = 0; i < a.size(); ++i)
    if (a[i].palabra != b[i].palabra || a[i].frecuencia != b[i].frecuencia ||
        a[i].distancia != b[i].distancia)
      return false;
  return true;
}

static void comparar(const BKTree &t, const BusquedaExhaustiva &e,
                     const std::string &q, int maxRadio) {
  for (int r = 0; r <= maxRadio; ++r)
    CHECK(iguales(t.busquedaRadio(q, r), e.busquedaRadio(q, r)));
  CHECK(iguales(t.claseDos(q), e.claseDos(q)));
  auto u1 = t.claseUno(q), u2 = e.claseUno(q);
  CHECK(u1.has_value() == u2.has_value());
  if (u1 && u2) CHECK(u1->palabra == u2->palabra && u1->distancia == u2->distancia);
}

static void casosBorde() {
  BKTree t;
  BusquedaExhaustiva e;
  // Árbol vacío.
  CHECK(t.busquedaRadio("a", 2).empty());
  CHECK(t.claseDos("a").empty());
  CHECK(!t.claseUno("a").has_value());
  CHECK(!e.claseUno("a").has_value());

  CHECK(t.insertar("casa", 10) && e.insertar("casa", 10));
  // Palabra repetida: se ignora.
  CHECK(!t.insertar("casa", 99) && !e.insertar("casa", 99));
  CHECK(t.tamano() == 1);
  // Distancia cero y radio cero.
  auto r = t.busquedaRadio("casa", 0);
  CHECK(r.size() == 1 && r[0].distancia == 0 && r[0].frecuencia == 10);
  CHECK(t.busquedaRadio("caso", 0).empty());
  CHECK(t.busquedaRadio("caso", -1).empty());

  // Empates: "caso" y "cosa" a distancia 1 de "casa"; gana mayor frecuencia.
  t.insertar("caso", 5); e.insertar("caso", 5);
  t.insertar("cosa", 7); e.insertar("cosa", 7);
  t.insertar("mesa", 1); e.insertar("mesa", 1);
  auto c2 = t.claseDos("casi");  // casa, caso -> 1 ; cosa -> 2
  CHECK(c2.size() == 2 && c2[0].palabra == "casa" && c2[1].palabra == "caso");
  CHECK(t.claseUno("casi")->palabra == "casa");
  auto c2b = t.claseDos("casaa");  // casa -> 1, caso -> 2
  CHECK(c2b.size() == 1);
  comparar(t, e, "casi", 4);
  comparar(t, e, "", 4);
  comparar(t, e, "canción", 4);

  // Contadores: coherentes y la exhaustiva examina todo.
  EstadisticasBusqueda st, se;
  t.busquedaRadio("casa", 1, &st);
  e.busquedaRadio("casa", 1, &se);
  CHECK(se.nodosExaminados == (long long)e.tamano());
  CHECK(st.distancias == st.nodosExaminados);
  CHECK(st.nodosExaminados >= 1 && st.nodosExaminados <= (long long)t.tamano());
}

static void aleatorio() {
  std::mt19937 rng(42);
  const std::u32string alf = U"abcdeáñ";
  auto utf8 = [](const std::u32string &s) {
    std::string o;
    for (char32_t c : s) {
      if (c < 0x80) o.push_back((char)c);
      else { o.push_back((char)(0xC0 | (c >> 6))); o.push_back((char)(0x80 | (c & 0x3F))); }
    }
    return o;
  };
  auto palabra = [&] {
    std::u32string s;
    int n = 1 + rng() % 7;
    for (int i = 0; i < n; ++i) s.push_back(alf[rng() % alf.size()]);
    return utf8(s);
  };
  for (int ronda = 0; ronda < 5; ++ronda) {
    BKTree t;
    BusquedaExhaustiva e;
    int n = 50 + 300 * ronda;
    for (int i = 0; i < n; ++i) {
      std::string w = palabra();
      long long f = 1 + rng() % 5;  // pocas frecuencias para forzar empates
      CHECK(t.insertar(w, f) == e.insertar(w, f));
    }
    CHECK(t.tamano() == e.tamano());
    for (int i = 0; i < 200; ++i) comparar(t, e, palabra(), 4);
  }
}

int main() {
  casosBorde();
  aleatorio();
  return resumen("test_bktree");
}
