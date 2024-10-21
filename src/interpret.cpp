#include "interpret.hpp"

#include <cassert>
#include <unordered_map>

template <typename T>
static T pop(std::vector<T>& vec) {
    const usize n = vec.size();
    assert(n != 0);
    const T t = vec[n - 1];
    vec.resize(n - 1);
    return t;
}

namespace interpret {

void resolve_labels(std::vector<inst::Inst>& insts) {
    std::unordered_map<std::string_view, usize> labels;

    const usize n = insts.size();
    for (usize i = 0; i < n; ++i) {
        if (insts[i].type != inst::LABEL) {
            continue;
        }
        std::string_view label = std::string_view(insts[i].op.as_string);
        assert(label[label.size() - 1] == ':');
        labels[label.substr(0, label.size() - 1)] = i + 1;
    }
    for (usize i = 0; i < n; ++i) {
        switch (insts[i].type) {
        case inst::JUMP:
        case inst::JZ:
        case inst::PUSH_LABEL: {
            auto key_value = labels.find(insts[i].op.as_string);
            assert(key_value != nullptr);
            insts[i].op.as_usize = key_value->second;
            break;
        }

        case inst::HALT:
        case inst::LABEL:
        case inst::RET:
        case inst::DUP:
        case inst::SWAP:
        case inst::DROP:
        case inst::PUSH_INT:
        case inst::GE:
        case inst::ADD: {
            break;
        }

        default: {
            assert(0);
        }
        }
    }
}

std::vector<inst::Op> interpret(const std::vector<inst::Inst>& insts) {
    std::vector<inst::Op> stack;

    usize pc = 0;
    for (;;) {
        const inst::Inst inst = insts[pc];

        switch (inst.type) {
        case inst::HALT: {
            return stack;
        }
        case inst::PUSH_INT:
        case inst::PUSH_LABEL: {
            stack.push_back(inst.op);
            ++pc;
            break;
        }
        case inst::LABEL: {
            ++pc;
            break;
        }
        case inst::JUMP: {
            pc = inst.op.as_usize;
            break;
        }
        case inst::JZ: {
            pc = pop(stack).as_bool == 0 ? inst.op.as_usize : pc + 1;
            break;
        }
        case inst::RET: {
            pc = pop(stack).as_usize;
            break;
        }
        case inst::DUP: {
            stack.push_back(stack[stack.size() - (inst.op.as_usize + 1)]);
            ++pc;
            break;
        }
        case inst::SWAP: {
            const usize n = stack.size();

            const usize i = n - 1;
            const usize j = n - (inst.op.as_usize + 1);

            assert(i != j);

            const inst::Op a = stack[i];
            const inst::Op b = stack[j];

            stack[i] = b;
            stack[j] = a;

            ++pc;
            break;
        }
        case inst::DROP: {
            const usize n = stack.size();
            assert(inst.op.as_usize <= n);
            stack.resize(n - inst.op.as_usize);

            ++pc;
            break;
        }
        case inst::GE: {
            const usize n = stack.size();
            assert(2 <= n);
            stack[n - 2].as_bool = stack[n - 2].as_i64 >= stack[n - 1].as_i64;
            stack.resize(n - 1);

            ++pc;
            break;
        }
        case inst::ADD: {
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

} // namespace interpret
