#ifndef _PARSEHELPER_H_
#define _PARSEHELPER_H_

// Функция удаляет пробелы в начале и конце строки
void alltrim(char *parseline);

// Функция возвращает число с нужным номером из строки где числа разделены запятыми
int get_comma_separate_valuei(char *line, int n);

// Функции для работы со строками вида "параметр=значение"
void getparamname(char *parseline);
void getparamvalue(char *parseline);


#endif // _PARSEHELPER_H_
