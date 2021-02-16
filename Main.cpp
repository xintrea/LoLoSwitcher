/*
LoLo Switcher

The low level keyboard language switcher for X11
Copyright 2011 Sergey M. Stepanov
Contact: xintrea@gmail.com, www.webhamster.ru
Licenses: GPL v.3, BSD
Volgodonsk, 2011
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <linux/joystick.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <errno.h>

#include "Main.h"
#include "Config.h"
#include "EventQueue.h"
#include "KeyLayout.h"
#include "AccessControl.h"

// 0.01 - Первая рабочая версия

// 0.02 - Основана на версии 0.01
//      - Переделан механизм поиска паттерна кодов 
//        с простой маски на регулярное выражение

// 0.03 - Основана на версии 0.02
//      - Сделано кеширование скомпилированных регулярных выражений
//        в первом приближении

// 0.04 - Основана на версии 0.03
//      - Подключение к дисплею вынесено в отдельный метод класса KeyLayout
//      - Изменен механизм циклического переключения раскладок,
//        теперь вместо статической переменной используется
//        определение номера текущей раскладки. Это нужно для
//        одновременной работы с другими менеджерами 
//        управления раскладками

// 0.05 - Основана на версии 0.04
//      - Добавлены команды установки хозяина на создаваемый
//        дефолтный каталог и файл конфигурации. Это необходимо
//        так как LoLo Switcher может быть запущен из-под sudo,
//        после чего обычный пользователь на сможет редактировать
//        файл конфигурации

// 0.06 - Основана на версии 0.05
//      - Сделано схлопывание одинаковых кодов в очереди событий
//      - Сделана блокировка переключения на конкретный язык, 
//        если он уже установлен (при прямом методе переключения)

// 0.07 - Основана на версии 0.06
//      - В конфиг добавлены конфигурирующие параметры CommandX 
//        которые содержат Bash команды, которые срабатывают при
//        переключении на новый язык
//      - Запуск Bash команд сделан в отдельных тредах
//      - В стандартный конфиг добавлены строки с Bash командами
//        которые издают звук на PC Seaker при переключении раскладки

// 0.08 - Основана на версии 0.07
//      - Добавлено создание каталога ~/.config в том случае если его
//        нет в системе         
//      - Добавлены опции показа номера версии

// 0.09 - Основана на версии 0.08
//      - Исправлен механизм определения текущей раскладки и возможности
//        переключения на другую. Теперь нет статической переменной
//        которая следила за номером раскладки, номер раскладки 
//        определяется динамически

// 0.10 - Основана на версии 0.09
//      - Доработано кеширование регулярных выражений согласно
//        документации в pcreprecompile

// 0.11 - Основана на версии 0.10
//      - Сделано правильное подключение к дисплею X11 один раз
//        при старте программы, а не каждый раз при использовании
//        функций X11, которое приводило к ошибке
//        Maximum number of clients reachedError XOpenDisplay
//      - Сделана чистка кода, чтобы не было предупреждений 
//        о прекращении использования типа char* в строковых C-функциях

// 0.12 - Основана на версии 0.11
//      - При выполнении команд, прописанных в опциях CommandX,
//        теперь при запуске LoLo Switcher с установленным SUID, если
//        запуск делается от обычного пользователя, происходит переключение 
//        эффективного ID на идентификатор обычного пользователя, 
//        который и запустил программу. Это нужно для того, чтобы
//        обычный пользователь не имел возможности выполнять произвольные
//        команды с правами рута

// 0.13 - Основана на версии 0.12
//      - Добавлено считывание переменной EventFilter из конфига
//      - Переделана фильрация событий с константы, прописанной в коде,
//        на проверку регулярного выражения, заданного 
//        в переменной EventFilter

// 0.14 - Основана на версии 0.13
//      - В метод печати теста считывания конфига (по опции -t3)
//        добавлен вывод содержимого переменной EventFilter
//      - В файл CMakeLists.txt добавлены опции инсталляции
//      - Добавлен файл postinstall.cmake, который необходим для
//        правильного завершения инсталляции

// 0.15 - Основана на версии 0.14
//      - Сделана структура универсального события universal_event, чтобы
//        можно было обрабатывать разные типы устройств
//      - В конфиг добавлена переменна DeviceType, указыващая тип
//        устройства - клавиатура или джойстик
//      - Опрос устройства ввода был вынесен в отдельную
//        функцию getUniversalEvents(), в ней реализовано заполнение
//        массива универсальных событий либо событиями клавиатуры,
//        либо событиями джойстика

// 0.16 - Основана на версии 0.15
//      - Исправлена кодировка некоторых *.cpp и *.h файлов,
//        которые имели cp1251 вместо UTF-8
//      - В CMakeLists.txt задана опция компилирования -O0 для
//        файла EventQueue.cpp, так как в нем gcc генерирует
//        неправильный цикл обратного обхода очереди для микропроцессора AMD

// 0.17 - Основана на версии 0.16
//      - В функции отдельного потока threadFunc() добавлено закрытие
//        потока bash-команды с помощью вызова pclose()
//      - Изменен механиз вызова функции отдельного потока. Сделана правильная
//        передача номера выполняемой команды, чтобы небыло потенциальной
//        порчи стека. Добавлена устновка атрибутов потока в
//        значение PTHREAD_CREATE_DETACHED, чтобы ресурсы потока после
//        его завершения были освобождены для использования новыми потоками.

// 0.18 - Основана на версии 0.17
//      - Удален файл postinstall.cmake, его функционал внесен в CMakeLists.txt
//      - В CMakeLists.txt выставлен режим компиляции Release

// 0.19 - Основана на версии 0.18
//      - Программа проверена с помощью valgrind, сделаны небольшие доработки,
//        теперь при работе в любом режиме нет ни одной ошибки 
//        в инструменте проверки Memcheck

// 0.20 - Основана на версии 0.19
//      - Релизная версия. Сделано два варианта: версия с исходниками
//        и версия с скомпилированным бинарником, доработаны readme-файлы

// 0.21 - Основана на версии 0.20
//      - Внесены изменения для компиляции на 64-х битных платформах

// 0.22 - Основана на версии 0.21
//      - Выделен отдельный класс AccessControl, в котором происходят
//        действия по повышению и понижению привилегий
//      - Сделана инверсия переключения привилегий. Раньше программа
//        работала с высокими привилегиями, понижая уровень доступа только
//        при выполнении bash-команд, заданных в опциях CommandX.
//        Теперь программа постоянно работает на низком уровне привилегий,
//        повышая уровень доступа только в моменты считывания данных
//        из файла устройства ввода

// 0.23 - Основана на версии 0.22
//      - Сделано определение STRING_LEN который задает стандартную длину
//        C-строк во всей программе. Магические константы во всех классах 
//        заменены на это определение
//      - Доработан Config.cpp, улучшена работа со строками при парсинге
//        данных в конфиг-файле

// 0.24 - Основана на версии 0.23
//      - Проверка на доступ к защищенным ресурсам
//      - Обнаружено, что инверсия переключения привилегий имеет побочный
//        эффект. Момент, когда выполняется bash-команда в отдельном
//        треде, может наложиться на момент повышения привилегий
//        при считывании данных из файла устройства. Так как треды имеют
//        общее адресное пространство и атрибуты выполнения, то изменение
//        уровня привилегий отражается как на основном потоке, так и на
//        порожденном потоке. Инверсия переключения привелегий исключена
//        из кода
//      - Доработки в ParseHelper.cpp 
//        Доработан метод get_comma_separate_valuei()
//        Упрощены методы getparamvalue() и getparamname()

// 0.25 - Основана на версии 0.24
//      - В ParseHelper.cpp добавлено подключение заголовка Main.h
//      - В функциях ParseHelper.cpp все магические константы 
//        с длинной строки заменены на STRING_LEN
//      - Проверка на доступ к защищенным ресурсам
//      - Проверка через valgring, проверка через htop, релиз

// 0.26 - Основана на версии 0.25
//      - В Config.cpp удалены вызовы popen, используемые для запуска
//        bash-команд chown для установки правильных прав на каталог конфигурации
//        и файл конфигурации в случае, если их нет в системе. Вместо них
//        теперь используется C-функция chown()
//      - В Config::getUserDirectory() и Config::getUserName() считывание 
//        значений переменных окружения в методах через функцию getenv() сделано
//        с ограничением на длину строки. Затем в конец области строки 
//        добавляется 0
//      - В Config::readFile() считанная из файла конфигурации строка 
//        для безопасности сразу подвергается обработке, в конец области 
//        считанной строки добавляется 0

// 0.27 - Основана на версии 0.26
//      - Исправлен выход из программы в случае получения обрывочного события
//        клавиатуры и джойстика. Такое поведение замечено в OpenSUSE 12.1 и
//        в Debian Testing (Wheezy) в среде KDE 4.8.x
//      - В режим теста кодов устройства (опция -t1) добавлено пояснение, что
//        нужно настроить параметр InputDevice в том случае, если не видно
//        кодов нажатых клавиш

// 0.28 - Основана на версии 0.27
//      - Исправлена кодировка некоторых файлов-исходников, разрабатываемых в
//        эмулирующемся редакторе FAR Editor
//      - В стандартном конфиге добавлено пояснение, что лучше прописывать
//        параметр InputDevice по идентификатору, а не просто как eventX.
//        Это необходимо, чтобы LoLoSwitcher всегда работал, даже если клавиатура 
//        была переключена в другой USB-слот, или eventX заняло другое 
//        USB-устройство,вставленное до запуска компьютера 

// 0.29 - Основана на версии 0.28
//      - Добавлена возможность переподключения устройства клавиатуры
//        в случае, если во время работы LoLoSwitcher клавиатура была вытащена
//        и снова воткнута в гнездо

// 0.30 - Основана на версии 0.29
//      - Исправлен сегфолт, появляющийся при компиляции в gcc 9.2.x,
//        которого в версиях gcc 4.7.x не было

// 0.31 - Основана на версии 0.30
//      - Расширена возможность обработки сложных RegExp
//      - Сделано выравниванивание на границу слова для размеров массивов,
//        содержащих строки
//      - Доработан readme файл, сделана более подробная инструкция по
//        установке программы


#define RUN_MODE_UNDEFINED       0
#define RUN_AS_PROCESS           1
#define RUN_HELP                 2
#define RUN_VERSION              3
#define RUN_TEST_KEY_CODE        4
#define RUN_TEST_LANGUAGE_LAYOUT 5
#define RUN_TEST_CONFIG_READ     6

#define SLEEP_INTERVAL           5

// Представление универсального события для клавиатуры и джойстика
struct universal_event 
{
 int a;
 int b;
 int c;
};

// Прототипы функций на случай, если вызов идет раньше объявления
void readConfig(void);
void runTestKeyCode(void);
void runTestLanguageLayout(void);
void runTestConfigRead(void);
void runAsProcess(void);
void runHelp(void);
void runVersion(void);
int getUniversalEvents(int fd, struct universal_event *ev);
bool checkEventFilter(char *text);
void switchLayout(int n);
void executeCommand(int n);
int openInputFile(char* filename, bool permissive);
static void *threadFunc(void *arg);

// Глобальный объект с конфигурацией программы
Config config;

// Очередь с событиями клавиатуры
EventQueue eventQueue;

// Объект для работы с раскладками клавиатуры
KeyLayout keyLayout;

// Объект для контроля за уровнем доступа программы
AccessControl accessControl;


int main (int argc, char *argv[]) 
{
 // Определение режима работы программы согласно опциям
 int mode=RUN_MODE_UNDEFINED;

 // Если нет опций
 if(argc < 2)
  mode=RUN_AS_PROCESS; // Запуск как процесс 
 else
  {
   // Иначе есть опции

   // printf("argv[0]: %s\n", argv[0]);
   // printf("argv[1]: %s\n", argv[1]);

   // Если нужно показать текст помощи
   if(strcmp(argv[1], "-h")==0 || strcmp(argv[1], "--help")==0)
    mode=RUN_HELP;

   // Если нужно показать номер версии
   if(strcmp(argv[1], "-v")==0 || strcmp(argv[1], "--version")==0)
    mode=RUN_VERSION;

   // Если нужно запустить тестирование кодов клавиатуры
   if(strcmp(argv[1], "-t1")==0)
    mode=RUN_TEST_KEY_CODE;

   // Если нужно запустить тестирование обнаруженных раскладок
   if(strcmp(argv[1], "-t2")==0)
    mode=RUN_TEST_LANGUAGE_LAYOUT;

   if(strcmp(argv[1], "-t3")==0)
    mode=RUN_TEST_CONFIG_READ;
  }

 // Если режим работы программы не был определен
 if(mode==RUN_MODE_UNDEFINED)
  {
   printf("Error program parameters\n");
   return 1;
  }


 if(mode==RUN_AS_PROCESS)
  runAsProcess();

 if(mode==RUN_HELP)
  runHelp();

 if(mode==RUN_VERSION)
  runVersion();

 if(mode==RUN_TEST_KEY_CODE)
  runTestKeyCode();

 if(mode==RUN_TEST_LANGUAGE_LAYOUT)
  runTestLanguageLayout();

 if(mode==RUN_TEST_CONFIG_READ)
  runTestConfigRead();
}


// Считывание конфига в глобальный объект конфига
void readConfig(void)
{
 // Имя конфиг-файла в директории пользователя
 char fileName[STRING_LEN];
 sprintf(fileName, "%s/.config/loloswitcher/config.ini", config.getUserDirectory());
 
 bool configReadResult=false; 

 // Вначале делается попытка считать конфиг из текущей директории
 configReadResult=config.readFile( (char *)"./config.ini");

 // Если в текущей директории конфига нет
 // То делается попытка считать из стандартной директории, отведенной для программы
 if(!configReadResult)
  configReadResult=config.readFile(fileName);

 // Если нигде нет конфига, создается и считывается стандартный конфиг
 if(!configReadResult)
  {
   config.createStandartConfig();
   configReadResult=config.readFile(fileName);
  }

 // Если никаким способом не удалось считать конфиг
 if(!configReadResult)
  {
   printf("Cant read config file.\n");
   exit(1);
  }

 // config.print();
}


// Помощь
void runHelp(void)
{
 runVersion();

 printf("Usage:\n");
 printf("Without parameter - run standart LoLo Switcher as service\n");
 printf("-h or --help - show this help\n");
 printf("-t1 - run event code test\n");
 printf("-t2 - run language layout test\n");
 printf("-t3 - run read config test\n");
 printf("\n");
}


// Номер версии
void runVersion(void)
{
 printf("LoLo Switcher v.%d.%d\n", PROGRAM_VERSION, PROGRAM_SUBVERSION);
 printf("\n");
}


// Запуск теста кодов клавиатуры при нажатии клавиш
void runTestKeyCode(void)
{
 // Чтение конфига
 readConfig();
 
 printf("Event code test.\n");
 printf("If you see typed symbols without codes, you should\n");
 printf("configure parameter InputDevice in your config.ini file.\n");
 if( strlen(config.getEventFilter()) > 0)
  printf("Note! This codes with previous filtration by EventFilter value: '%s'\n", config.getEventFilter());
 printf("Type CTRL+C for exit.\n");
 printf("\n");

 // Имя файла устройства ввода
 char inputDeviceFileName[STRING_LEN];
 strcpy(inputDeviceFileName, config.getInputDevice());

 // Открытие устройства ввода
 int fd=openInputFile(inputDeviceFileName, false);

 int eventCount=1;


 // Цикл опроса устройства ввода
 while(true) 
  {
   // Массив событий
   struct universal_event ev[64];

   int ev_count=getUniversalEvents(fd, ev); // Считываются события
   if (errno == ENODEV)
    {
      printf("Device not found. Try reconnect...\n");

      // Устройство ввода отвалилось, ожидание 5 секунд и попытка переподключения
      close(fd);
      
      do {
       sleep(SLEEP_INTERVAL);
       fd=openInputFile(inputDeviceFileName, true);
      }
      while (fd < 0);

      printf("Device reconnect success\n");
      
      continue;
    }

   // Перебираются события 
   for(int i = 0; i < ev_count; i++) 
    {
     char eventText[STRING_LEN];
     sprintf(eventText, "%d,%d,%d;\n", ev[i].a, ev[i].b, ev[i].c);

     // Применение фильтра событий
     if( checkEventFilter(eventText) ) 
      {
       bool eventShow=true;

       // Не нужно показывать событие отжатия ENTER в первом
       // полученном событии, так как оно остается после нажатия ENTER
       // при запуске программы в консоли
       if(eventCount==1 && ev[i].a==1 && ev[i].b==28 && ev[i].c==0)
        eventShow=false;

       if(eventShow)
        {
         printf("[%d]:\n", eventCount);
         printf("%s\n", eventText);
         printf("\n");

         eventCount++;
        }
      }
    } // Закрылся цикл перебора событий клавиатуры
  
  } // Закрылся бесконечный цикл опроса устройства ввода

}


// Считывание событий с устройства ввода
// fd - дескриптор открытого файла устройства
// ev - указатель на массив событий
int getUniversalEvents(int fd, struct universal_event *ev)
{
 int ev_count;
 
 // Обработка клавиатуры
 if(config.getDeviceType()==CONFIG_DEVICE_TYPE_KEYBOARD)
  {
   // Массив событий клавиатуры
   struct input_event keyboard_ev[64]; 

   // Считываются события
   ssize_t rb = read(fd, keyboard_ev, sizeof(keyboard_ev));

   // Проверяется результат считывания
   if(rb < (int) sizeof(struct input_event)) 
    {
     // В KDE 4.8.x в дистрибутивах OpenSUSE 12.1 и Debian Testing (Wheezy)
     // разломан обработчик клавиатурных событий, поэтому завершать Loloswitcher
     // не нужно, а нужно просто пропустить сбойное событие
     /* 
     printf("Short input keyboard device read\n");
     exit(1);
     */

     return 0; // 0 обозначает, что никаких событий не было
    }

   ev_count=(int) (rb / sizeof(struct input_event));

   for(int i = 0; i < ev_count; i++) 
    {
     ev[i].a=keyboard_ev[i].type;
     ev[i].b=keyboard_ev[i].code;
     ev[i].c=keyboard_ev[i].value;
    }
  }


 // Обработка джойстика
 if(config.getDeviceType()==CONFIG_DEVICE_TYPE_JOYSTICK)
  {
   // Массив событий джойстика
   struct js_event joystick_ev[64]; 

   // Считываются события
   ssize_t rb = read(fd, joystick_ev, sizeof(joystick_ev));

   // Проверяется результат считывания
   if(rb < (int) sizeof(struct js_event)) 
    {
     /*
     printf("Short input joystick device read\n");
     exit(1);
     */

     return 0; // 0 обозначает, что никаких событий не было
    }

   ev_count=(int) (rb / sizeof(struct js_event));

   for(int i = 0; i < ev_count; i++) 
    {
     ev[i].a=joystick_ev[i].type;
     ev[i].b=joystick_ev[i].number;
     ev[i].c=joystick_ev[i].value;
    }
  }


 return ev_count;
}


