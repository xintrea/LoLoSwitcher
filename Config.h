#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "Main.h"
#include <pcre.h>


#define CURRENT_CONFIG_VERSION 2 // Актуальная версия конфиг-файла

#define CODES_EVENT_VARIABLE 3  // Количество чисел, кодирующих одно событие клавиатуры
#define CODES_EVENT_LENGTH   16 // Возможное количество событий клавиатуры в буфере
#define CODES_LAYOUT_NUMBER  10 // Возможное количество раскладок клавиатуры

#define EVENT_ARRAY_SIZE 64

#define REGEXP_COMPILE_SIZE 4096
#define REGEXP_OVECTOR_SIZE 64

#define CONFIG_DEVICE_TYPE_KEYBOARD 0
#define CONFIG_DEVICE_TYPE_JOYSTICK 1

#define CONFIG_SWITCH_METHOD_CYCLIC 0
#define CONFIG_SWITCH_METHOD_DIRECT 1


struct REGEXPDATA
{
    char regexp[STRING_LEN]; // Текст регулярного выражения
    char regexpCompile[REGEXP_COMPILE_SIZE]; // Скомпилированный образ регулярного выражения
};


struct CMDDATA
{
    char cmdText[STRING_LEN]; // Текст команды
};


class Config
{
public:
    Config();
    ~Config();

    void init(void);

    void setFileName(const char *fileName);
    const char *getFileName();

    bool readFile();
    bool readFile(const char *fileName);

    void createStandartConfig();
    void print(void);
    void printStandartConfig();

    // Методы получения значений из конфига
    char *getInputDevice();
    int getAllowWaitDeviceConnect();
    int getAllowDeviceReconnect();
    int getDeviceReconnectTime();
    int getDeviceType();
    int getNumberOfLayout();
    int getSwitchMethod();
    char *getEventFilter();
    char *getEventFilterCompile();
    char *getSequence(int n);
    char *getSequenceCompile(int n);
    char *getCommand(int n);

    char *getUserDirectory();
    char *getUserName();

protected:

    void regexpCompiling(char *regexpText, char *regexpCompileData);
    void createDirIfNotExists(char *dirName, char *userName);
    void printStandartConfigToFileDescriptor(FILE *uk);

    void updateVersion(const char *fileName, const int versionFrom, const int versionTo);
    bool readAllValues(const char *fileName);
    bool updateValue(const char *fileName, const char *name, const char *value);

    // Переменные, в которых хранится конфигурация программы
    int configVersion;
    char inputDevice[STRING_LEN]; // Строка с именем устройства ввода
    int allowWaitDeviceConnect; // Разрешено ли ожидание появления устройства при старте LLS
    int allowDeviceReconnect; // Разрешено ли переподключение устройства в процессе работы
    int deviceReconnectTime; // Переодичность переустановки соединения с устройством, сек
    int deviceType; // Тип устройства
    int numberOfLayout; // Количество раскладок
    int switchMethod; // Метод переключения
    REGEXPDATA eventFilter; // Первичный фильтр событий
    REGEXPDATA sequences[CODES_LAYOUT_NUMBER]; // Строки с регулярными выражениями
    CMDDATA commands[CODES_LAYOUT_NUMBER]; // Строки с командами

    // Имя файла, из которого считывается конфигурация
    char configFileName[STRING_LEN];

};

#endif // _CONFIG_H_
