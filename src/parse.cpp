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
            insts.push_back(inst::halt());
            continue;
        }
        if (tokens[i] == std::string_view("ret")) {
            insts.push_back(inst::ret());
            continue;
        }
        if (tokens[i] == std::string_view("eq")) {
            insts.push_back(inst::eq());
            continue;
        }
        if (tokens[i] == std::string_view("ge")) {
            insts.push_back(inst::ge());
            continue;
        }
        if (tokens[i] == std::string_view("add")) {
            insts.push_back(inst::add());
            continue;
        }
        if (tokens[i][tokens[i].size() - 1] == ':') {
            insts.push_back(inst::label(tokens[i].c_str()));
            continue;
        }

        usize j = i + 1;
        assert(j < n);

        if (tokens[i] == std::string_view("jump")) {
            insts.push_back(inst::jump(tokens[j].c_str()));
            goto next;
        }
        if (tokens[i] == std::string_view("jz")) {
            insts.push_back(inst::jz(tokens[j].c_str()));
            goto next;
        }
        if (tokens[i] == std::string_view("dup")) {
            insts.push_back(inst::dup(stoull(tokens[j])));
            goto next;
        }
        if (tokens[i] == std::string_view("swap")) {
            insts.push_back(inst::swap(stoull(tokens[j])));
            goto next;
        }
        if (tokens[i] == std::string_view("drop")) {
            insts.push_back(inst::drop(stoull(tokens[j])));
            goto next;
        }
        if (tokens[i] == std::string_view("push")) {
            try {
                insts.push_back(inst::push_integer(stoll(tokens[j])));
            } catch (const std::invalid_argument& _) {
                insts.push_back(inst::push_label(tokens[j].c_str()));
            }
            goto next;
        }

        assert(0);
    next:
        i = j;
    }

    return insts;
}

} // namespace parse
