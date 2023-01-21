# search-server
Упрощенная версия поисковой системы

# Описание
* Ранжирование по  TF-IDF.
* Обработка ошибок:
1. Попытка добавить документ с отрицательным id;
2. Попытка добавить документ c id ранее добавленного документа;
3. Наличие недопустимых символов (с кодами от 0 до 31) в тексте добавляемого документа.
4. В словах поискового запроса есть недопустимые символы с кодами от 0 до 31;
5. Наличие более чем одного минуса перед словами, которых не должно быть в искомых документах, например, пушистый --кот. В середине слов минусы разрешаются, например: иван-чай.
6. Отсутствие текста после символа «минус»: в поисковом запросе: "пушистый -".
Сделала этот проект в качестве упражнения с новыми для технологиями.
# Используемые технологии и инструменты:
ООП, структуры, макросы, list, перечисления enum, лямбда - функции, санитайзеры, С++17
Нужен для того, чтобы лучше понять как происходит поиск в интернете.

# Инструкция по развёртыванию и системные требования
* С++17
* gcc10
* Программа Visual C++ 
* ОС Windows 
* команда для запуска:
```
gcc main.cpp -o <название файла>
```
Ожидаемый вывод этой программы:
```
Before duplicates removed: 9
Found duplicate document id 3
Found duplicate document id 4
Found duplicate document id 5
Found duplicate document id 7
After duplicates removed: 5 
```
