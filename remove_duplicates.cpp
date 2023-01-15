#include "remove_duplicates.h"

#include "search_server.h"
#include <iostream>
#include <string>
#include <set>
#include <map>

using std::cout;
using std::endl;
using std::map;
using std::set;
using std::string;

void RemoveDuplicates(SearchServer& search_server) {
    set<set<string>> documents_inverting;
    set<int> duplicates;

    for (const int& id : search_server) {
        set<string> words;
        for (const auto [word, _] : search_server.GetWordFrequencies(id)) {
            words.insert(word);
        }

        if (documents_inverting.count(words)) {
            duplicates.insert(id);
        }
        else {
            documents_inverting.insert(words);
        }
    }

    for (const int& id : duplicates) {
        cout << "Found duplicate document id " << id << endl;
        search_server.RemoveDocument(id);
    }
}
