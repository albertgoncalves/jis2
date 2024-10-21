#include "inst.hpp"
#include "interpret.hpp"
#include "parse.hpp"

#include <cassert>
#include <fstream>

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

static std::string string_from_path(const char* path) {
    std::ifstream file(path);
    assert(file);
    const std::string buffer((std::istreambuf_iterator<char>(file)),
                             std::istreambuf_iterator<char>());
    return buffer;
}

int main(int argc, const char** argv) {
    assert(1 < argc);

    const std::string source = string_from_path(argv[1]);

    std::vector<std::string> tokens = parse::tokenize(source);
    std::vector<inst::Inst>  insts = parse::parse(tokens);

    interpret::resolve_labels(insts);
    std::vector<inst::Op> stack = interpret::interpret(insts);

    std::cout << insts << std::endl << stack << std::endl;
    return 0;
}
