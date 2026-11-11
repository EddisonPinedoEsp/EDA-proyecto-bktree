#ifndef BKTREE_TEXT_UTILS_HPP
#define BKTREE_TEXT_UTILS_HPP

#include <string>
#include <iostream>

using namespace std;

// Normalización y decodificación UTF-8. La distancia vive en levenshtein.hpp.
string recortar(const string &texto);
string aMinusculasEspanolUtf8Basico(const string &texto);
u32string utf8ACodepoints(const string &texto);
string codepointsAUtf8(const u32string &texto);

#endif
