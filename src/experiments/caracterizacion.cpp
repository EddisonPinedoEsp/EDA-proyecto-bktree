// Fase 4: caracterización de distancias y estructura del BK-Tree.
//
// Uso: caracterizacion [clave=valor ...]
//   dir=data/processed/subsets   carpeta con dict_<N>.csv (subconjuntos anidados
//                                por frecuencia, ver config.json)
//   tamanos=1000,5000,10000      tamaños de diccionario
//   semillas=5                   nº de semillas (1..K) por tamaño y orden aleatorio
//   pares=200000                 pares muestreados para la distribución de distancias
//   consultas=300                consultas por tipo
//   radios=1,2,3                 radios de búsqueda
//   salida=resultados/caracterizacion
//   verificar=1                  compara contra la búsqueda exhaustiva (consultas
//                                de los primeros 100 de cada tipo)
//
// Salidas (CSV, todas con población, tamaño y semilla explícitos):
//   distancias_pares.csv  histograma de d(a,b) entre pares ALEATORIOS distintos
//   resumen_distancias.csv estadísticas descriptivas de esa muestra
//   estructura.csv        métricas estructurales por (tamaño, orden, semilla)
//   hist_etiquetas.csv    distribución de etiquetas de aristas nodo->hijo
//   hist_grados.csv       distribución del número de hijos
//   subarboles_raiz.csv   tamaño de cada subárbol hijo de la raíz
//   consultas.csv         costo de consulta (distancias, % examinado)
#include "baseline/exhaustive.hpp"
#include "bktree/bktree.hpp"
#include "builders/orden.hpp"
#include "core/dictionary.hpp"
#include "core/levenshtein.hpp"
#include "core/text_utils.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <random>
#include <sstream>
#include <iostream>

using namespace std;
namespace fs = filesystem;

struct Resumen {
  double media = 0, desv = 0, min = 0, max = 0;
};

static Resumen resumir(const vector<double> &v) {
  Resumen r;
  if (v.empty()) return r;
  double s = 0;
  r.min = r.max = v[0];
  for (double x : v) { s += x; r.min = min(r.min, x); r.max = max(r.max, x); }
  r.media = s / v.size();
  double q = 0;
  for (double x : v) q += (x - r.media) * (x - r.media);
  r.desv = sqrt(q / v.size());
  return r;
}

static double cuantil(vector<int> ordenado, double p) {
  sort(ordenado.begin(), ordenado.end());
  double pos = p * (ordenado.size() - 1);
  size_t i = static_cast<size_t>(pos);
  double f = pos - i;
  return i + 1 < ordenado.size() ? ordenado[i] * (1 - f) + ordenado[i + 1] * f
                                  : ordenado[i];
}

static vector<string> separar(const string &s) {
  vector<string> r;
  stringstream ss(s);
  string t;
  while (getline(ss, t, ',')) if (!t.empty()) r.push_back(t);
  return r;
}

// Aplica `ediciones` ediciones aleatorias (sustitución, inserción, eliminación).
static string perturbar(const string &w, int ediciones,
                             mt19937_64 &rng) {
  static const u32string alfabeto = U"abcdefghijklmnopqrstuvwxyzáéíóúñ";
  u32string s = utf8ACodepoints(w);
  for (int k = 0; k < ediciones; ++k) {
    int op = s.empty() ? 1 : rng() % 3;
    char32_t c = alfabeto[rng() % alfabeto.size()];
    if (op == 0) s[rng() % s.size()] = c;
    else if (op == 1) s.insert(s.begin() + rng() % (s.size() + 1), c);
    else s.erase(s.begin() + rng() % s.size());
  }
  return codepointsAUtf8(s);
}

struct Consulta {
  string tipo;
  string texto;
};

