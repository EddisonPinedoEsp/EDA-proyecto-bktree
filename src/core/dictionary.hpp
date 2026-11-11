#ifndef BKTREE_DICTIONARY_HPP
#define BKTREE_DICTIONARY_HPP

#include "types.hpp"

#include <string>
#include <vector>
#include <iostream>

using namespace std;

// Lee un CSV "palabra,frecuencia" (con o sin encabezado). Aplica la misma
// normalización que el resto del proyecto (recorte, minúsculas), descarta
// vacías y duplicados (conserva la primera aparición) y respeta el orden del
// archivo. `maximo` = 0 significa sin límite. Lanza runtime_error si el
// archivo no se puede abrir.
vector<InfoPalabra> cargarDiccionario(const string &ruta,
                                           size_t maximo = 0);

#endif
