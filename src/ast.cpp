#include "pch.hpp"
#include "ast.hpp"

namespace miniparser {

void printAST(std::ostream &out, std::shared_ptr<ASTnode> root) {
    static const std::string patterns[4] {
        "│   ",
        "├───",
        "└───",
        "    ",
    };

    std::vector<int> tab;
    auto dfs = [&](auto &&self, std::shared_ptr<ASTnode> ptr) -> void {
        for (int i : tab) {
            out << patterns[i];
        }
        out << ptr->display << '\n';
        int last = -1;
        if (!tab.empty()) {
            last = tab.back();
            if (last == 1) {
                tab.back() = 0;
            } else if (last == 2) {
                tab.back() = 3;
            }
        }
        tab.push_back(1);
        for (auto sub : ptr->children) {
            if (sub == ptr->children.back()) {
                tab.back() = 2;
            }
            self(self, sub);
        }
        tab.pop_back();
        if (last != -1) {
            tab.back() = last;
        }
    };
    dfs(dfs, root);
}

} // namespace miniparser