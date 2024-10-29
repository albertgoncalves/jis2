#ifndef PRELUDE_HPP
#define PRELUDE_HPP

#include <cstdint>

#define STATIC_ASSERT(condition) _Static_assert(condition, "!(" #condition ")")

typedef std::size_t usize;
typedef uint8_t     u8;
typedef int64_t     i64;

STATIC_ASSERT(sizeof(usize) == 8);
STATIC_ASSERT(sizeof(u8) == 1);
STATIC_ASSERT(sizeof(i64) == 8);

#endif