// Проверка кода события на фильтрующее регулярное выражение
// text - код события
bool checkEventFilter(char *text)
{
 // Если фильтр не задан, любое событие всегда проходит
 if(strlen(config.getEventFilter())==0)
  return true;
 
 // Проверка регулярного выражения EventFilter
 int count = 0;
 int ovector[REGEXP_OVECTOR_SIZE];
 count=pcre_exec( (pcre *)config.getEventFilterCompile(), NULL, text, strlen(text), 0, 0, ovector, REGEXP_OVECTOR_SIZE);

 // printf("Count=%d\n", count);

 if(count<=0)
  return false; // Если ничего не было найдено
 else
  return true; // Что-то было найдено

}


void runTestLanguageLayout(void)
{
 printf("Language layout test.\n");
 printf("Layout found:\n");
 printf("\n");

 keyLayout.print();
}


void runTestConfigRead(void)
{
 printf("Read config test.\n");

 // Загрузка конфига
 readConfig();

 // Печать конфига (конфиг во всей программе доступен как глобальный объект)
 config.print();
}


// Запуск программы в основном режиме
void runAsProcess(void)
{
 // Чтение конфига
 readConfig();

 // Инициализация очереди событий клавиатуры
 eventQueue.init();

 // Имя файла устройства ввода
 char inputDeviceFileName[STRING_LEN];
 strcpy(inputDeviceFileName, config.getInputDevice());

 // Открытие устройства ввода
 int fd=openInputFile(inputDeviceFileName, false);

 // Цикл опроса устройства ввода
 while(true) 
  {
   // Массив событий
   struct universal_event ev[64];

   int ev_count=getUniversalEvents(fd, ev); // Считываются события
   if (errno == ENODEV)
    {
      // Устройство ввода отвалилось, ожидание 5 секунд и попытка переподключения
      close(fd);
      do {
       sleep(SLEEP_INTERVAL);
       fd=openInputFile(inputDeviceFileName, true);
      }
      while (fd < 0);

      continue;
    }

   // Перебираются события 
   for(int i = 0; i < ev_count; i++) 
    {
     char eventText[STRING_LEN];
     sprintf(eventText, "%d,%d,%d;\n", ev[i].a, ev[i].b, ev[i].c);

     // Применение фильтра событий
     if( checkEventFilter(eventText) ) 
      {
       /*
       printf(": type, code, value\n");
       printf("%d, %d, %d\n", ev[i].type, ev[i].code, ev[i].value);
       printf("\n");
       */

       // Событие добавляется в очередь
       eventQueue.add(ev[i].a, ev[i].b, ev[i].c);

       // Перебираются раскладки, для которых нужно обнаруживать последовательности
       // событий в очереди
       for(int layoutCount=0; layoutCount<config.getNumberOfLayout(); layoutCount++)
        {
         // printf("RegExp: '%s'\n", config.getSequence(layoutCount));

         // Для циклического переключения раскладок проверяется 
         // только первая последовательность
         if(config.getSwitchMethod()==0 && layoutCount>=1)
          break;
         
         // Проверка последовательности
         if(eventQueue.checkSequence( config.getSequenceCompile(layoutCount) )==true)
          {
           // printf("Detect codes sequence %d\n", layoutCount);

           // Запускается переключение раскладки
           switchLayout(layoutCount);
          }
          
        } // Закрылся цикл перебора раскладок

       
      }
    }
  }

}


