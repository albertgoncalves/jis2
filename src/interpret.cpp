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
        std::string_view key = label.substr(0, label.size() - 1);
        assert(labels.find(key) == nullptr);
        labels[key] = i + 1;
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

        case inst::GUARD0:
        case inst::GUARD1:
        default: {
            assert(0);
        }
        }
    }
}

static usize run_trace(const std::vector<inst::Inst>& trace, std::vector<inst::Op>& stack) {
    const usize m = trace.size();
    for (usize pc = 0;; pc = (pc + 1) % m) {
        const inst::Inst inst = trace[pc];
        switch (inst.type) {
        case inst::PUSH_INT: {
            stack.push_back(inst.op);
            break;
        }
        case inst::DUP: {
            stack.push_back(stack[stack.size() - (inst.op.as_usize + 1)]);
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
            break;
        }
        case inst::DROP: {
            const usize n = stack.size();
            assert(inst.op.as_usize <= n);
            stack.resize(n - inst.op.as_usize);
            break;
        }
        case inst::GE: {
            const usize n = stack.size();
            assert(2 <= n);
            stack[n - 2].as_bool = stack[n - 2].as_i64 >= stack[n - 1].as_i64;
            stack.resize(n - 1);
            break;
        }
        case inst::ADD: {
            const usize n = stack.size();
            assert(2 <= n);
            stack[n - 2].as_i64 += stack[n - 1].as_i64;
            stack.resize(n - 1);
            break;
        }
        case inst::GUARD0: {
            if (!pop(stack).as_bool) {
                return inst.op.as_usize;
            }
            break;
        }
        case inst::GUARD1: {
            if (pop(stack).as_bool) {
                return inst.op.as_usize;
            }
            break;
        }
        case inst::HALT:
        case inst::LABEL:
        case inst::JUMP:
        case inst::JZ:
        case inst::RET:
        case inst::PUSH_LABEL:
        default: {
            assert(0);
        }
        }
    }
}

std::vector<inst::Op> interpret(const std::vector<inst::Inst>& insts) {
    std::vector<inst::Op> stack;

    std::unordered_map<usize, usize> jump_targets;

    bool  can_trace = true;
    bool  tracing = false;
    usize trace_start = insts.size();

    std::vector<inst::Inst> trace;

    usize pc = 0;
    for (;;) {
        if (can_trace && (!tracing) && (10 < jump_targets[pc])) {
            can_trace = false;
            tracing = true;
            trace_start = pc;
        } else if (tracing && (pc == trace_start)) {
            tracing = false;

            std::cout << trace_start << ": [\n";
            for (usize i = 0; i < trace.size(); ++i) {
                std::cout << "    " << trace[i] << '\n';
            }
            std::cout << "]\n\n";

            pc = run_trace(trace, stack);
        }

        const inst::Inst inst = insts[pc];

        switch (inst.type) {
        case inst::HALT: {
            std::cout << "jump_targets: {\n";
            for (auto entry = jump_targets.begin(); entry != jump_targets.end(); ++entry) {
                if (entry->second == 0) {
                    continue;
                }
                std::cout << "    " << entry->first << ": " << entry->second << '\n';
            }
            std::cout << "}\n";
            return stack;
        }
        case inst::PUSH_INT:
        case inst::PUSH_LABEL: {
            if (tracing) {
                trace.push_back(inst);
            }
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
            ++jump_targets[pc];
            break;
        }
        case inst::JZ: {
            const bool condition = pop(stack).as_bool;
            if (condition == false) {
                pc = inst.op.as_usize;
                ++jump_targets[pc];

                if (tracing) {
                    trace.push_back({.type = inst::GUARD0, .op = {.as_usize = pc}});
                }
            } else {
                ++pc;

                if (tracing) {
                    trace.push_back({.type = inst::GUARD1, .op = {.as_usize = pc}});
                }
            }
            break;
        }
        case inst::RET: {
            if (tracing) {
                trace.push_back({.type = inst::DROP, .op = {.as_usize = 1}});
            }
            pc = pop(stack).as_usize;
            ++jump_targets[pc];
            break;
        }
        case inst::DUP: {
            if (tracing) {
                trace.push_back(inst);
            }
            stack.push_back(stack[stack.size() - (inst.op.as_usize + 1)]);
            ++pc;
            break;
        }
        case inst::SWAP: {
            if (tracing) {
                trace.push_back(inst);
            }
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
            if (tracing) {
                trace.push_back(inst);
            }
            const usize n = stack.size();
            assert(inst.op.as_usize <= n);
            stack.resize(n - inst.op.as_usize);

            ++pc;
            break;
        }
        case inst::GE: {
            if (tracing) {
                trace.push_back(inst);
            }
            const usize n = stack.size();
            assert(2 <= n);
            stack[n - 2].as_bool = stack[n - 2].as_i64 >= stack[n - 1].as_i64;
            stack.resize(n - 1);

            ++pc;
            break;
        }
        case inst::ADD: {
            if (tracing) {
                trace.push_back(inst);
            }
            const usize n = stack.size();
            assert(2 <= n);
            stack[n - 2].as_i64 += stack[n - 1].as_i64;
            stack.resize(n - 1);

            ++pc;
            break;
        }
        case inst::GUARD0:
        case inst::GUARD1:
        default: {
            assert(0);
        }
        }
    }
}

} // namespace interpret
