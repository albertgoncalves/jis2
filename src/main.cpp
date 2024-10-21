#include <cassert>
#include <cstdint>
#include <fstream>
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

static std::ostream& operator<<(std::ostream& stream, Inst& inst) {
    switch (inst.type) {
    case INST_HALT: {
        stream << "halt";
        break;
    }
    case INST_LABEL: {
        stream << inst.op.as_string;
        break;
    }
    case INST_JUMP: {
        stream << "jump " << inst.op.as_usize;
        break;
    }
    case INST_JZ: {
        stream << "jz " << inst.op.as_usize;
        break;
    }
    case INST_RET: {
        stream << "ret";
        break;
    }
    case INST_DUP: {
        stream << "dup " << inst.op.as_usize;
        break;
    }
    case INST_SWAP: {
        stream << "swap " << inst.op.as_usize;
        break;
    }
    case INST_DROP: {
        stream << "drop " << inst.op.as_usize;
        break;
    }
    case INST_PUSH_INT: {
        stream << "push " << inst.op.as_i64;
        break;
    }
    case INST_PUSH_LABEL: {
        stream << "push " << inst.op.as_usize;
        break;
    }
    case INST_GE: {
        stream << "ge";
        break;
    }
    case INST_ADD: {
        stream << "add";
        break;
    }
    default: {
        assert(0);
    }
    }
    return stream;
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

static void resolve_labels(std::vector<Inst>& insts) {
    std::unordered_map<std::string_view, usize> labels;

    const usize n = insts.size();
    for (usize i = 0; i < n; ++i) {
        if (insts[i].type != INST_LABEL) {
            continue;
        }
        std::string_view label = std::string_view(insts[i].op.as_string);
        assert(label[label.size() - 1] == ':');
        labels[label.substr(0, label.size() - 1)] = i + 1;
    }
    for (usize i = 0; i < n; ++i) {
        switch (insts[i].type) {
        case INST_JUMP:
        case INST_JZ:
        case INST_PUSH_LABEL: {
            auto key_value = labels.find(insts[i].op.as_string);
            assert(key_value != nullptr);
            insts[i].op.as_usize = key_value->second;
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

static std::vector<InstOp> interpret(const std::vector<Inst>& insts) {
    std::vector<InstOp> stack;

    usize pc = 0;
    for (;;) {
        const Inst inst = insts[pc];

        switch (inst.type) {
        case INST_HALT: {
            return stack;
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

static std::string string_from_path(const char* path) {
    std::ifstream file(path);
    assert(file);
    const std::string buffer((std::istreambuf_iterator<char>(file)),
                             std::istreambuf_iterator<char>());
    return buffer;
}

static std::vector<std::string> tokenize(const std::string_view& source) {
    std::vector<std::string> tokens;

    const usize n = source.size();
    for (usize i = 0;;) {
        i = source.find_first_not_of(" \t\r\n", i);
        if (n <= i) {
            break;
        }
        if (source[i] == ';') {
            i = source.find_first_of('\n', i);
            continue;
        }
        usize j = source.find_first_of(" \t\r\n;", i);
        assert(j <= n);
        assert(i != j);
        tokens.push_back(std::string(source.substr(i, j - i)));
        i = j;
    }

    return tokens;
}

static std::vector<Inst> parse(std::vector<std::string>& tokens) {
    std::vector<Inst> insts;

    const usize n = tokens.size();
    for (usize i = 0; i < n; ++i) {
        if (tokens[i] == std::string_view("halt")) {
            insts.push_back(inst_halt());
            continue;
        }
        if (tokens[i] == std::string_view("ret")) {
            insts.push_back(inst_ret());
            continue;
        }
        if (tokens[i] == std::string_view("ge")) {
            insts.push_back(inst_ge());
            continue;
        }
        if (tokens[i] == std::string_view("add")) {
            insts.push_back(inst_add());
            continue;
        }
        if (tokens[i][tokens[i].size() - 1] == ':') {
            insts.push_back(inst_label(tokens[i].c_str()));
            continue;
        }

        usize j = i + 1;
        assert(j < n);

        if (tokens[i] == std::string_view("jump")) {
            insts.push_back(inst_jump(tokens[j].c_str()));
            i = j;
            continue;
        }
        if (tokens[i] == std::string_view("jz")) {
            insts.push_back(inst_jz(tokens[j].c_str()));
            i = j;
            continue;
        }
        if (tokens[i] == std::string_view("dup")) {
            insts.push_back(inst_dup(stoull(tokens[j])));
            i = j;
            continue;
        }
        if (tokens[i] == std::string_view("swap")) {
            insts.push_back(inst_swap(stoull(tokens[j])));
            i = j;
            continue;
        }
        if (tokens[i] == std::string_view("drop")) {
            insts.push_back(inst_drop(stoull(tokens[j])));
            i = j;
            continue;
        }
        if (tokens[i] == std::string_view("push")) {
            try {
                insts.push_back(inst_push_integer(stoll(tokens[j])));
                i = j;
                continue;
            } catch (const std::invalid_argument& _) {
                insts.push_back(inst_push_label(tokens[j].c_str()));
                i = j;
                continue;
            }
            assert(0);
        }

        assert(0);
    }

    return insts;
}

int main(int argc, const char** argv) {
    assert(1 < argc);

    const std::string        source = string_from_path(argv[1]);
    std::vector<std::string> tokens = tokenize(source);
    std::vector<Inst>        insts = parse(tokens);
    resolve_labels(insts);
    std::vector<InstOp> stack = interpret(insts);

    std::cout << insts << std::endl << stack << std::endl;
    return 0;
}
