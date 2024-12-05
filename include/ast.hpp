#pragma once

#include "pch.hpp"

namespace miniparser {

struct ASTnode {
    std::any result;
    std::list<std::shared_ptr<ASTnode>> children;
    std::string display;
};

void dfs(std::shared_ptr<ASTnode>, std::string = "");

} // namespace miniparser