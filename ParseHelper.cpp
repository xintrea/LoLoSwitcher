#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "Main.h"
#include "ParseHelper.h"


// Функция удаляет пробелы в начале и конце принятой строки
void alltrim(char *parseline)
{
 int i,j,k,pos=0;

 if(parseline==NULL)
  {
   printf("Function alltrim() call with NULL parameter!\n");
   return;
  }

 // Определяется индекс последнего символа в строке
 j=strlen(parseline)-1;

 // Если строка пустая и нуливой длины, ее обрабатывать ненужно
 if(j<=0)
  return;

 // Пропуск пробелов в начале строки
 for (i = 0; isspace(parseline[i]); i++);

 // Пропуск символов переноса строк в конце строки
 for (; ((parseline[j]=='\n') || (parseline[j]=='\r') || (parseline[j]==0)); j--);

 // Пропуск пробелов в конце строки
 for (; isspace(parseline[j]); j--);

 // Подготовка результата.
 // Такая конструкция возможна, т.к. строка может смещаться только влево
 for(k=i; k<=j; k++)
  parseline[pos++]=parseline[k];
 parseline[pos]=0;
}


// Функция возвращает число с нужным номером из строки где числа разделены запятыми
// Нумерация выбираемого элемента идет с нуля
int get_comma_separate_valuei(char *line, int n)
{
 const char *delimeters= (char *) ",";
 char *ptr;
 char tmpline[STRING_LEN];

 // logprint("Function get_comma_separate_valuei() '%s' '%d'\n",line,n);

 // Переданная строка копируется в рабочую строку чтобы ее не испортила функция strtok()
 strcpy(tmpline, line);

 // Взятие первого элемента
 ptr=strtok(tmpline, delimeters);

 // Если запрошен первый элемент
 if(n==0)
  {
   // Если первый элемент (индекс 0) нормально обнаружен
   if(ptr) return atoi(ptr);
   else
    {
     // printf("In get_comma_separate_valuei() first index not found\n");
     return 0;
    }
  }

  int i=1;

  // Далее цикл если номер запрошенного элемента был вторым (индекс 1) или больше
  while(ptr)
   {
    ptr=strtok(NULL, delimeters);

    // Если указатель номально определен на данном этапе
    // и выбранная подстрока имеет нужный индекс
    if(ptr)
     if(i==n)
      return atoi(ptr);

    i++;
   }

  // printf("In get_comma_separate_valuei() index %d not found\n",i);
  return 0;
}


// Функция взятия значения параметра из строки вида
// параметр=значение
// значение возвращается как строка
void getparamvalue(char *parseline)
{
  char tmpline[STRING_LEN];
  char *p;

  strcpy(tmpline, parseline);

  // Поиск позиции символа равенства
  p=strchr(tmpline, '=');

  // Если указатель на символ равенства не равен NULL, значит символ найден
  if(p!=NULL)
   {
    p++;
    strcpy(parseline, p); // Копирование результата в принятый массив
    return;
   }

  parseline[0]=0;
  return;
}


// Функция взятия имени параметра из строки вида
// параметр=значение
// значение возвращается как строка
void getparamname(char *parseline)
{
  for(int i=0; i<=strlen(parseline); i++)
   if(parseline[i]=='=')
    {
     parseline[i]=0; // На месте знака равенства устанавливается признак конца строки
     return;
    }

  parseline[0]=0;
  return;
}

