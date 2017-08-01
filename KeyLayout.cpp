#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "KeyLayout.h"


KeyLayout::KeyLayout()
{
 init();
}


KeyLayout::~KeyLayout()
{

}


void KeyLayout::init() 
{
 // Подключение к рабочему дисплею
 display = XOpenDisplay(NULL);

 if( display == NULL ) 
  {
   printf("Error XOpenDisplay");
   exit(1);
  }
}


// Получение номера раскладки, которая активна в данный момент
int KeyLayout::getActiveGroup() 
{
 XkbStateRec state[1];
 memset(state, 0, sizeof(state));

 XkbGetState(display, XkbUseCoreKbd, state);

 return state->group;
}


// Переключение раскладки
// dpy - идентификатор дисплея
// group - номер раскладки (обычно порядковый номер 
//         в списке включенных раскладок, счет с 0
int KeyLayout::setActiveGroup(int group) 
{
 // Отправка запроса на переключение раскладки
 XkbLockGroup( display, XkbUseCoreKbd, group );

 // Вызов XkbGetState() для выполнения запроса,
 // без этого вызова переключение раскладки не сработает
 XkbStateRec state[1];
 memset(state, 0, sizeof(state));
 XkbGetState(display, XkbUseCoreKbd, state);
}


// Получение списка доступных раскладок клавиатуры
// Массив **names заполняется именами раскладок
// Функция возвращает количество раскладок
int KeyLayout::getLayouts(char **names) 
{
 XkbDescRec desc[1];
 int gc;
 memset(desc, 0, sizeof(desc));
 desc->device_spec = XkbUseCoreKbd;
 XkbGetControls(display, XkbGroupsWrapMask, desc);
 XkbGetNames(display, XkbGroupNamesMask, desc);
 XGetAtomNames(display, desc->names->groups, gc = desc->ctrls->num_groups, names);
 XkbFreeControls(desc, XkbGroupsWrapMask, True);
 XkbFreeNames(desc, XkbGroupNamesMask, True);
 return gc;
}


// Очистка массива имен доступных раскладок
// Функция получает указатель на массив имен
// И количество доступных раскладок
void KeyLayout::freeLayouts(char **names, int gc) 
{
 for (; gc--; ++names)
  if (*names) 
   {
    XFree(*names);
    *names = NULL;
   }
}


void KeyLayout::print() 
{
 // Массив имен раскладок
 char *names[XkbNumKbdGroups];

 // Заполнение массива имен раскладок
 int gc=getLayouts(names);

 // Получение номера активной раскладки
 int g=getActiveGroup();

 // Печать информации о доступных раскладках
 for(int i = 0; i < gc; i++)
  printf("%d - %s %s\n", i, names[i], i == g ? "(Active)" : "");
 
 // Очистка массива имен раскладок
 freeLayouts(names, gc);
}


// Получение номера текущей раскладки
int KeyLayout::getLayout() 
{
 return getActiveGroup();
}


// Включение указанной раскладки
void KeyLayout::setLayout(int n)
{
 setActiveGroup(n);
}


// Количество раскладок
int KeyLayout::getLayoutNumber()
{
 XkbDescRec desc[1];

 memset(desc, 0, sizeof(desc));
 desc->device_spec = XkbUseCoreKbd;
 XkbGetControls(display, XkbGroupsWrapMask, desc);

 return desc->ctrls->num_groups;
}
