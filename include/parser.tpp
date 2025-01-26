#include "parser.hpp"

namespace miniparser {

template<class T>
ParserCombinator<T>::ParserCombinator(Parser parser) {
    this->parser = parser;
}

template<class T>
template<class... A> requires (std::is_same_v<T, std::string>)
ParserCombinator<T>::ParserCombinator(const ParserCombinator<detail::MulType<A...>>& pc) {
    (*this) = downToStr(pc);
}

template<class T>
ParserOutput ParserCombinator<T>::operator()(ParserInput input) const {
    auto ret = parser(input);
    if (ret and is_terminal) {
        ret->ast->children.clear();
    }
    return ret;
}

template<class T>
void ParserCombinator<T>::setTerminal(bool terminal) {
    this->is_terminal = terminal;
}

template<class T>
void ParserCombinator<T>::setParser(Parser parser) {
    this->parser = parser;
}

template<class T>
Parser ParserCombinator<T>::getParser() const {
    return parser;
}

template<class T>
ParserCombinator<T> ParserCombinator<T>::lazy() const {
    return ParserCombinator(
        [ptr=this](ParserInput input) -> ParserOutput {
            return (*ptr)(input);
        }
    );
}

template<class T>
ParserCombinator<T>& ParserCombinator<T>::operator=(ParserCombinator<T>&& pc) {
    this->parser = pc.getParser();
    return *this;
}

template<class T>
ParserCombinator<T>& ParserCombinator<T>::operator=(const ParserCombinator<T>& pc) {
    this->parser = pc.lazy().getParser();
    return *this;
}

template<class T>
template<class... A> requires (std::is_same_v<T, std::string>)
ParserCombinator<T>& ParserCombinator<T>::operator=(const ParserCombinator<detail::MulType<A...>>& pc) {
    return *this = downToStr(pc);
}

template<class T> requires (!detail::is_MulType_v<T>)
ParserCombinator<detail::MulType<T>> upToMul(ParserCombinator<T> pc) {
    return Parser{[=](ParserInput input) -> ParserOutput {
        auto ret = pc(input);
        if (!ret) {
            return failureParserOutput;
        }
        std::shared_ptr<ASTnode> node = std::make_shared<ASTnode>();
        node->children.push_back(ret->ast);
        return successParserOutput{node, ret->remain};
    }};
}

template<class T> requires (detail::is_MulType_v<T>)
ParserCombinator<std::string> downToStr(ParserCombinator<T> pc) {
    return ParserCombinator<std::string>{Parser{
        [=](ParserInput input) -> ParserOutput {
            auto ret = pc(input);
            if (!ret) {
                return failureParserOutput;
            }
            for (auto subast : ret->ast->children) {
                ret->ast->display += subast->display;
            }
            ret->ast->result = ret->ast->display;
            return ret;
        }
    }};
}

template<class A, class B, class R>
requires (detail::is_Combinator_v<A> and detail::is_Combinator_v<B>)
auto operator+(A&& pc1, B&& pc2) {
    if constexpr (std::is_lvalue_reference_v<A>) {
        return pc1.lazy()           + std::forward<B>(pc2);
    } else if constexpr (std::is_lvalue_reference_v<B>) {
        return std::forward<A>(pc1) + pc2.lazy();
    } else if constexpr (!detail::is_MulType_v<detail::CombinatorType_t<A>>) {
        return upToMul(pc1)         + std::forward<B>(pc2);
    } else if constexpr (!detail::is_MulType_v<detail::CombinatorType_t<B>>) {
        return std::forward<A>(pc1) + upToMul(pc2);
    } else {
        return ParserCombinator<R>{Parser{
            [=](ParserInput input) -> ParserOutput {
                auto ret1 = pc1(input);
                if (!ret1) {
                    return failureParserOutput;
                }
                auto ret2 = pc2(ret1->remain);
                if (!ret2) {
                    return failureParserOutput;
                }
                ret1->ast->children.splice(ret1->ast->children.end(), ret2->ast->children);
                ret1->remain = ret2->remain;
                return ret1;
            }
        }};
    }
}

template<class A, class B, class R>
requires (detail::is_Combinator_v<A> and detail::is_Combinator_v<B>)
auto operator|(A&& pc1, B&& pc2) {
    if constexpr (std::is_lvalue_reference_v<A>) {
        return pc1.lazy()           | std::forward<B>(pc2);
    } else if constexpr (std::is_lvalue_reference_v<B>) {
        return std::forward<A>(pc1) | pc2.lazy();
    } else if constexpr (detail::is_MulType_v<detail::CombinatorType_t<A>>) {
        return downToStr(pc1)       | std::forward<B>(pc2);
    } else if constexpr (detail::is_MulType_v<detail::CombinatorType_t<B>>) {
        return std::forward<A>(pc1) | downToStr(pc2);
    } else {
        static_assert(std::is_same_v<detail::CombinatorType_t<A>, detail::CombinatorType_t<B>>);
        return ParserCombinator<R>{Parser{
            [=](ParserInput input) -> ParserOutput {
                auto ret1 = pc1(input);
                if (ret1) {
                    return ret1;
                }
                return pc2(input);
            }
        }};
    }
}

template<class Func, class R, class T>
requires (std::is_same_v<typename detail::getSignature_t<Func>::args_tuple, std::tuple<T>>)
ParserCombinator<R> operator>>(ParserCombinator<T> pc, Func&& func) {
    return upToMul(pc) >> func;
}

template<class Func, class R, class... T>
requires (std::is_same_v<typename detail::getSignature_t<Func>::args_tuple, std::tuple<T...>>)
ParserCombinator<R> operator>>(ParserCombinator<detail::MulType<T...>> pc, Func&& func) {
    return ParserCombinator<R>{Parser{
        [=](ParserInput input) -> ParserOutput {
            auto ret = pc(input);
            if (!ret) {
                return failureParserOutput;
            }
            ret->ast->result = std::apply(func, detail::toTuple<T...>(ret->ast->children.begin()));
            ret->ast->display = detail::toString(std::any_cast<R>(ret->ast->result));
            return ret;
        }
    }};
}

namespace detail {

template<class A, class... B>
std::tuple<A, B...> toTuple(std::list<std::shared_ptr<ASTnode>>::iterator it) {
    if constexpr (sizeof...(B) == 0) {
        return std::tuple<A>(any_cast<A>((*it)->result));
    } else {
        return std::tuple_cat(toTuple<A>(it), toTuple<B...>(next(it)));
    }
}

template<class T>
std::string toString(T value) {
    return (std::stringstream{} << value).str();
}

} // namespace detail

} // namespace miniparser