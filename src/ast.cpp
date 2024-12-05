#include "pch.hpp"
#include "ast.hpp"

namespace miniparser {

// '|' "─" '└' '├' '┬' '│'

void dfs(std::shared_ptr<ASTnode> ptr, std::string tab) {
    std::cerr << tab << ptr->display << '\n';
    if (!tab.empty()) {
        if (tab.substr(tab.size() - 12, 3) == "├") {
            tab = tab.substr(0, tab.size() - 12) + "│   ";
        } else if (tab.substr(tab.size() - 12, 3) == "└") {
            tab = tab.substr(0, tab.size() - 12) + "    ";
        }
    }
    for (auto sub : ptr->children) {
        bool last = (sub == ptr->children.back());
        dfs(sub, tab + (last ? "└───" : "├───"));
    }
}

} // namespace miniparser