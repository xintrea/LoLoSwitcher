/*
LoLo Switcher

The low level keyboard language switcher for X11

Copyright 2011 Sergey M. Stepanov
Contact: xintrea@gmail.com, www.webhamster.ru
Licenses: GPL v.3, BSD

Russia, Volgodonsk, 2011
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

#include <string>
#include <vector>

#include "Main.h"
#include "Config.h"
#include "EventQueue.h"
#include "KeyLayout.h"
#include "AccessControl.h"

#define RUN_MODE_UNDEFINED       0
#define RUN_AS_PROCESS           1
#define RUN_HELP                 2
#define RUN_VERSION              3
#define RUN_TEST_KEY_CODE        4
#define RUN_TEST_LANGUAGE_LAYOUT 5
#define RUN_TEST_CONFIG_READ     6
#define RUN_SHOW_STANDART_CONFIG 7

#define SLEEP_INTERVAL           5


// Представление универсального события для клавиатуры и джойстика
struct universal_event 
{
    int a;
    int b;
    int c;
};


// Прототипы функций на случай, если вызов идет раньше объявления
void run(const int mode);
void readConfig(void);
void runTestKeyCode(void);
void runTestLanguageLayout(void);
void runTestConfigRead(void);
void runAsProcess(void);
void runHelp(void);
void runVersion(void);
void runShowStandartConfig(void);
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
    std::vector<std::string> args(argv+1, argv+argc);

    // Режим работы программы
    int mode=RUN_MODE_UNDEFINED;

    // Если нет опций
    if(args.size()==0)
        mode=RUN_AS_PROCESS; // Запуск как процесс
    else
    {
        // Иначе есть опции

        // printf("argv[0]: %s\n", argv[0]);
        // printf("argv[1]: %s\n", argv[1]);

        for(auto i=args.begin(); i!=args.end(); ++i)
        {
            // Если нужно показать текст помощи
            if(*i=="-h" || *i=="--help")
            {
                mode=RUN_HELP;
                break; // Больше никакие опции на этот режим не влияют
            }

            // Если нужно показать номер версии
            if(*i=="-v" || *i=="--version")
            {
                mode=RUN_VERSION;
                break; // Больше никакие опции на этот режим не влияют
            }

            // Если нужно показать содержимое стандартного конфига
            if(*i=="-i")
            {
                mode=RUN_SHOW_STANDART_CONFIG;
                break; // Больше никакие опции на этот режим не влияют
            }

            // Если нужно запустить тестирование кодов клавиатуры
            if(*i=="-t1")
            {
                mode=RUN_TEST_KEY_CODE;
            }

            // Если нужно запустить тестирование обнаруженных раскладок
            if(*i=="-t2")
            {
                mode=RUN_TEST_LANGUAGE_LAYOUT;
            }

            if(*i=="-t3")
            {
                mode=RUN_TEST_CONFIG_READ;
            }

            if(*i=="-c")
            {
                std::string configFileName=*++i;
                config.setFileName( configFileName.c_str() );

                if(mode==RUN_MODE_UNDEFINED)
                {
                    mode=RUN_AS_PROCESS;
                }
            }
        }
    }


    // Если режим работы программы не был определен
    if(mode==RUN_MODE_UNDEFINED)
    {
        printf("Error program parameters\n");
        return 1;
    }


    // Запуск
    run(mode);
}


// Запуск в вычисленном режиме
void run(const int mode)
{
    switch(mode)
    {
        case RUN_AS_PROCESS:
            runAsProcess();
            break;

        case RUN_HELP:
            runHelp();
            break;

        case RUN_VERSION:
            runVersion();
            break;

        case RUN_SHOW_STANDART_CONFIG:
            runShowStandartConfig();
            break;

        case RUN_TEST_KEY_CODE:
            runTestKeyCode();
            break;

        case RUN_TEST_LANGUAGE_LAYOUT:
            runTestLanguageLayout();
            break;

        case RUN_TEST_CONFIG_READ:
            runTestConfigRead();
            break;

        default:
            printf("Unavailable program mode\n");
    }
}


// Считывание конфига в глобальный объект конфига
void readConfig(void)
{
    bool configReadResult=false;

    // Если имя файла конфига было задано через опцию
    if( strlen(config.getFileName())!=0 )
    {
        configReadResult=config.readFile( config.getFileName() );
    }
    else
    {
        // Имя файла не задано через опцию, и надо искать конфиг
        // в возможных местополежениях

        // Вначале делается попытка считать конфиг из текущей директории
        configReadResult=config.readFile( (char *)"./config.ini" );


        // Если в текущей директории конфига нет
        // То делается попытка считать из стандартной директории, отведенной для программы
        char fileName[STRING_LEN]; // Имя конфиг-файла в директории пользователя
        sprintf(fileName, "%s/.config/loloswitcher/config.ini", config.getUserDirectory());

        if(!configReadResult)
        {
            configReadResult=config.readFile(fileName);
        }

        // Если нигде нет конфига, создается и считывается стандартный конфиг
        if(!configReadResult)
        {
            config.createStandartConfig();
            configReadResult=config.readFile(fileName);
        }

    }

    // Если никаким способом не удалось считать конфиг
    if(!configReadResult)
    {
        printf("Config file can not be read.\n");
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
    printf("-i           - show standart config file with initial settings\n");
    printf("-t1          - run event code test\n");
    printf("-t2          - run language layout test\n");
    printf("-t3          - run read config test\n");
    printf("-c fileName  - specifying the required config file\n");
    printf("\n");
}


// Номер версии
void runVersion(void)
{
    printf("LoLo Switcher v.%d.%d\n", PROGRAM_VERSION, PROGRAM_SUBVERSION);
    printf("\n");
}


// Печать содержимого стандартного конфига
void runShowStandartConfig(void)
{
    config.printStandartConfig();
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
        struct universal_event ev[EVENT_ARRAY_SIZE];

        int ev_count=getUniversalEvents(fd, ev); // Считываются события
        if (errno == ENODEV)
        {
            // Устройство ввода отвалилось, ожидание некоторого времени и попытка переподключения
            printf("Device not found. Try reconnect...\n");

            close(fd);

            do
            {
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
                {
                    eventShow=false;
                }

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
        struct input_event keyboard_ev[EVENT_ARRAY_SIZE];

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
        struct js_event joystick_ev[EVENT_ARRAY_SIZE];

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
    {
        return true;
    }

    // Проверка регулярного выражения EventFilter
    int count = 0;
    int ovector[REGEXP_OVECTOR_SIZE];
    count=pcre_exec( (pcre *)config.getEventFilterCompile(), NULL, text, strlen(text), 0, 0, ovector, REGEXP_OVECTOR_SIZE);

    // printf("Count=%d\n", count);

    return count>0; // Если что-то было найдено, возвращается true
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
    printf("Config file reading test...\n");

    // Загрузка конфига
    // Если конфиг невозможно прочитать, произойдет завершение программы
    readConfig();

    // Печать конфига (конфиг во всей программе доступен как глобальный объект)
    config.print();

    printf("Config file success read.\n");
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
        struct universal_event ev[EVENT_ARRAY_SIZE];

        int ev_count=getUniversalEvents(fd, ev); // Считываются события
        if (errno == ENODEV)
        {
            // Устройство ввода отвалилось, ожидание некоторого времени и попытка переподключения
            close(fd);

            do
            {
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
                    {
                        break;
                    }

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
        {
            number=0;
        }

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
        printf("Couldn't open device file %s for input device\n", filename);
        printf("The device is not connected or permission denied\n", filename);
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
    {
        printf("Can't run command: %s\n", cmd);
    }

    // Ожидается завершение процесса с выполняемой командой и процесс завершается
    pclose(processPointer);

    // pthread_exit(NULL);

    return NULL;
}

