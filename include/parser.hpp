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

template<class T = std::string>
struct ParserCombinator {
private:
    Parser parser;
    bool is_terminal{};
public:
    ParserCombinator() {}
    ParserCombinator(Parser);
    template<class... A> requires (std::is_same_v<T, std::string>)
    ParserCombinator(const ParserCombinator<detail::MulType<A...>>&);
    ParserOutput operator()(ParserInput input) const;
    void setTerminal(bool = true);
    void setParser(Parser);
    Parser getParser() const;
    ParserCombinator<T> lazy() const;
    ParserCombinator<T>& operator=(const ParserCombinator<T>&);
    template<class... A> requires (std::is_same_v<T, std::string>)
    ParserCombinator<T>& operator=(const ParserCombinator<detail::MulType<A...>>&);
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
struct Signature {
    using result_type = R;
    using args_tuple = std::tuple<A...>;
};

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
using getSignature_t = getSignature_impl<T>::type;

template<class A, class... B>
std::tuple<A, B...> toTuple(std::list<std::shared_ptr<ASTnode>>::iterator);

template<class T>
std::string toString(T);

} // namespace detail

template<class A, class B, 
         class R = detail::CatType_t<detail::CombinatorType_t<A>, detail::CombinatorType_t<B>>>
requires (detail::is_Combinator_v<A> and detail::is_Combinator_v<B>)
auto operator+(A&&, B&&);

template<class A, class B, class R = detail::CombinatorType_t<A>>
requires (detail::is_Combinator_v<A> and detail::is_Combinator_v<B>)
auto operator|(A&&, B&&);

template<class Func, class R = detail::getSignature_t<Func>::result_type, class T>
requires (std::is_same_v<typename detail::getSignature_t<Func>::args_tuple, std::tuple<T>>)
ParserCombinator<R> operator>>(ParserCombinator<T>, Func&&);

template<class Func, class R = detail::getSignature_t<Func>::result_type, class... T>
requires (std::is_same_v<typename detail::getSignature_t<Func>::args_tuple, std::tuple<T...>>)
ParserCombinator<R> operator>>(ParserCombinator<detail::MulType<T...>>, Func&&);

template<class T> requires (!detail::is_MulType_v<T>)
ParserCombinator<detail::MulType<T>> upToMul(ParserCombinator<T>);

template<class T> requires (detail::is_MulType_v<T>)
ParserCombinator<std::string> downToStr(ParserCombinator<T>);

ParserCombinator<std::string> ending();
ParserCombinator<std::string> token(std::string);
ParserCombinator<std::string> token(const std::function<bool(char)>&);
ParserCombinator<std::string> oneof(std::string);

} // namespace miniparser

#include "parser.tpp"