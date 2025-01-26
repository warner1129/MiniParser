#include "miniparser"
#include <iostream>
#include <fstream>

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

    Combinator<string> Terms, Factors, Factor, Number, Digit;

    Digit = token(::isdigit);

    Number 
        = Digit + Number
        | Digit;

    Factor 
        = Number
        | token("(") + Terms + token(")");

    Factors 
        = Factor + (token("*") | token("/")) + Factors 
            >> [](string l, string o, string r) -> string {
                return o + " " + l + " " + r;
            }
        | Factor;

    Terms 
        = Factors + (token("+") | token("-")) + Terms 
            >> [](string l, string o, string r) -> string {
                return o + " " + l + " " + r;
            }
        | Factors;
    
    auto res = Terms(text);

    if (res) {
        printAST(cout, res->ast);
    } else {
        cout << "fail" << '\n';
    }
}