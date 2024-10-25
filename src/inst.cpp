#include "inst.hpp"

#include <cassert>

namespace inst {

std::ostream& operator<<(std::ostream& stream, const Op& op) {
    stream << op.as_i64;
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const Inst& inst) {
    switch (inst.type) {
    case HALT: {
        stream << "halt";
        break;
    }
    case LABEL: {
        stream << inst.op.as_string;
        break;
    }
    case JUMP: {
        stream << "jump " << inst.op.as_usize;
        break;
    }
    case JZ: {
        stream << "jz " << inst.op.as_usize;
        break;
    }
    case RET: {
        stream << "ret";
        break;
    }
    case DUP: {
        stream << "dup " << inst.op.as_usize;
        break;
    }
    case SWAP: {
        stream << "swap " << inst.op.as_usize;
        break;
    }
    case DROP: {
        stream << "drop " << inst.op.as_usize;
        break;
    }
    case PUSH_INT: {
        stream << "push-int " << inst.op.as_i64;
        break;
    }
    case PUSH_LABEL: {
        stream << "push-label " << inst.op.as_usize;
        break;
    }
    case EQ: {
        stream << "eq";
        break;
    }
    case GE: {
        stream << "ge";
        break;
    }
    case ADD: {
        stream << "add";
        break;
    }
    case GUARD_0: {
        stream << "guard-false " << inst.op.as_usize;
        break;
    }
    case GUARD_1: {
        stream << "guard-true " << inst.op.as_usize;
        break;
    }
    case GUARD_RET: {
        stream << "guard-ret " << inst.op.as_usize;
        break;
    }
    default: {
        assert(0);
    }
    }
    return stream;
}

Inst halt() {
    return (Inst){.type = HALT, .op = {}};
}

Inst label(const char* label) {
    return (Inst){.type = LABEL, .op = {.as_string = label}};
}

Inst jump(const char* label) {
    return (Inst){.type = JUMP, .op = {.as_string = label}};
}

Inst jz(const char* label) {
    return (Inst){.type = JZ, .op = {.as_string = label}};
}

Inst ret() {
    return (Inst){.type = RET, .op = {}};
}

Inst dup(usize offset) {
    return (Inst){.type = DUP, .op = {.as_usize = offset}};
}

Inst swap(usize offset) {
    return (Inst){.type = SWAP, .op = {.as_usize = offset}};
}

Inst drop(usize offset) {
    return (Inst){.type = DROP, .op = {.as_usize = offset}};
}

Inst push_integer(i64 integer) {
    return (Inst){.type = PUSH_INT, .op = {.as_i64 = integer}};
}

Inst push_label(const char* label) {
    return (Inst){.type = PUSH_LABEL, .op = {.as_string = label}};
}

Inst eq() {
    return (Inst){.type = EQ, .op = {}};
}

Inst ge() {
    return (Inst){.type = GE, .op = {}};
}

Inst add() {
    return (Inst){.type = ADD, .op = {}};
}

} // namespace inst
