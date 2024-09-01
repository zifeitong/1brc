#pragma once

#include <cstring>
/*
  Author: Wang Yi <godspeed_china@yeah.net>
  This is a quick and dirty hash function designed for O(1) speed.
  It makes your hash table application fly in most cases.
  It samples first, middle and last 4 bytes to produce the hash.
  Do not use it in very serious applications as it's not secure.
*/

static inline uint32_t _o1r4(const uint8_t *p) {
  uint32_t v;
  memcpy(&v, p, 4);
  return v;
}

static inline uint64_t o1hash(const void *key, size_t len) {
  const uint8_t *p = (const uint8_t *)key;
  if (len >= 4) {
    uint32_t first = _o1r4(p), middle = _o1r4(p + (len >> 1) - 2),
             last = _o1r4(p + len - 4);
    return (uint64_t)(first + last) * middle;
  }

  if (len) {
    uint64_t tail = ((((uint32_t)p[0]) << 16) | (((uint32_t)p[len >> 1]) << 8) |
                     p[len - 1]);
    return tail * 0xa0761d6478bd642full;
  }
  return 0;
}
