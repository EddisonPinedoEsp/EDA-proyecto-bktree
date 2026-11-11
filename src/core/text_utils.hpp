#ifndef BKTREE_TEXT_UTILS_HPP
#define BKTREE_TEXT_UTILS_HPP

#include <string>

// Normalización y decodificación UTF-8. La distancia vive en levenshtein.hpp.
std::string recortar(const std::string &texto);
std::string aMinusculasEspanolUtf8Basico(const std::string &texto);
std::u32string utf8ACodepoints(const std::string &texto);

#endif
