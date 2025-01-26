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

// rbs.txt
int main(int argc, char *argv[]) {
    const string text = readFile(string(argv[1]));
    Combinator RBS;

    RBS = token("(") + RBS + token(")") + RBS
        | token("[") + RBS + token("]") + RBS
        | token("{") + RBS + token("}") + RBS
        | token("")
        ;

    auto res = RBS(text);

    if (res) {
        printAST(cout, res->ast);
    } else {
        cout << "fail" << '\n';
    }
}