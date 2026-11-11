#include "text_utils.hpp"
#include <iostream>

using namespace std;

string recortar(const string& texto) {
	size_t inicio = texto.find_first_not_of(" \t\r\n");
	if (inicio == string::npos) return "";

	size_t fin = texto.find_last_not_of(" \t\r\n");
	return texto.substr(inicio, fin - inicio + 1);
}

string aMinusculasEspanolUtf8Basico(const string& texto) {
	string resultado = texto;

	for (char& caracter : resultado) {
		if (caracter >= 'A' && caracter <= 'Z') {
			caracter = char(caracter - 'A' + 'a');
		}
	}

	for (size_t indice = 0; indice + 1 < resultado.size(); ++indice) {
		unsigned char a = static_cast<unsigned char>(resultado[indice]);
		unsigned char b = static_cast<unsigned char>(resultado[indice + 1]);

		if (a == 0xC3) {
			switch (b) {
				case 0x81: resultado[indice + 1] = char(0xA1); break;
				case 0x89: resultado[indice + 1] = char(0xA9); break;
				case 0x8D: resultado[indice + 1] = char(0xAD); break;
				case 0x93: resultado[indice + 1] = char(0xB3); break;
				case 0x9A: resultado[indice + 1] = char(0xBA); break;
				case 0x9C: resultado[indice + 1] = char(0xBC); break;
				case 0x91: resultado[indice + 1] = char(0xB1); break;
				default: break;
			}
		}
	}

	return resultado;
}

u32string utf8ACodepoints(const string& texto) {
	u32string resultado;

	for (size_t indice = 0; indice < texto.size();) {
		unsigned char caracter = static_cast<unsigned char>(texto[indice]);

		if (caracter < 0x80) {
			resultado.push_back(caracter);
			indice += 1;
		} else if ((caracter >> 5) == 0x6 && indice + 1 < texto.size()) {
			char32_t codigo = ((caracter & 0x1F) << 6) |
							  (static_cast<unsigned char>(texto[indice + 1]) & 0x3F);
			resultado.push_back(codigo);
			indice += 2;
		} else if ((caracter >> 4) == 0xE && indice + 2 < texto.size()) {
			char32_t codigo = ((caracter & 0x0F) << 12) |
							  ((static_cast<unsigned char>(texto[indice + 1]) & 0x3F) << 6) |
							  (static_cast<unsigned char>(texto[indice + 2]) & 0x3F);
			resultado.push_back(codigo);
			indice += 3;
		} else if ((caracter >> 3) == 0x1E && indice + 3 < texto.size()) {
			char32_t codigo = ((caracter & 0x07) << 18) |
							  ((static_cast<unsigned char>(texto[indice + 1]) & 0x3F) << 12) |
							  ((static_cast<unsigned char>(texto[indice + 2]) & 0x3F) << 6) |
							  (static_cast<unsigned char>(texto[indice + 3]) & 0x3F);
			resultado.push_back(codigo);
			indice += 4;
		} else {
			indice += 1;
		}
	}

	return resultado;
}

string codepointsAUtf8(const u32string &texto) {
  string r;
  for (char32_t c : texto) {
    if (c < 0x80) {
      r.push_back(static_cast<char>(c));
    } else if (c < 0x800) {
      r.push_back(static_cast<char>(0xC0 | (c >> 6)));
      r.push_back(static_cast<char>(0x80 | (c & 0x3F)));
    } else if (c < 0x10000) {
      r.push_back(static_cast<char>(0xE0 | (c >> 12)));
      r.push_back(static_cast<char>(0x80 | ((c >> 6) & 0x3F)));
      r.push_back(static_cast<char>(0x80 | (c & 0x3F)));
    } else {
      r.push_back(static_cast<char>(0xF0 | (c >> 18)));
      r.push_back(static_cast<char>(0x80 | ((c >> 12) & 0x3F)));
      r.push_back(static_cast<char>(0x80 | ((c >> 6) & 0x3F)));
      r.push_back(static_cast<char>(0x80 | (c & 0x3F)));
    }
  }
  return r;
}
