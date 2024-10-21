#include "inst.hpp"

#include <cassert>

namespace inst {

std::ostream& operator<<(std::ostream& stream, InstOp& op) {
    stream << op.as_i64;
    return stream;
}

std::ostream& operator<<(std::ostream& stream, Inst& inst) {
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
        stream << "push " << inst.op.as_i64;
        break;
    }
    case PUSH_LABEL: {
        stream << "push " << inst.op.as_usize;
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
    default: {
        assert(0);
    }
    }
    return stream;
}

Inst inst_halt() {
    return (Inst){.type = HALT, .op = {}};
}

Inst inst_label(const char* label) {
    return (Inst){.type = LABEL, .op = {.as_string = label}};
}

Inst inst_jump(const char* label) {
    return (Inst){.type = JUMP, .op = {.as_string = label}};
}

Inst inst_jz(const char* label) {
    return (Inst){.type = JZ, .op = {.as_string = label}};
}

Inst inst_ret() {
    return (Inst){.type = RET, .op = {}};
}

Inst inst_dup(usize offset) {
    return (Inst){.type = DUP, .op = {.as_usize = offset}};
}

Inst inst_swap(usize offset) {
    return (Inst){.type = SWAP, .op = {.as_usize = offset}};
}

Inst inst_drop(usize offset) {
    return (Inst){.type = DROP, .op = {.as_usize = offset}};
}

Inst inst_push_integer(i64 integer) {
    return (Inst){.type = PUSH_INT, .op = {.as_i64 = integer}};
}

Inst inst_push_label(const char* label) {
    return (Inst){.type = PUSH_LABEL, .op = {.as_string = label}};
}

Inst inst_ge() {
    return (Inst){.type = GE, .op = {}};
}

Inst inst_add() {
    return (Inst){.type = ADD, .op = {}};
}

} // namespace inst
