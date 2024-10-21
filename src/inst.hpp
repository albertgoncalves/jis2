#ifndef INST_HPP
#define INST_HPP

#include "prelude.hpp"

#include <iostream>
#include <vector>

namespace inst {

enum InstType {
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

union InstOp {
    usize       as_usize;
    i64         as_i64;
    bool        as_bool;
    const char* as_string;
    void*       as_pointer;
};

STATIC_ASSERT(sizeof(void*) == sizeof(InstOp));

struct Inst {
    InstType type;
    InstOp   op;
};

std::ostream& operator<<(std::ostream&, InstOp&);
std::ostream& operator<<(std::ostream&, Inst&);

Inst inst_halt();
Inst inst_label(const char*);
Inst inst_jump(const char*);
Inst inst_jz(const char*);
Inst inst_ret();
Inst inst_dup(usize);
Inst inst_swap(usize);
Inst inst_drop(usize);
Inst inst_push_integer(i64);
Inst inst_push_label(const char*);
Inst inst_ge();
Inst inst_add();

} // namespace inst

#endif
