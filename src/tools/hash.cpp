/* The MIT License (MIT)
 *
 * Copyright (c) 2014-2016 David Medina and Tim Warburton
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 */

#include <sstream>
#include <stdint.h>

#include "occa/types.hpp"
#include "occa/tools/hash.hpp"
#include "occa/tools/env.hpp"
#include "occa/tools/io.hpp"

namespace occa {
  hash_t::hash_t() {
    initialized = false;
    h[0] = 101527; h[1] = 101531;
    h[2] = 101533; h[3] = 101537;
    h[4] = 101561; h[5] = 101573;
    h[6] = 101581; h[7] = 101599;
    for (int i = 0; i < 8; ++i) {
      sh[i] = 0;
    }
  }

  hash_t::hash_t(const int *h_) {
    initialized = true;
    for (int i = 0; i < 8; ++i) {
      h[i] = h_[i];
    }
    for (int i = 0; i < 8; ++i) {
      sh[i] = 0;
    }
  }

  hash_t::hash_t(const hash_t &hash) {
    *this = hash;
  }

  hash_t& hash_t::operator = (const hash_t &hash) {
    initialized = hash.initialized;
    for (int i = 0; i < 8; ++i) {
      h[i] = hash.h[i];
    }
    for (int i = 0; i < 8; ++i) {
      sh[i] = 0;
    }
    return *this;
  }

  bool hash_t::operator < (const hash_t &fo) const {
    for (int i = 0; i < 8; ++i) {
      if (h[i] < fo.h[i]) {
        return true;
      } else if (h[i] > fo.h[i]) {
        return false;
      }
    }
    return true;
  }

  bool hash_t::operator == (const hash_t &fo) const {
    for (int i = 0; i < 8; ++i) {
      if (h[i] != fo.h[i]) {
        return false;
      }
    }
    return true;
  }

  bool hash_t::operator != (const hash_t &fo) const {
    for (int i = 0; i < 8; ++i) {
      if (h[i] != fo.h[i]) {
        return true;
      }
    }
    return false;
  }

  hash_t hash_t::operator ^ (const hash_t hash) const {
    hash_t mix;
    for (int i = 0; i < 8; ++i) {
      mix.h[i] = (h[i] ^ hash.h[i]);
    }
    mix.initialized = true;
    return mix;
  }

  hash_t& hash_t::operator ^= (const hash_t hash) {
    *this = (*this ^ hash);
    return *this;
  }

  std::string hash_t::const_toString() const {
    std::stringstream ss;
    for (int i = 0; i < 8; ++i) {
      ss << std::hex << h[i];
    }
    std::string s = ss.str();
    return (s.size() < 16) ? s : s.substr(0, 16);
  }

  std::string hash_t::toString() {
    if (*this != hash_t(sh)) {
      h_string = const_toString();
    }
    return h_string;
  }

  std::string hash_t::toString() const {
    if (*this != hash_t(sh)) {
      return const_toString();
    }
    return h_string;
  }

  hash_t::operator std::string () const {
    return toString();
  }

  std::ostream& operator << (std::ostream &out, const hash_t &hash) {
    out << hash.toString();
    return out;
  }

  hash_t hash(const void *ptr, udim_t bytes) {
    std::stringstream ss;
    const char *c = (char*) ptr;

    hash_t hash;
    int *h = hash.h;

    const int p[8] = {
      102679, 102701, 102761, 102763,
      102769, 102793, 102797, 102811
    };

    for (udim_t i = 0; i < bytes; ++i) {
      for (int j = 0; j < 8; ++j) {
        h[j] = (h[j] * p[j]) ^ c[i];
      }
    }
    hash.initialized = true;

    return hash;
  }

  hash_t hash(const char *c) {
    return hash(c, strlen(c));
  }

  hash_t hash(const std::string &str) {
    return hash(str.c_str(), str.size());
  }

  hash_t hashFile(const std::string &filename) {
    const char *c = io::c_read(filename);
    hash_t ret = hash(c);
    ::free((void*) c);
    return ret;
  }
}