#ifndef _EVENTQUEUE_H_
#define _EVENTQUEUE_H_

#include <pcre.h>

#include "Config.h"


struct EVENTELEMENT
{
 int type;
 int code;
 int value;
};


class EventQueue
{
public:
 EventQueue();
 ~EventQueue();

 void init(void);
 void add(int type, int code, int value);
 void print();

 bool checkSequence(char *regexpCompile);

private:

 // Данные очереди
 EVENTELEMENT evQueue[CODES_EVENT_LENGTH];

};


#endif // _EVENTQUEUE_H_
