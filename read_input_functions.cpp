#include "read_input_functions.h"

#include <iostream>

using std::string;
using std::cin;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result;
    cin >> result;
    ReadLine();
    return result;
}// в качестве заготовки кода используйте последнюю версию своей поисковой системы 
