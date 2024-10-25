#include "inst.hpp"
#include "interpret.hpp"
#include "parse.hpp"

#include <cassert>
#include <fstream>

template <typename T>
static std::ostream& operator<<(std::ostream& stream, const std::vector<T>& vec) {
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

int main(int argc, const char** argv) {
    assert(2 < argc);

    std::ifstream file(argv[1]);
    assert(file);
    const std::string source((std::istreambuf_iterator<char>(file)),
                             std::istreambuf_iterator<char>());

    std::vector<std::string> tokens = parse::tokenize(source);
    std::vector<inst::Inst>  insts = parse::parse(tokens);

    interpret::resolve_labels(insts);

    bool can_trace;
    if (argv[2][0] == 'f') {
        can_trace = false;
    } else if (argv[2][0] == 't') {
        can_trace = true;
    } else {
        assert(0);
    }

    std::vector<inst::Op> stack = interpret::interpret(insts, can_trace);

    std::cout << '\n' << stack << std::endl;
    return 0;
}
