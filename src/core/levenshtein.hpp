#ifndef BKTREE_LEVENSHTEIN_HPP
#define BKTREE_LEVENSHTEIN_HPP

#include <string>

// Distancia de Levenshtein exacta (inserción, eliminación, sustitución; costo 1)
// sobre codepoints Unicode. Memoria auxiliar O(min(|a|,|b|)).
int distanciaLevenshtein(const std::u32string &a, const std::u32string &b);
int distanciaLevenshtein(const std::string &utf8A, const std::string &utf8B);

// d(a,b) / max(|a|,|b|); 0 si ambas son vacías. Función aparte: no se usa
// dentro de las búsquedas para no alterar los contadores.
double distanciaLevenshteinNormalizada(const std::string &utf8A,
                                       const std::string &utf8B);

#endif
