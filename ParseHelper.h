#ifndef _PARSEHELPER_H_
#define _PARSEHELPER_H_

// Функция удаляет пробелы в начале и конце строки
void allTrim(char *parseLine);

// Функция возвращает число с нужным номером из строки где числа разделены запятыми
int getCommaSeparateValueInt(char *line, int n);

// Функции для работы со строками вида "параметр=значение"
void getParameterName(char *parseLine);
void getParameterValue(char *parseLine);


#endif // _PARSEHELPER_H_
