#include "pch.hpp"
#include "parser.hpp"
#include "ast.hpp"

namespace miniparser {

namespace detail {

template<>
std::string toString<std::string>(std::string str) { return str; }

} // namespaec detail

ParserCombinator<std::string> ending() {
    return Parser{
        [](ParserInput input) -> ParserOutput {
            if (input.empty()) {
                std::shared_ptr<ASTnode> node = std::make_shared<ASTnode>();
                node->display = "END";
                return successParserOutput{node, input};
            }
            return failureParserOutput;
        }
    };
}

ParserCombinator<std::string> token(std::string t) {
    return Parser{
        [=](ParserInput input) -> ParserOutput {
            if (input.starts_with(t)) {
                input.remove_prefix(t.length());
                std::shared_ptr<ASTnode> node = std::make_shared<ASTnode>();
                node->result = t;
                node->display = t;
                return successParserOutput{node, input};
            }
            return failureParserOutput;
        }
    };
}

ParserCombinator<std::string> token(const std::function<bool(char)>& pred) {
    return Parser{
        [=](ParserInput input) -> ParserOutput {
            if (input.empty() or !pred(input[0])) {
                return failureParserOutput;
            }
            std::shared_ptr<ASTnode> node = std::make_shared<ASTnode>();
            node->result = std::string(1, input[0]);
            node->display = std::string(1, input[0]);
            input.remove_prefix(1);
            return successParserOutput{node, input};
        }
    };
}

} // namespace miniparser