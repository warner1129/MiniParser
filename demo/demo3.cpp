#include "miniparser"
#include <iostream>
#include <fstream>
#include <cmath>

using namespace std;
using namespace miniparser;

template<class T = string>
using Combinator = ParserCombinator<T>;

string readFile(string filename) {
    std::ifstream file;
    file.open(filename);
    if (file.fail()) {
        cerr << "file not exist\n";
        exit(0);
    }
    string str = (stringstream{} << file.rdbuf()).str();
    file.close();
    return str;
}

// expr.txt
int main(int argc, char *argv[]) {
    const string text = readFile(string(argv[1]));

    Combinator<double> Terms, Factors, Factor, Number, Digit;

    Digit = token(::isdigit) >> [](string s) -> double { return s[0] - '0'; };

    Number 
        = Digit + Number
            >> [](double a, double b) { 
                return a * pow(10., floor(log(b)) + 1) + b;
            }
        | Digit
        ;

    Factor 
        = Number
        | token("(") + Terms + token(")") 
            >> [](string a, double b, string c) -> double {
                return b;
            }
        ;

    Factors 
        = Factor + (token("*") | token("/")) + Factors 
            >> [](double l, string op, double r) -> double {
                return (op == "*") ? l * r : l / r;
            }
        | Factor
        ;

    Terms 
        = Factors + (token("+") | token("-")) + Terms 
            >> [](double l, string o, double r) -> double {
                return (o == "+") ? l + r : l - r;
            }
        | Factors
        ;
    
    auto res = Terms(text);

    if (res) {
        printAST(cout, res->ast);
    } else {
        cout << "fail" << '\n';
    }
}