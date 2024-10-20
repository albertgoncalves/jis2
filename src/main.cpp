#include <cassert>
#include <cstdint>
#include <iostream>
#include <string_view>
#include <unordered_map>
#include <vector>

typedef size_t  usize;
typedef int64_t i64;

#define STATIC_ASSERT(condition) _Static_assert(condition, "!(" #condition ")")

enum InstType {
    INST_HALT = 0,

    INST_LABEL,

    INST_JUMP,
    INST_JZ,

    INST_RET,

    INST_DUP,
    INST_SWAP,
    INST_DROP,

    INST_PUSH_INT,
    INST_PUSH_LABEL,

    INST_GE,

    INST_ADD,
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

template <typename T>
static std::ostream& operator<<(std::ostream& stream, std::vector<T>& vec) {
    const usize n = vec.size();
    if (n == 0) {
        stream << "[]";
        return stream;
    }

    stream << '[' << vec[0];
    for (usize i = 1; i < n; ++i) {
        stream << ", " << vec[i];
    }
    stream << ']';
    return stream;
}

static std::ostream& operator<<(std::ostream& stream, InstOp& op) {
    stream << op.as_i64;
    return stream;
}

template <typename T>
static T pop(std::vector<T>& vec) {
    const usize n = vec.size();
    assert(n != 0);
    const T t = vec[n - 1];
    vec.resize(n - 1);
    return t;
}

static Inst inst_halt() {
    return (Inst){.type = INST_HALT, .op = {}};
}

static Inst inst_label(const char* label) {
    return (Inst){.type = INST_LABEL, .op = {.as_string = label}};
}

static Inst inst_jump(const char* label) {
    return (Inst){.type = INST_JUMP, .op = {.as_string = label}};
}

static Inst inst_jz(const char* label) {
    return (Inst){.type = INST_JZ, .op = {.as_string = label}};
}

static Inst inst_ret() {
    return (Inst){.type = INST_RET, .op = {}};
}

static Inst inst_dup(usize offset) {
    return (Inst){.type = INST_DUP, .op = {.as_usize = offset}};
}

static Inst inst_swap(usize offset) {
    return (Inst){.type = INST_SWAP, .op = {.as_usize = offset}};
}

static Inst inst_drop(usize offset) {
    return (Inst){.type = INST_DROP, .op = {.as_usize = offset}};
}

static Inst inst_push_integer(i64 integer) {
    return (Inst){.type = INST_PUSH_INT, .op = {.as_i64 = integer}};
}

static Inst inst_push_label(const char* label) {
    return (Inst){.type = INST_PUSH_LABEL, .op = {.as_string = label}};
}

static Inst inst_ge() {
    return (Inst){.type = INST_GE, .op = {}};
}

static Inst inst_add() {
    return (Inst){.type = INST_ADD, .op = {}};
}

static void resolve_labels(Inst* insts, usize insts_len) {
    std::unordered_map<std::string_view, usize> labels;

    for (usize i = 0; i < insts_len; ++i) {
        if (insts[i].type != INST_LABEL) {
            continue;
        }
        labels[insts[i].op.as_string] = i + 1;
    }
    for (usize i = 0; i < insts_len; ++i) {
        switch (insts[i].type) {
        case INST_JUMP:
        case INST_JZ:
        case INST_PUSH_LABEL: {
            insts[i].op.as_usize = labels[insts[i].op.as_string];
            break;
        }

        case INST_HALT:
        case INST_LABEL:
        case INST_RET:
        case INST_DUP:
        case INST_SWAP:
        case INST_DROP:
        case INST_PUSH_INT:
        case INST_GE:
        case INST_ADD: {
            break;
        }

        default: {
            assert(0);
        }
        }
    }
}

static void interpret(Inst* insts, std::vector<InstOp>& stack) {
    usize pc = 0;
    for (;;) {
        const Inst inst = insts[pc];

        switch (inst.type) {
        case INST_HALT: {
            return;
        }
        case INST_PUSH_INT:
        case INST_PUSH_LABEL: {
            stack.push_back(inst.op);
            ++pc;
            break;
        }
        case INST_LABEL: {
            ++pc;
            break;
        }
        case INST_JUMP: {
            pc = inst.op.as_usize;
            break;
        }
        case INST_JZ: {
            pc = pop(stack).as_bool == 0 ? inst.op.as_usize : pc + 1;
            break;
        }
        case INST_RET: {
            pc = pop(stack).as_usize;
            break;
        }
        case INST_DUP: {
            stack.push_back(stack[stack.size() - (inst.op.as_usize + 1)]);
            ++pc;
            break;
        }
        case INST_SWAP: {
            const usize n = stack.size();

            const usize i = n - 1;
            const usize j = n - (inst.op.as_usize + 1);

            assert(i != j);

            const InstOp a = stack[i];
            const InstOp b = stack[j];

            stack[i] = b;
            stack[j] = a;

            ++pc;
            break;
        }
        case INST_DROP: {
            const usize n = stack.size();
            assert(inst.op.as_usize <= n);
            stack.resize(n - inst.op.as_usize);

            ++pc;
            break;
        }
        case INST_GE: {
            const usize n = stack.size();
            assert(2 <= n);
            stack[n - 2].as_bool = stack[n - 2].as_i64 >= stack[n - 1].as_i64;
            stack.resize(n - 1);

            ++pc;
            break;
        }
        case INST_ADD: {
            const usize n = stack.size();
            assert(2 <= n);
            stack[n - 2].as_i64 += stack[n - 1].as_i64;
            stack.resize(n - 1);

            ++pc;
            break;
        }
        default: {
            assert(0);
        }
        }
    }
}

/*
    int sum_1_to_n(int n) {
        int x = 0;
        while (n >= 0) {
            x += n;
            n -= 1;
        }
        return x;
    }
*/

/*
            push    .return         ; .return                   0
            push    100             ; .return, 100              1
            jump    sum_1_to_n      ; .return, 100              2
        .return
            halt                    ; sum_1_to_n(100)           3

    sum_1_to_n:
            push    0               ; .return, n, x             4

        .while_start
            dup     1               ; .return, n, x, n          5
            push    0               ; .return, n, x, n, 0       6
            ge                      ; .return, n, x, n>=0       7
            jz      .while_end      ; .return, n, x             8

            dup     1               ; .return, n, x, n          9
            add                     ; .return, n, x+n          10
            swap    1               ; .return, x+n, n          11
            push    -1              ; .return, x+n, n, -1      12
            add                     ; .return, x+n, n-1        13
            swap    1               ; .return, n-1, x+n        14

            jump    .while_start                               15
        .while_end

            swap    1               ; .return, x+n, n-1        16
            drop    1               ; .return, x+n             17
            swap    1               ; x+n, .return             18
            ret                                                19
*/

int main() {
    Inst insts[] = {
        inst_push_label(".return"),
        inst_push_integer(100),
        inst_jump("sum_1_to_n"),
        inst_label(".return"),
        inst_halt(),
        inst_label("sum_1_to_n"),
        inst_push_integer(0),
        inst_label(".while_start"),
        inst_dup(1),
        inst_push_integer(0),
        inst_ge(),
        inst_jz(".while_end"),
        inst_dup(1),
        inst_add(),
        inst_swap(1),
        inst_push_integer(-1),
        inst_add(),
        inst_swap(1),
        inst_jump(".while_start"),
        inst_label(".while_end"),
        inst_swap(1),
        inst_drop(1),
        inst_swap(1),
        inst_ret(),
    };
    std::vector<InstOp> stack;

    resolve_labels(insts, sizeof(insts) / sizeof(Inst));
    interpret(insts, stack);

    std::cout << stack << std::endl;
    return 0;
}
