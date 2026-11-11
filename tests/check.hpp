#ifndef BKTREE_TESTS_CHECK_HPP
#define BKTREE_TESTS_CHECK_HPP

#include <cstdio>
#include <cstdlib>
#include <iostream>

using namespace std;

inline int &fallos() { static int n = 0; return n; }

#define CHECK(cond)                                                      \
  do {                                                                   \
    if (!(cond)) {                                                       \
      printf("FALLO %s:%d: %s\n", __FILE__, __LINE__, #cond);       \
      ++fallos();                                                        \
    }                                                                    \
  } while (0)

inline int resumen(const char *nombre) {
  if (fallos() == 0) { printf("[OK] %s\n", nombre); return 0; }
  printf("[FALLOS: %d] %s\n", fallos(), nombre);
  return 1;
}

#endif
