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
        case inst::EQ:
        case inst::GE:
        case inst::ADD: {
            break;
        }

        case inst::GUARD_0:
        case inst::GUARD_1:
        case inst::GUARD_RET:
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
        case inst::PUSH_INT:
        case inst::PUSH_LABEL: {
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
        case inst::EQ: {
            const usize n = stack.size();
            assert(2 <= n);
            stack[n - 2].as_bool = stack[n - 2].as_i64 == stack[n - 1].as_i64;
            stack.resize(n - 1);
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
        case inst::GUARD_0: {
            if (!pop(stack).as_bool) {
                return inst.op.as_usize;
            }
            break;
        }
        case inst::GUARD_1: {
            if (pop(stack).as_bool) {
                return inst.op.as_usize;
            }
            break;
        }
        case inst::GUARD_RET: {
            const usize ret_pc = pop(stack).as_usize;
            if (inst.op.as_usize != ret_pc) {
                return ret_pc;
            }
            break;
        }
        case inst::HALT:
        case inst::LABEL:
        case inst::JUMP:
        case inst::JZ:
        case inst::RET:
        default: {
            std::cerr << inst << std::endl;
            assert(0);
        }
        }
    }
}

#define MAX_TRACE_SIZE   100
#define TARGET_THRESHOLD 5

std::vector<inst::Op> interpret(const std::vector<inst::Inst>& insts, bool can_trace) {
    std::vector<inst::Op> stack;

    std::unordered_map<usize, usize>                   jump_targets;
    std::unordered_map<usize, std::vector<inst::Inst>> traces;

    bool                     tracing = false;
    usize                    trace_start = insts.size();
    std::vector<inst::Inst>* current_trace = nullptr;

    for (usize pc = 0;;) {
        if (MAX_TRACE_SIZE <= traces[pc].size()) {
            current_trace->clear();
            current_trace = nullptr;
            tracing = false;
        } else if (can_trace && (!tracing) && (TARGET_THRESHOLD <= jump_targets[pc]) &&
                   (traces[pc].size() == 0))
        {
            tracing = true;
            trace_start = pc;
            current_trace = &traces[trace_start];
            current_trace->clear();
        } else if (tracing && (trace_start == pc)) {
            std::cout << trace_start << ": [\n";
            for (usize i = 0; i < current_trace->size(); ++i) {
                std::cout << "    " << (*current_trace)[i] << '\n';
            }
            std::cout << "]\n\n";

            current_trace = nullptr;
            tracing = false;
            trace_start = insts.size();
        }

        if ((!tracing) && (traces[pc].size() != 0)) {
            pc = run_trace(traces[pc], stack);
            ++jump_targets[pc];
            continue;
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
                current_trace->push_back(inst);
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
            if (condition) {
                const usize branch_pc = inst.op.as_usize;
                ++pc;

                if (tracing) {
                    current_trace->push_back({
                        .type = inst::GUARD_0,
                        .op = {.as_usize = branch_pc},
                    });
                }
            } else {
                const usize branch_pc = pc + 1;
                pc = inst.op.as_usize;

                if (tracing) {
                    current_trace->push_back({
                        .type = inst::GUARD_1,
                        .op = {.as_usize = branch_pc},
                    });
                }
            }
            break;
        }
        case inst::RET: {
            pc = pop(stack).as_usize;
            if (tracing) {
                current_trace->push_back({.type = inst::GUARD_RET, .op = {.as_usize = pc}});
            }
            break;
        }
        case inst::DUP: {
            if (tracing) {
                current_trace->push_back(inst);
            }
            stack.push_back(stack[stack.size() - (inst.op.as_usize + 1)]);
            ++pc;
            break;
        }
        case inst::SWAP: {
            if (tracing) {
                current_trace->push_back(inst);
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
                current_trace->push_back(inst);
            }
            const usize n = stack.size();
            assert(inst.op.as_usize <= n);
            stack.resize(n - inst.op.as_usize);

            ++pc;
            break;
        }
        case inst::EQ: {
            if (tracing) {
                current_trace->push_back(inst);
            }
            const usize n = stack.size();
            assert(2 <= n);
            stack[n - 2].as_bool = stack[n - 2].as_i64 == stack[n - 1].as_i64;
            stack.resize(n - 1);

            ++pc;
            break;
        }
        case inst::GE: {
            if (tracing) {
                current_trace->push_back(inst);
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
                current_trace->push_back(inst);
            }
            const usize n = stack.size();
            assert(2 <= n);
            stack[n - 2].as_i64 += stack[n - 1].as_i64;
            stack.resize(n - 1);

            ++pc;
            break;
        }
        case inst::GUARD_0:
        case inst::GUARD_1:
        case inst::GUARD_RET:
        default: {
            assert(0);
        }
        }
    }
}

} // namespace interpret
