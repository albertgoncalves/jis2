#include "parse.hpp"

#include <cassert>

namespace parse {

std::vector<std::string> tokenize(const std::string_view& source) {
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

std::vector<inst::Inst> parse(std::vector<std::string>& tokens) {
    std::vector<inst::Inst> insts;

    const usize n = tokens.size();
    for (usize i = 0; i < n; ++i) {
        if (tokens[i] == std::string_view("halt")) {
            insts.push_back(inst::inst_halt());
            continue;
        }
        if (tokens[i] == std::string_view("ret")) {
            insts.push_back(inst::inst_ret());
            continue;
        }
        if (tokens[i] == std::string_view("ge")) {
            insts.push_back(inst::inst_ge());
            continue;
        }
        if (tokens[i] == std::string_view("add")) {
            insts.push_back(inst::inst_add());
            continue;
        }
        if (tokens[i][tokens[i].size() - 1] == ':') {
            insts.push_back(inst::inst_label(tokens[i].c_str()));
            continue;
        }

        usize j = i + 1;
        assert(j < n);

        if (tokens[i] == std::string_view("jump")) {
            insts.push_back(inst::inst_jump(tokens[j].c_str()));
            i = j;
            continue;
        }
        if (tokens[i] == std::string_view("jz")) {
            insts.push_back(inst::inst_jz(tokens[j].c_str()));
            i = j;
            continue;
        }
        if (tokens[i] == std::string_view("dup")) {
            insts.push_back(inst::inst_dup(stoull(tokens[j])));
            i = j;
            continue;
        }
        if (tokens[i] == std::string_view("swap")) {
            insts.push_back(inst::inst_swap(stoull(tokens[j])));
            i = j;
            continue;
        }
        if (tokens[i] == std::string_view("drop")) {
            insts.push_back(inst::inst_drop(stoull(tokens[j])));
            i = j;
            continue;
        }
        if (tokens[i] == std::string_view("push")) {
            try {
                insts.push_back(inst::inst_push_integer(stoll(tokens[j])));
                i = j;
                continue;
            } catch (const std::invalid_argument& _) {
                insts.push_back(inst::inst_push_label(tokens[j].c_str()));
                i = j;
                continue;
            }
        }

        assert(0);
    }

    return insts;
}

} // namespace parse