// Переключение раскладки
void switchLayout(int n)
{
 // Если нужно циклическое переключение раскладок
 if(config.getSwitchMethod()==CONFIG_SWITCH_METHOD_CYCLIC)
  {
   int number=keyLayout.getLayout();

   number++;

   if(number>=keyLayout.getLayoutNumber())
    number=0;

   keyLayout.setLayout(number);
   executeCommand(number);

   return;
  }

 // Если нужно прямое переключение раскладок
 if(config.getSwitchMethod()==CONFIG_SWITCH_METHOD_DIRECT)
  {
   int number=keyLayout.getLayout();

   if(number!=n)
    {
     keyLayout.setLayout(n);
     executeCommand(n);
    }
  }

}


// Выполнение нужной Bash-команды
void executeCommand(int n)
{
 // Если команда не задана
 if(strlen(config.getCommand(n))==0)
  return;

 size_t keyboardLayerNum=(size_t) n;

 // Выполнение команды будет происходить в отдельном потоке
 pthread_t thread;

 // Настройка атрибутов потока
 pthread_attr_t threadAttr; 
 pthread_attr_init(&threadAttr); 
 pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_DETACHED); 

 // int result = pthread_create(&thread, NULL, threadFunc, &keyboardLayerNum);
 int result = pthread_create(&thread, &threadAttr, threadFunc, (void*)keyboardLayerNum);
 
 if(result != 0) 
  {
   printf("Creating command thread false. Error: %d\n", result);
   exit(1);
  }

}


// Открытие устройства ввода
int openInputFile(char* filename, bool permissive)
{
 int fd=open(filename, O_RDONLY);

 if(!permissive && fd < 0)
  {
   printf("Couldn't open input device %s, may be permission denied\n", filename);
   exit(1);
  }

 return fd;
}


static void *threadFunc(void *arg)
{
 // int keyboardLayerNum = * (int *) arg;
 int keyboardLayerNum = (int) (size_t) arg;

 char cmd[STRING_LEN];

 sprintf(cmd, config.getCommand(keyboardLayerNum));

 // printf("Run command %s\n", cmd);

 FILE *processPointer; 

 // Команда выполняется
 accessControl.accessDown();
 processPointer=popen(cmd, "r");
 accessControl.accessUp();
 
 if( processPointer == NULL)
  printf("Can't run command: %s\n", cmd);

 // Ожидается завершение процесса с выполняемой командой и процесс завершается
 pclose(processPointer);

 // pthread_exit(NULL);

 return NULL;
}

