#pragma once

#include "pch.hpp"
#include "ast.hpp"

namespace miniparser {

namespace detail {

template<class... T>
struct MulType {};

template<class>
struct is_MulType : std::false_type {};
template<class... T>
struct is_MulType<MulType<T...>> : std::true_type {};
template<class T>
inline constexpr bool is_MulType_v = is_MulType<T>::value;

} // namespace detail

using ParserInput = std::string_view;
constexpr std::nullopt_t failureParserOutput = std::nullopt;
struct successParserOutput {
    std::shared_ptr<ASTnode> ast;
    ParserInput remain;
};
using ParserOutput = std::optional<successParserOutput>;
using Parser = std::function<ParserOutput(ParserInput)>;

template<class T>
struct ParserCombinator {
    Parser parser;
    ParserCombinator() {}
    ParserCombinator(Parser parser) {
        this->parser = parser;
    }
    ParserCombinator<T> lazy() const {
        return ParserCombinator(
            [ptr=this](ParserInput input) -> ParserOutput {
                return (*ptr)(input);
            }
        );
    }
    ParserOutput operator()(ParserInput input) const {
        return parser(input);
    }
    template<class... A> requires (std::is_same_v<T, std::string>)
    ParserCombinator<T> &operator=(const ParserCombinator<detail::MulType<A...>>& pc) {
        parser = [=](ParserInput input) -> ParserOutput {
            auto ret = pc(input);
            if (!ret) {
                return failureParserOutput;
            }
            for (auto subast : ret->ast->children) {
                ret->ast->display += subast->display;
            }
            ret->ast->result = ret->ast->display;
            return ret;
        };
        return *this;
    }
};

namespace detail {

template<class... A>
struct CatType : std::type_identity<MulType<A...>> {};
template<class... A, class... B>
struct CatType<MulType<A...>, MulType<B...>> : std::type_identity<MulType<A..., B...>> {};
template<class... A, class B>
struct CatType<MulType<A...>, B> : std::type_identity<MulType<A..., B>> {};
template<class... T>
using CatType_t = CatType<T...>::type;

template<class>
struct is_Combinator : std::false_type {};
template<class T>
struct is_Combinator<ParserCombinator<T>> : std::true_type {};
template<class T>
inline constexpr bool is_Combinator_v = is_Combinator<std::remove_reference_t<T>>::value;

template<class>
struct CombinatorType {};
template<class T>
struct CombinatorType<ParserCombinator<T>> : std::type_identity<T> {};
template<class T>
using CombinatorType_t = CombinatorType<std::remove_reference_t<T>>::type;

template<class R, class... A>
struct Signature { using result_type = R; using args_tuple = std::tuple<A...>; };

template<class T>
struct removeClass {};
template<class C, class R, class... A>
struct removeClass<R(C::*)(A...)>             : std::type_identity<Signature<R, A...>> {};
template<class C, class R, class... A> 
struct removeClass<R(C::*)(A...) const>       : std::type_identity<Signature<R, A...>> {};

template<class T>
struct getSignature_impl {
    using type = removeClass<decltype(&std::remove_reference_t<T>::operator())>::type;
};
template<class R, class... A>
struct getSignature_impl<R(A...)>             : std::type_identity<Signature<R, A...>> {};
template<class R, class... A>
struct getSignature_impl<R(&)(A...)>          : std::type_identity<Signature<R, A...>> {};
template<class R, class... A>
struct getSignature_impl<R(*)(A...)>          : std::type_identity<Signature<R, A...>> {};
template<class R, class... A>
struct getSignature_impl<R(&)(A...) noexcept> : std::type_identity<Signature<R, A...>> {};
template<class R, class... A>
struct getSignature_impl<R(*)(A...) noexcept> : std::type_identity<Signature<R, A...>> {};
template<class T>
using getSignature = getSignature_impl<T>::type;

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
    return std::to_string(value);
}
template<>
std::string toString<std::string>(std::string);

} // namespace detail

template<class T> requires (!detail::is_MulType_v<T>)
ParserCombinator<detail::MulType<T>> upToMul(ParserCombinator<T> combinator) {
    return Parser{[=](ParserInput input) -> ParserOutput {
        auto ret = combinator(input);
        if (!ret) {
            return failureParserOutput;
        }
        std::shared_ptr<ASTnode> node = std::make_shared<ASTnode>();
        node->children.push_back(ret->ast);
        return successParserOutput{node, ret->remain};
    }};
}

template<class A, class B, 
         class R = detail::CatType_t<detail::CombinatorType_t<A>, detail::CombinatorType_t<B>>>
requires (detail::is_Combinator_v<A> and detail::is_Combinator_v<B>)
auto operator+(A&& pc1, B&& pc2) {
    if constexpr (std::is_lvalue_reference_v<A>) {
        return pc1.lazy() + std::forward<B>(pc2);
    } else if constexpr (std::is_lvalue_reference_v<B>) {
        return std::forward<A>(pc1) + pc2.lazy();
    } else if constexpr (!detail::is_MulType_v<detail::CombinatorType_t<A>>) {
        return upToMul(pc1) + std::forward<B>(pc2);
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

template<class Func, class R = detail::getSignature<Func>::result_type, class T>
requires (std::is_same_v<typename detail::getSignature<Func>::args_tuple, std::tuple<T>>)
ParserCombinator<R> operator>>(ParserCombinator<T> pc, Func&& func) {
    return upToMul(pc) >> func;
}

template<class Func, class R = detail::getSignature<Func>::result_type, class... T>
requires (std::is_same_v<typename detail::getSignature<Func>::args_tuple, std::tuple<T...>>)
ParserCombinator<R> operator>>(ParserCombinator<detail::MulType<T...>> pc, Func&& func) {
    return ParserCombinator<R>{Parser{
        [=](ParserInput input) -> ParserOutput {
            auto ret = pc(input);
            if (!ret) {
                return failureParserOutput;
            }
            ret->ast->result = std::apply(func, detail::toTuple<T...>(ret->ast->children.begin()));
            ret->ast->display = detail::toString<R>(std::any_cast<R>(ret->ast->result));
            return ret;
        }
    }};
}

ParserCombinator<std::string> ending();
ParserCombinator<std::string> token(std::string);
ParserCombinator<std::string> token(const std::function<bool(char)>&);
ParserCombinator<std::string> oneof(std::string);

} // namespace miniparser