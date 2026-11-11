#include "dictionary.hpp"

#include "text_utils.hpp"

#include <fstream>
#include <stdexcept>
#include <unordered_set>
#include <iostream>

using namespace std;

vector<InfoPalabra> cargarDiccionario(const string &ruta,
                                           size_t maximo) {
  ifstream in(ruta);
  if (!in) throw runtime_error("no se pudo abrir " + ruta);

  vector<InfoPalabra> res;
  unordered_set<string> vistas;
  string linea;
  while (getline(in, linea)) {
    size_t coma = linea.rfind(',');
    if (coma == string::npos) continue;
    string palabra =
        aMinusculasEspanolUtf8Basico(recortar(linea.substr(0, coma)));
    string f = recortar(linea.substr(coma + 1));
    if (palabra.empty() || f.empty() ||
        f.find_first_not_of("0123456789") != string::npos)
      continue;  // encabezado o línea inválida
    if (!vistas.insert(palabra).second) continue;
    res.push_back({palabra, stoll(f)});
    if (maximo && res.size() >= maximo) break;
  }
  return res;
}
