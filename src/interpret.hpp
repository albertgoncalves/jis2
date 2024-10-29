#ifndef INTERPRET_HPP
#define INTERPRET_HPP

#include "inst.hpp"

#include <vector>

namespace interpret {

void resolve_labels(std::vector<inst::Inst>&);

inst::Op* interpret(const std::vector<inst::Inst>&, bool);

} // namespace interpret

#endif
