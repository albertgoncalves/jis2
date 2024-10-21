#ifndef INST_HPP
#define INST_HPP

#include "prelude.hpp"

#include <iostream>

namespace inst {

enum Type {
    HALT = 0,

    LABEL,

    JUMP,
    JZ,

    RET,

    DUP,
    SWAP,
    DROP,

    PUSH_INT,
    PUSH_LABEL,

    GE,

    ADD,
};

union Op {
    usize       as_usize;
    i64         as_i64;
    bool        as_bool;
    const char* as_string;
    void*       as_pointer;
};

STATIC_ASSERT(sizeof(void*) == sizeof(Op));

struct Inst {
    Type type;
    Op   op;
};

std::ostream& operator<<(std::ostream&, Op&);
std::ostream& operator<<(std::ostream&, Inst&);

Inst halt();
Inst label(const char*);
Inst jump(const char*);
Inst jz(const char*);
Inst ret();
Inst dup(usize);
Inst swap(usize);
Inst drop(usize);
Inst push_integer(i64);
Inst push_label(const char*);
Inst ge();
Inst add();

} // namespace inst

#endif