int main(int argc, char **argv) {
  map<string, string> arg = {
      {"dir", "data/processed/subsets"}, {"tamanos", "1000,5000,10000"},
      {"semillas", "5"},   {"pares", "200000"},
      {"consultas", "300"}, {"radios", "1,2,3"},
      {"salida", "resultados/caracterizacion"}, {"verificar", "1"}};
  for (int i = 1; i < argc; ++i) {
    string a = argv[i];
    size_t e = a.find('=');
    if (e == string::npos) { cerr << "argumento inválido: " << a << "\n"; return 2; }
    arg[a.substr(0, e)] = a.substr(e + 1);
  }
  const int nSemillas = stoi(arg["semillas"]);
  const size_t nPares = stoul(arg["pares"]);
  const size_t nConsultas = stoul(arg["consultas"]);
  const bool verificar = arg["verificar"] == "1";
  vector<int> radios;
  for (auto &r : separar(arg["radios"])) radios.push_back(stoi(r));

  fs::create_directories(arg["salida"]);
  auto abrir = [&](const char *n) {
    ofstream f((fs::path(arg["salida"]) / n).string());
    f.precision(6);
    return f;
  };
  auto fPares = abrir("distancias_pares.csv");
  auto fResumen = abrir("resumen_distancias.csv");
  auto fEstr = abrir("estructura.csv");
  auto fEtiq = abrir("hist_etiquetas.csv");
  auto fGrados = abrir("hist_grados.csv");
  auto fSub = abrir("subarboles_raiz.csv");
  auto fCons = abrir("consultas.csv");
  fPares << "tamano,semilla,pares,distancia,frecuencia\n";
  fResumen << "tamano,semilla,pares,media,mediana,desv,min,p5,p25,p75,p95,max\n";
  fEstr << "tamano,orden,semilla,nodos,hojas,prof_max,prof_media,grado_max,"
           "grado_medio_internos,hijos_raiz,max_subarbol_raiz_frac,"
           "construccion_ms\n";
  fEtiq << "tamano,orden,semilla,etiqueta,aristas\n";
  fGrados << "tamano,orden,semilla,hijos,nodos\n";
  fSub << "tamano,orden,semilla,etiqueta,nodos_subarbol\n";
  fCons << "tamano,orden,semilla,tipo_consulta,radio,n_consultas,"
           "dist_media,dist_desv,dist_min,dist_max,"
           "pct_examinado_media,pct_examinado_desv,pct_examinado_min,"
           "pct_examinado_max,busqueda_us_media,aciertos_media,correcto\n";

  const uint64_t semillaGlobal = 42;
  bool todoCorrecto = true;

  for (auto &sz : separar(arg["tamanos"])) {
    const size_t n = stoul(sz);
    const string ruta = arg["dir"] + "/dict_" + sz + ".csv";
    vector<InfoPalabra> dic = cargarDiccionario(ruta);
    printf("== tamaño %zu (%zu palabras leídas de %s)\n", n, dic.size(), ruta.c_str());
    if (dic.empty()) continue;

    // --- Distribución de distancias entre pares aleatorios (muestreo) ---
    {
      mt19937_64 rng(semillaGlobal + n);
      vector<int> ds;
      ds.reserve(nPares);
      map<int, size_t> hist;
      for (size_t k = 0; k < nPares && dic.size() > 1; ++k) {
        size_t i = rng() % dic.size(), j = rng() % (dic.size() - 1);
        if (j >= i) ++j;  // pares distintos
        int d = distanciaLevenshtein(dic[i].palabra, dic[j].palabra);
        ds.push_back(d);
        ++hist[d];
      }
      for (auto &[d, c] : hist) fPares << n << "," << semillaGlobal + n << "," << ds.size() << "," << d << "," << c << "\n";
      vector<double> dv(ds.begin(), ds.end());
      Resumen r = resumir(dv);
      fResumen << n << "," << semillaGlobal + n << "," << ds.size() << "," << r.media
               << "," << cuantil(ds, .5) << "," << r.desv << "," << r.min << ","
               << cuantil(ds, .05) << "," << cuantil(ds, .25) << ","
               << cuantil(ds, .75) << "," << cuantil(ds, .95) << "," << r.max << "\n";
      printf("   distancia entre pares aleatorios: media=%.2f desv=%.2f (muestra de %zu pares)\n",
                  r.media, r.desv, ds.size());
    }

    // --- Consultas (mismas para todas las variantes de este tamaño) ---
    vector<Consulta> consultas;
    {
      mt19937_64 rng(semillaGlobal * 31 + n);
      for (size_t k = 0; k < nConsultas; ++k) {
        const string &w = dic[rng() % dic.size()].palabra;
        consultas.push_back({"presente", w});
      }
      for (size_t k = 0; k < nConsultas; ++k) {
        const string &w = dic[rng() % dic.size()].palabra;
        consultas.push_back({"error_1", perturbar(w, 1, rng)});
      }
      for (size_t k = 0; k < nConsultas; ++k) {
        const string &w = dic[rng() % dic.size()].palabra;
        consultas.push_back({"error_2", perturbar(w, 2, rng)});
      }
    }

    BusquedaExhaustiva exh;
    if (verificar) for (auto &p : dic) exh.insertar(p.palabra, p.frecuencia);

    struct Variante { OrdenInsercion orden; int semilla; };
    vector<Variante> variantes = {{OrdenInsercion::Original, 0},
                                       {OrdenInsercion::Alfabetico, 0}};
    for (int s = 1; s <= nSemillas; ++s) variantes.push_back({OrdenInsercion::Aleatorio, s});

    for (auto &v : variantes) {
      const string orden = nombreOrden(v.orden);
      auto ordenado = ordenar(dic, v.orden, v.semilla);
      BKTree arbol;
      auto t0 = chrono::steady_clock::now();
      construir(arbol, ordenado);
      double ms = chrono::duration<double, milli>(chrono::steady_clock::now() - t0).count();

      EstructuraArbol e = arbol.analizarEstructura();
      size_t maxSub = 0;
      for (auto &[et, t] : e.subarbolesRaiz) {
        maxSub = max(maxSub, t);
        fSub << n << "," << orden << "," << v.semilla << "," << et << "," << t << "\n";
      }
      for (auto &[et, c] : e.histEtiquetas) fEtiq << n << "," << orden << "," << v.semilla << "," << et << "," << c << "\n";
      for (auto &[g, c] : e.histGrados) fGrados << n << "," << orden << "," << v.semilla << "," << g << "," << c << "\n";
      fEstr << n << "," << orden << "," << v.semilla << "," << e.nodos << "," << e.hojas
            << "," << e.profundidadMaxima << "," << e.profundidadMedia << ","
            << e.gradoMaximo << "," << e.gradoMedioInternos << ","
            << e.subarbolesRaiz.size() << ","
            << (e.nodos > 1 ? double(maxSub) / (e.nodos - 1) : 0.0) << "," << ms << "\n";
      printf("   %-10s s=%d nodos=%zu prof_max=%d prof_media=%.2f grado_max=%d build=%.0fms\n",
                  orden.c_str(), v.semilla, e.nodos, e.profundidadMaxima, e.profundidadMedia, e.gradoMaximo, ms);

      // --- Costo de consultas por tipo y radio (radio 0 = coincidencia exacta) ---
      for (const string tipo : {"presente", "error_1", "error_2"}) {
        for (int pasada = -1; pasada < (int)radios.size(); ++pasada) {
          // pasada -1: clase 2 (vecino más cercano); resto: búsqueda por radio
          vector<double> dist, pct, us, aciertos;
          bool ok = true;
          size_t vistas = 0;
          for (auto &q : consultas) {
            if (q.tipo != tipo) continue;
            EstadisticasBusqueda st;
            auto t1 = chrono::steady_clock::now();
            vector<ResultadoBusqueda> res =
                pasada < 0 ? arbol.claseDos(q.texto, &st)
                           : arbol.busquedaRadio(q.texto, radios[pasada], &st);
            us.push_back(chrono::duration<double, micro>(chrono::steady_clock::now() - t1).count());
            dist.push_back(st.distancias);
            pct.push_back(100.0 * st.nodosExaminados / arbol.tamano());
            aciertos.push_back(res.size());
            if (verificar && vistas++ < 100) {
              auto ref = pasada < 0 ? exh.claseDos(q.texto) : exh.busquedaRadio(q.texto, radios[pasada]);
              bool igual = ref.size() == res.size();
              for (size_t i = 0; igual && i < ref.size(); ++i)
                igual = ref[i].palabra == res[i].palabra && ref[i].distancia == res[i].distancia;
              if (!igual) { ok = false; todoCorrecto = false; }
            }
          }
          Resumen rd = resumir(dist), rp = resumir(pct), ru = resumir(us), ra = resumir(aciertos);
          fCons << n << "," << orden << "," << v.semilla << "," << tipo << ","
                << (pasada < 0 ? string("clase2") : to_string(radios[pasada])) << ","
                << dist.size() << "," << rd.media << "," << rd.desv << "," << rd.min << "," << rd.max << ","
                << rp.media << "," << rp.desv << "," << rp.min << "," << rp.max << ","
                << ru.media << "," << ra.media << "," << (verificar ? (ok ? "1" : "0") : "NA") << "\n";
        }
      }
    }
  }
  if (verificar) printf(todoCorrecto ? "Verificación contra búsqueda exhaustiva: OK\n"
                                          : "Verificación contra búsqueda exhaustiva: FALLÓ\n");
  printf("Resultados en %s\n", arg["salida"].c_str());
  return todoCorrecto ? 0 : 1;
}
