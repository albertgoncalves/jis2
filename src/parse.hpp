#ifndef PARSE_HPP
#define PARSE_HPP

#include "inst.hpp"

namespace parse {

std::vector<std::string> tokenize(const std::string_view&);

std::vector<inst::Inst> parse(std::vector<std::string>&);

} // namespace parse

#endif
