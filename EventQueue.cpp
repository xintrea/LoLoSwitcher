#include <stdio.h>
#include <string.h>

#include "Main.h"
#include "EventQueue.h"


EventQueue::EventQueue()
{
 init();
}


EventQueue::~EventQueue()
{

}


void EventQueue::init(void)
{
 for(int i=0; i<CODES_EVENT_LENGTH; i++)
  {
   evQueue[i].type=0;
   evQueue[i].code=0;
   evQueue[i].value=0;
  }
}


void EventQueue::add(int type, int code, int value)
{
 // Если новый код отличается от последнего в очереди
 if(evQueue[0].type!=type ||
    evQueue[0].code!=code ||
    evQueue[0].value!=value)
  {
   // Очередь сдвигается к концу чтобы освободить нуливой (т.е. первый) элемент
   for(int i=CODES_EVENT_LENGTH-1; i>=1; i--)
    evQueue[i]=evQueue[i-1];

   // В нуливой элемент помещаются новые значения
   evQueue[0].type=type;
   evQueue[0].code=code;
   evQueue[0].value=value;
  }
}


void EventQueue::print()
{
 for(int i=0; i<CODES_EVENT_LENGTH; i++)
  printf("Queue [%d]: %d, %d, %d\n", i,
                                     evQueue[i].type,
                                     evQueue[i].code,
                                     evQueue[i].value);
}


// Проверяется, есть ли последовательность событий, заданная в строке поиска
bool EventQueue::checkSequence(char *regexpCompile)
{
 // Очередь преобразуется в строку (для последующего сравнения)
 char queueSequence[STRING_LEN];
 sprintf(queueSequence, "");

 // Перебор очереди с последнего элемента к первому
 for(int i=CODES_EVENT_LENGTH-1; i>=0; i--)
  {
   // Строка с событием из очереди
   char event[STRING_LEN];
   sprintf(event, "%d,%d,%d;", evQueue[i].type, evQueue[i].code, evQueue[i].value);
   
   // Добавление строки с событием
   sprintf(queueSequence, "%s%s", queueSequence, event);
  }

 // printf("Queue: '%s'\n", queueSequence);

 // Проверка регулярного выражения
 int count = 0;
 int ovector[30];
 count=pcre_exec( (pcre *)regexpCompile, NULL, queueSequence, strlen(queueSequence), 0, 0, ovector, 30);

 // printf("Count=%d\n", count);

 if(count<=0)
  return false; // Если ничего небыло найдено
 else
  return true; // Что-то было найдено
}

