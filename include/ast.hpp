#pragma once

#include "pch.hpp"

namespace miniparser {

struct ASTnode {
    std::any result;
    std::list<std::shared_ptr<ASTnode>> children;
    std::string display;
};

void printAST(std::ostream&, std::shared_ptr<ASTnode>);

} // namespace miniparser