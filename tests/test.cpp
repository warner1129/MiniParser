#include "miniparser"

#include <iostream>
#include <cassert>

using namespace std;

template<class T = string>
using Combinator = miniparser::ParserCombinator<T>;

void test_type() {
    using namespace miniparser;
    using namespace miniparser::detail;

    using T1 = Combinator<int>;
    using T2 = Combinator<float>;
    using T3 = Combinator<string>;
    using T4 = Combinator<MulType<int, float>>;
    using T5 = Combinator<CatType_t<int, float>>;

    auto testcase = {
        is_MulType_v<MulType<MulType<int>>>,
        is_MulType_v<MulType<MulType<int>, float, MulType<double>>>,
        is_MulType_v<MulType<int>>,
        is_MulType_v<MulType<int>>,
        is_MulType_v<MulType<int, float, double>>,
        (is_MulType_v<int> == false),
        (is_MulType_v<vector<int>> == false),
        is_MulType_v<CatType_t<int, float>>,
        is_MulType_v<CatType_t<MulType<int, float>, double>>,
        is_MulType_v<CatType_t<MulType<int, float>, MulType<double, string>>>,

        (is_same_v<MulType<float>, CatType_t<float>>),
        (is_same_v<MulType<int, float>, CatType_t<int, float>>),
        (is_same_v<MulType<int, float>, CatType_t<MulType<int>, float>>),
        (is_same_v<MulType<int, float, double>, CatType_t<MulType<int, float>, double>>),
        (is_same_v<MulType<int, float, double, int>, CatType_t<MulType<int, float>, MulType<double, int>>>),

        (is_MulType_v<MulType<int>>),
        (is_MulType_v<MulType<int, float, double>>),
        (is_MulType_v<int> == false),
        (is_MulType_v<CatType_t<int, float>>),

        (is_same_v<CombinatorType_t<T4>, CombinatorType_t<T5>>),
        (!is_same_v<CombinatorType_t<T1>, CombinatorType_t<T2>>),
        (is_same_v<CatType_t<CombinatorType_t<T1>, CombinatorType_t<T2>>, MulType<int, float>>),
        (is_same_v<Combinator<CatType_t<CombinatorType_t<T2>, CombinatorType_t<T3>>>, Combinator<MulType<float, string>>>),

        (is_Combinator_v<T1>),
        (is_Combinator_v<T2>),
        (is_Combinator_v<T3>),
        (is_Combinator_v<vector<int>> == false),
        (is_Combinator_v<int> == false),
        (is_Combinator_v<MulType<int, float>> == false),
    };

    for (bool x : testcase) {
        assert(x);
    }

    cerr << "pass type test" << endl;
}

void test_parser1() {
    using namespace miniparser;

/*
Expr ::= Term Terms
Terms ::= (‘+’ | ‘-’) Term Terms | ε 
Term ::= Factor Factors
Factors ::= (‘*’ | ‘/’) Factor Factors | ε 
Factor ::= number | ‘(‘ Expr ‘)’
number :== digit number | digit
digit = '0' | '1' | '2' | ... | '9'
*/

    Combinator<string> Expr, Term, Terms, Factors, Factor, Number, Digit;
    Digit = token(::isdigit);

    Expr = Terms;
    Term = Factors;

    Terms 
        = Term + (token("+") | token("-")) + Terms
        | Term
        ;


    Factors
        = Factor + (token("*") | token("/")) + Factors
        | Factor
        ;

    Factor
        = Number
        | token("(") + Expr + token(")")
        ;

    Number
        = Digit + Number
        | Digit
        ;


    Number.setTerminal(true);

    string text = "(12+(11-3)*3)/6";

    auto res = Expr(text);

    assert(res);
    printAST(cout, res->ast);

    cerr << "pass parser test" << endl;
}

void test_parser2() {
    using namespace miniparser;

    Combinator<int> Exp, Ter, Fac, Num, Dig;
    Dig = token(::isdigit) >> [](string s) { return stoi(s); };

    Exp = Ter + (token("+") | token("-")) + Exp >> [](int a, string b, int c) -> int { return b == "+" ? (a + c) : (a - c); }
        | Ter;

    Ter = Fac + (token("*") | token("/")) + Exp >> [](int a, string b, int c) -> int { return b == "*" ? (a * c) : (a / c); }
        | Fac;

    Fac = Num 
        | token("(") + Exp + token(")") >> [](string a, int b, string c) -> int { return b; };

    Num = Dig + Num >> [](int a, int b) -> int { return stoi(to_string(a) + to_string(b)); }
        | Dig;

    Num.setTerminal(true);

    const string text = "122+232*323-(2+10-(223*322)+1)";

    auto res = Exp(text);

    assert(res);
    printAST(cout, res->ast);

    cerr << "pass parser test" << endl;
}

void test_parser3() {
    using namespace miniparser;

    Combinator Exp;

    Exp = token("(") + Exp + token(")") + Exp
        | token("[") + Exp + token("]") + Exp
        | token("{") + Exp + token("}") + Exp
        | token("")
        ;

    const string text = "()([{}][]({}{}[{}]))([][{[]}][](){}{[]{}}[[]])";

    auto res = Exp(text);

    assert(res);
    printAST(cout, res->ast);

    cerr << "pass parser test" << endl;
}

int main() {
    cerr << "testing..." << endl;
    test_type();
    test_parser1();
    test_parser2();
    test_parser3();
}