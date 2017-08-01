#ifndef _KEYLAYOUT_H_
#define _KEYLAYOUT_H_

#include <X11/Xlib.h>
#include <X11/XKBlib.h>


class KeyLayout
{
public:
 KeyLayout();
 ~KeyLayout();

 void init();

 int getLayout(); // Номер текущей раскладки
 void setLayout(int n); // Установка нужной раскладки
 int getLayoutNumber(); // Общее количество раскладок
 void print();

protected:

 int getActiveGroup();
 int setActiveGroup(int group);
 int getLayouts(char **names);
 void freeLayouts(char **names, int gc);

 Display *display;
};

#endif // _KEYLAYOUT_H_
