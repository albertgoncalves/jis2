#ifndef PRELUDE_HPP
#define PRELUDE_HPP

#include <cstdint>

#define STATIC_ASSERT(condition) _Static_assert(condition, "!(" #condition ")")

typedef std::size_t usize;
typedef int64_t     i64;

#endif
