/*  Written in 2016-2018 by David Blackman and Sebastiano Vigna (vigna@acm.org)

To the extent possible under law, the author has dedicated all copyright
and related and neighboring rights to this software to the public domain
worldwide. This software is distributed without any warranty.

See <http://creativecommons.org/publicdomain/zero/1.0/>. */

#ifndef XOROSHIRO_H
#define XOROSHIRO_H

#include <stdint.h>

static uint64_t xoroshiro128plus(uint64_t s[2]) {
  uint64_t s0 = s[0];
  uint64_t s1 = s[1];
  uint64_t result = s0 + s1;
  s1 ^= s0;
  s[0] = ((s0 << 24) | (s0 >> 40)) ^ s1 ^ (s1 << 16);
  s[1] = (s1 << 37) | (s1 >> 27);
  return result;
}

#endif // XOROSHIRO_H
