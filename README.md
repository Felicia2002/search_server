# search-server
Упрощенная версия поисковой системы

# Описание
* Ранжирование по  TF-IDF.
* Параллельные запросы и поиск документов (работа в нексольких потоках).
* Обработка ошибок:
1. Попытка добавить документ с отрицательным id;
2. Попытка добавить документ c id ранее добавленного документа;
3. Наличие недопустимых символов (с кодами от 0 до 31) в тексте добавляемого документа.
4. В словах поискового запроса есть недопустимые символы с кодами от 0 до 31;
5. Наличие более чем одного минуса перед словами, которых не должно быть в искомых документах, например, пушистый --кот. В середине слов минусы разрешаются, например: иван-чай.
6. Отсутствие текста после символа «минус»: в поисковом запросе: "пушистый -".
Сделала этот проект в качестве упражнения с новыми для технологиями.
# Используемые технологии и инструменты:
Параллельность, ООП, структуры, макросы, list, перечисления enum, лямбда - функции, санитайзеры, С++17
Нужен для того, чтобы лучше понять как происходит поиск в интернете.

# Cистемные требования 
* С++17
* gcc10
* Программа Visual C++ 
* ОС Windows 
* команда для запуска:
```
gcc main.cpp -o <название файла>
```
Для того, чтобы проверить работу программы 
```
#include "process_queries.h"
#include "search_server.h"
#include <execution>
#include <iostream>
#include <string>
#include <vector>
using namespace std;
void PrintDocument(const Document& document) {
    cout << "{ "s
         << "document_id = "s << document.id << ", "s
         << "relevance = "s << document.relevance << ", "s
         << "rating = "s << document.rating << " }"s << endl;
}
int main() {
    SearchServer search_server("and with"s);
    int id = 0;
    for (
        const string& text : {
            "white cat and yellow hat"s,
            "curly cat curly tail"s,
            "nasty dog with big eyes"s,
            "nasty pigeon john"s,
        }
    ) {
        search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, {1, 2});
    }
    cout << "ACTUAL by default:"s << endl;
    // последовательная версия
    for (const Document& document : search_server.FindTopDocuments("curly nasty cat"s)) {
        PrintDocument(document);
    }
    cout << "BANNED:"s << endl;
    // последовательная версия
    for (const Document& document : search_server.FindTopDocuments(execution::seq, "curly nasty cat"s, DocumentStatus::BANNED)) {
        PrintDocument(document);
    }
    cout << "Even ids:"s << endl;
    // параллельная версия
    for (const Document& document : search_server.FindTopDocuments(execution::par, "curly nasty cat"s, [](int document_id, DocumentStatus status, int rating) { return document_id % 2 == 0; })) {
        PrintDocument(document);
    }
    return 0;
} 
```
