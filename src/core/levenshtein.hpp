#ifndef BKTREE_LEVENSHTEIN_HPP
#define BKTREE_LEVENSHTEIN_HPP

#include <string>
#include <iostream>

using namespace std;

// Distancia de Levenshtein exacta (inserción, eliminación, sustitución; costo 1)
// sobre codepoints Unicode. Memoria auxiliar O(min(|a|,|b|)).
int distanciaLevenshtein(const u32string &a, const u32string &b);
int distanciaLevenshtein(const string &utf8A, const string &utf8B);

// d(a,b) / max(|a|,|b|); 0 si ambas son vacías. Función aparte: no se usa
// dentro de las búsquedas para no alterar los contadores.
double distanciaLevenshteinNormalizada(const string &utf8A,
                                       const string &utf8B);

#endif
