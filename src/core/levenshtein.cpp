#include "levenshtein.hpp"

#include "text_utils.hpp"

#include <algorithm>
#include <vector>
#include <iostream>

using namespace std;

int distanciaLevenshtein(const u32string &a, const u32string &b) {
  const u32string &corta = (a.size() <= b.size()) ? a : b;
  const u32string &larga = (a.size() <= b.size()) ? b : a;
  const size_t m = corta.size();

  vector<int> anterior(m + 1), actual(m + 1);
  for (size_t j = 0; j <= m; ++j) anterior[j] = static_cast<int>(j);

  for (size_t i = 1; i <= larga.size(); ++i) {
    actual[0] = static_cast<int>(i);
    for (size_t j = 1; j <= m; ++j) {
      int costo = (larga[i - 1] == corta[j - 1]) ? 0 : 1;
      actual[j] = min({anterior[j] + 1, actual[j - 1] + 1,
                            anterior[j - 1] + costo});
    }
    anterior.swap(actual);
  }
  return anterior[m];
}

int distanciaLevenshtein(const string &utf8A, const string &utf8B) {
  return distanciaLevenshtein(utf8ACodepoints(utf8A), utf8ACodepoints(utf8B));
}

double distanciaLevenshteinNormalizada(const string &utf8A,
                                       const string &utf8B) {
  u32string a = utf8ACodepoints(utf8A), b = utf8ACodepoints(utf8B);
  size_t maxima = max(a.size(), b.size());
  if (maxima == 0) return 0.0;
  return static_cast<double>(distanciaLevenshtein(a, b)) /
         static_cast<double>(maxima);
}
