#ifndef INTERPRET_HPP
#define INTERPRET_HPP

#include "inst.hpp"

namespace interpret {

void                      resolve_labels(std::vector<inst::Inst>&);
std::vector<inst::InstOp> interpret(const std::vector<inst::Inst>&);

} // namespace interpret

#endif
