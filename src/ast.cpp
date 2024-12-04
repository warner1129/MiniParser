#include "pch.hpp"
#include "ast.hpp"

namespace miniparser {

void dfs(std::shared_ptr<ASTnode> ptr) {
    std::cerr << ptr->display << '\n';
    for (auto sub : ptr->children) {
        dfs(sub);
    }
}

} // namespace miniparser