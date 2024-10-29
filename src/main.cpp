#include "inst.hpp"
#include "interpret.hpp"
#include "parse.hpp"

#include <cassert>
#include <fstream>

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

    inst::Op* stack = interpret::interpret(insts, can_trace);

    std::cout << '\n' << *stack << std::endl;

    return 0;
}
