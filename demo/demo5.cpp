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

// json.txt
int main(int argc, char *argv[]) {
    const string text = readFile(string(argv[1]));

    Combinator body, list, item, items, id, ids, pair, str, pairs;
    Combinator space, seq, com, sem;
    Combinator SEM, COM, EMP, LB, RB, Lb, Rb, QUO, CH, ANY, SPA;

    CH  = token([](char c) {
        return ::isalnum(c) or (int)string{"+-*/%=_><|?.!@#$^"}.find(c) != -1;
    });
    SPA = token(::isspace);
    ANY = CH | token(" ");
    QUO = token("\"");
    SEM = token(":");
    COM = token(",");
    EMP = token("");
    LB  = token("{");
    RB  = token("}");
    Lb  = token("[");
    Rb  = token("]");

    seq   =  ANY + seq | EMP;
    space = (SPA + space | EMP)               >> [](string s) -> string { return " space "; };
    com   = space + COM + space               >> [](string a, string b, string c) -> string { return ","; };
    sem   = space + SEM + space               >> [](string a, string b, string c) -> string { return ":"; };
    list  = Lb + space + items + space + Rb   >> [](string a, string b, string c, string d, string e) -> string { return "list"; };
    id    = CH + id | CH;
    ids   = id + com + ids | id | EMP;
    str   = QUO + seq + QUO;
    item  = (list | body | id | str)          >> [](string a) -> string { return "item"; };
    items = item + com + items | item | EMP;
    pair  = (str + sem + item)                >> [&](string a, string b, string c) -> string { return "pair"; };
    pairs = pair + com + pairs | pair | EMP;
    body  = LB + space + pairs + space + RB;

    space.setTerminal(true);
    com.setTerminal(true);
    sem.setTerminal(true);
    id.setTerminal(true);
    str.setTerminal(true);

    auto res = item(text);

    if (res) {
        printAST(cout, res->ast);
    } else {
        cout << "fail" << '\n';
    }
}