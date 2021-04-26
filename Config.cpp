#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "Config.h"
#include "ParseHelper.h"


Config::Config()
{
    // Очистка имени файла конфига
    sprintf(configFileName, "");

    init();
}


Config::~Config()
{

}


// Сброс переменных конфига
void Config::init(void)
{
    // Файл устройства клавиатуры
    sprintf(inputDevice, "");

    allowWaitDeviceConnect=0;
    allowDeviceReconnect=0;
    deviceReconnectTime=0;

    // Количество языковых раскладок
    numberOfLayout=0;

    // Метод переключения
    switchMethod=0;

    // Первичный фильтр
    sprintf(eventFilter.regexp, "");
    memset(eventFilter.regexpCompile, 0, REGEXP_COMPILE_SIZE);

    for(int i=0; i<CODES_LAYOUT_NUMBER; i++)
    {
        // Очищаются данные регулярных выражений
        sprintf(sequences[i].regexp, "");
        memset(sequences[i].regexpCompile, 0, REGEXP_COMPILE_SIZE);

        // Очищаются строки команд
        sprintf(commands[i].cmdText, "");
    }
}


// Задание имени файла конфига
// Конфигу можно отдельно указать, с каким файлом ему работать
void Config::setFileName(const char *fileName)
{
    // Запоминается имя файла конфига
    sprintf(configFileName, fileName);
}


// Получение имени файла конфига
const char * Config::getFileName()
{
    return static_cast<const char *>( configFileName );
}


// Чтение ранее указанного файла конфига
bool Config::readFile()
{
    // Если имя файла конфига было не задано
    if(strlen(configFileName)==0)
    {
        return false;
    }

    return this->readFile(configFileName);
}


// Чтение указанного файла конфига
bool Config::readFile(const char *fileName)
{
    init();

    // Открытие файла
    FILE *uk;
    if((uk = fopen (fileName, "rt")) == NULL) // Если файл не был открыт
    {
        return false;
    }


    // Здесь файл открыт на чтение

    // Запоминается имя файла
    sprintf(configFileName, fileName);


    // Считывается файл
    char readline[STRING_LEN];
    char tmpline[STRING_LEN];
    char tmpline2[STRING_LEN];
    bool remflag,eqflag;

    // Чтение файла по строкам
    while(!feof(uk))
    {
        // Обнуление строки
        readline[0]='\0';

        // Чтение строки
        fgets(readline, STRING_LEN-1, uk);

        // Для безопасности последующей работы со считанными строками
        readline[STRING_LEN-1]=0;

        // Убираются ведущие и концевые пробелы
        alltrim(readline);

        // Проверка, не является ли строка комментарием
        remflag=false;
        if(readline[0]=='#')
        {
            remflag=true;
        }

        // Поиск символа равенства
        eqflag=false;
        if(strchr(readline, '=')!=NULL)
        {
            eqflag=true;
        }

        // Если строка содержит присвоение и не является комментарием
        if(eqflag==true && remflag!=true)
        {
            // Получение имени параметра
            strcpy(tmpline, readline);
            getparamname(tmpline);

            // Получение значения параметра
            strcpy(tmpline2, readline);
            getparamvalue(tmpline2);

            /*
            printf("readline: %s\n", readline);
            printf("tmpline : %s\n", tmpline);
            printf("tmpline2: %s\n", tmpline2);
            */

            // Загрузка переменной InputDevice
            if(strcmp(tmpline, "InputDevice")==0)
            {
                sprintf(inputDevice, "%s", tmpline2);

                if(strlen(inputDevice)==0)
                {
                    printf("Detect empty value for inputDevice\n");
                    exit(1);
                }
            }


            // Загрузка переменной AllowWaitDeviceConnect
            if(strcmp(tmpline, "AllowWaitDeviceConnect")==0)
            {
                allowWaitDeviceConnect=atoi(tmpline2);

                if(allowWaitDeviceConnect<0 || allowWaitDeviceConnect>1)
                {
                    printf("Illegal value for AllowWaitDeviceConnect: %d\n", allowWaitDeviceConnect);
                    exit(1);
                }
            }


            // Загрузка переменной AllowDeviceReconnect
            if(strcmp(tmpline, "AllowDeviceReconnect")==0)
            {
                allowDeviceReconnect=atoi(tmpline2);

                if(allowDeviceReconnect<0 || allowDeviceReconnect>1)
                {
                    printf("Illegal value for AllowDeviceReconnect: %d\n", allowDeviceReconnect);
                    exit(1);
                }
            }


            // Загрузка переменной DeviceReconnectTime
            if(strcmp(tmpline, "DeviceReconnectTime")==0)
            {
                deviceReconnectTime=atoi(tmpline2);

                if(deviceReconnectTime<0 || deviceReconnectTime>10)
                {
                    printf("Illegal value for DeviceReconnectTime: %d\n", deviceReconnectTime);
                    exit(1);
                }
            }


            // Загрузка переменной DeviceType
            if(strcmp(tmpline, "DeviceType")==0)
            {
                deviceType=atoi(tmpline2);

                if(deviceType<0 || deviceType>1)
                {
                    printf("Illegal value for DeviceType: %d\n", deviceType);
                    exit(1);
                }
            }

            // Загрузка переменной NumberOfLayout
            if(strcmp(tmpline, "NumberOfLayout")==0)
            {
                numberOfLayout=atoi(tmpline2);

                if(numberOfLayout<1 || numberOfLayout>CODES_LAYOUT_NUMBER)
                {
                    printf("Illegal value for NumberOfLayout: %d\n", numberOfLayout);
                    exit(1);
                }
            }


            // Загрузка переменной SwitchMethod
            if(strcmp(tmpline, "SwitchMethod")==0)
            {
                switchMethod=atoi(tmpline2);

                if(switchMethod<0 || switchMethod>1)
                {
                    printf("Illegal value for switchMethod: %d\n", switchMethod);
                    exit(1);
                }
            }


            // Загрузка переменной EventFilter
            if(strcmp(tmpline, "EventFilter")==0)
            {
                // Запоминается текст регулярного выражения
                sprintf(eventFilter.regexp, tmpline2);

                // Компилируется регулярное выражение
                regexpCompiling(eventFilter.regexp, eventFilter.regexpCompile);
            }


            // Загрузка переменных Sequence и Command
            for(int n=0; n<CODES_LAYOUT_NUMBER && n<numberOfLayout; n++)
            {
                // Формируется имя переменной
                char varName[STRING_LEN];
                sprintf(varName, "Sequence%d", n);

                // Считывание переменной Sequence
                if(strcmp(tmpline, varName)==0)
                {
                    // Запоминается текст регулярного выражения
                    sprintf(sequences[n].regexp, tmpline2);

                    // Компилируется регулярное выражение
                    regexpCompiling(sequences[n].regexp, sequences[n].regexpCompile);
                }


                // Загрузка переменной Command
                sprintf(varName, "Command%d", n);

                // Считывание переменной
                if(strcmp(tmpline, varName)==0)
                {
                    sprintf(commands[n].cmdText, tmpline2); // Запоминается текст команды
                }

            } // Закрылся цикл загрузки переменных Sequence
        }
    };

    fclose(uk);

    return true;
}

// Компилирование регулярного выражения во внутреннее представление
void Config::regexpCompiling(char *regexpText, char *regexpCompileData)
{
    int options = 0;
    const char *error;
    int erroffset;
    pcre *regexp;

    regexp=pcre_compile(regexpText, options, &error, &erroffset, NULL);

    // Если компиляция была неудачной
    if(!regexp)
    {
        printf("Problem RegExp compile: '%s'\n", regexpText);
        printf("Error: %s\n", error);
        exit(1);
    }


    // Сохранение скомпилированного регулярного выражения
    int size, sizeResult;
    sizeResult = pcre_fullinfo(regexp, NULL, PCRE_INFO_SIZE, &size);

    if(sizeResult<0)
    {
        printf("Error if call pcre_fullinfo() for get size, %s\n", regexpText);
        exit(1);
    }

    if(size>=REGEXP_COMPILE_SIZE)
    {
        printf("Compile data for regexp too long, %d, %s\n", size, regexpText);
        exit(1);
    }

    // Скомпилированное выражение заносится в память
    memcpy(regexpCompileData, (char *)regexp, size);
}


// Метод, распечатывающий на экран значения конфига
void Config::print(void)
{
    printf("\n");
    printf("-------------------\n");
    printf("Reading config data from file %s\n", configFileName);
    printf("-------------------\n");

    printf("InputDevice: %s\n", inputDevice);

    printf("AllowWaitDeviceConnect: %d\n", allowWaitDeviceConnect);
    printf("AllowDeviceReconnect: %d\n", allowDeviceReconnect);
    printf("DeviceReconnectTime: %d\n", deviceReconnectTime);

    printf("DeviceType: %d\n", deviceType);
    printf("NumberOfLayout: %d\n", numberOfLayout);
    printf("SwitchMethod: %d\n", switchMethod);
    printf("EventFilter: %s\n", eventFilter.regexp);

    // Перебираются языковые слои
    for(int n=0; n<CODES_LAYOUT_NUMBER && n<numberOfLayout; n++)
    {
        printf("Sequence%d: %s\n", n, sequences[n].regexp);
    }

    // Перебираются языковые слои
    for(int n=0; n<CODES_LAYOUT_NUMBER && n<numberOfLayout; n++)
    {
        printf("Command%d: %s\n", n, commands[n].cmdText);
    }

    printf("\n");
}


void Config::printStandartConfig()
{
    // Печать стандартного содержимого конфиг-файла в открытый файловый дескриптор
    this->printStandartConfigToFileDescriptor(stdout);
}


// Метод создания стандартного конфига
void Config::createStandartConfig()
{
    char dirName[STRING_LEN];

    sprintf(dirName, "%s/.config", getUserDirectory());
    createDirIfNotExists(dirName, getUserName());

    sprintf(dirName, "%s/.config/loloswitcher", getUserDirectory());
    createDirIfNotExists(dirName, getUserName());

    // Открытие файла для записи
    char fileName[STRING_LEN];
    sprintf(fileName, "%s/.config/loloswitcher/config.ini", getUserDirectory());

    FILE *uk;
    if((uk = fopen (fileName, "wt")) == NULL)
    {
        // Если файл не был открыт
        printf("Error! Can not create file %s.\n", fileName);
        return;
    }

    // Печать стандартного содержимого конфиг-файла в открытый файловый дескриптор
    this->printStandartConfigToFileDescriptor(uk);

    // Файл закрывается
    fclose(uk);

    // Устанавливается правильный хозяин файла
    // Это нужно из-за того, что при запуске от sudo хозяин
    // ставится как root а не как пользователь
    chown(fileName, getuid(), getgid());
}


// Стандартное содержимое конфиг-файла
void Config::printStandartConfigToFileDescriptor(FILE *uk)
{
    fputs("# LoLo Switcher config file\n",                                            uk);
    fputs("\n",                                                                       uk);

    fputs("# Config version (do not edit this parameter!)\n",                         uk);
    fputs("ConfigVersion=1\n",                                                        uk);
    fputs("\n",                                                                       uk);

    fputs("# Input device\n",                                                         uk);
    fputs("# simple: /dev/input/event0\n",                                            uk);
    fputs("# but recommendet set by ID: /dev/input/by-id/usb-SIGMACH1P_USB_Keykoard-event-kbd\n",               uk);
    fputs("# for keep USB connection if keyboard moving to another USB slot or new USB device occupy eventX\n", uk);
    fputs("InputDevice=/dev/input/event0\n",                                          uk);
    fputs("\n",                                                                       uk);

    fputs("# Allow waiting for device to connect at LoLo Switcher start\n",           uk);
    fputs("# 0 - no waiting\n",                                                       uk);
    fputs("# 1 - waiting on (strong recommendet for KVM-switch)\n",                   uk);
    fputs("AllowWaitDeviceConnect=1\n",                                               uk);
    fputs("\n",                                                                       uk);

    fputs("# Allow device reconnection\n",                                            uk);
    fputs("# 0 - disable\n",                                                          uk);
    fputs("# 1 - enable (used for KVM-switch or bad USB cable, if you see trouble at dmesg)\n", uk);
    fputs("AllowDeviceReconnect=1\n",                                                 uk);
    fputs("\n",                                                                       uk);

    fputs("# Device reconnection time, sec\n",                                        uk);
    fputs("# Value from 0 to 10\n",                                                   uk);
    fputs("# If set 0, there will be high load on the system\n",                      uk);
    fputs("DeviceReconnectTime=3\n",                                                  uk);
    fputs("\n",                                                                       uk);

    fputs("# Type of device\n",                                                       uk);
    fputs("# 0 - keyboard\n",                                                         uk);
    fputs("# 1 - joystick\n",                                                         uk);
    fputs("DeviceType=0\n",                                                           uk);
    fputs("\n",                                                                       uk);

    fputs("# Total number of language layout\n",                                      uk);
    fputs("# For example:\n",                                                         uk);
    fputs("# if you use ENG and RUS, set 2\n",                                        uk);
    fputs("# if you use ENG, RUS and UKR, set 3\n",                                   uk);
    fputs("NumberOfLayout=2\n",                                                       uk);
    fputs("\n",                                                                       uk);

    fputs("# Language switch method\n",                                               uk);
    fputs("# 0 - cyclic switch\n",                                                    uk);
    fputs("# 1 - direct switch\n",                                                    uk);
    fputs("SwitchMethod=1\n",                                                         uk);
    fputs("\n",                                                                       uk);

    fputs("# RegExp for primary filtering device events\n",                           uk);
    fputs("# For classic keyboard, set value to ^1,[0-9]+,[0-9]+;$\n",                uk);
    fputs("# and this filter enabled only KeyPress, KeyHold and KeyRelease event.\n", uk);
    fputs("# Before research device codes (with option -t1), clear this value.\n",    uk);
    fputs("EventFilter=^1,[0-9]+,[0-9]+;$\n",                                         uk);
    fputs("\n",                                                                       uk);

    fputs("# RegExp with codes for switch language layouts\n",                        uk);
    fputs("# If you use cyclic switch method, set variable Sequence0 only.\n",        uk);
    fputs("# If you use direct switch method, set variable SequenceX\n",              uk);
    fputs("# for each layout (numeric from 0).\n",                                    uk);
    fputs("Sequence0=(?<!1,29,1;|1,29,2;|1,97,1;|1,97,2;|1,56,1;|1,56,2;|1,100,1;|1,100,2;)1,42,1;1,42,0;$\n", uk);
    fputs("Sequence1=(?<!1,29,1;|1,29,2;|1,97,1;|1,97,2;|1,56,1;|1,56,2;|1,100,1;|1,100,2;)1,54,1;1,54,0;$\n", uk);
    fputs("\n",                                                                       uk);

    fputs("# Bash command if language layout switched (optional).\n",                 uk);
    fputs("# Set CommandX for each layout (numeric from 0).\n",                       uk);
    fputs("Command0=beep -f 440 -l 25\n",                                             uk);
    fputs("Command1=beep -f 520 -l 25\n",                                             uk);
}


// Проверка и содание каталога если его еще нет
void Config::createDirIfNotExists(char *dirName, char *userName)
{
    struct stat status;

    if(stat(dirName, &status)!=0)
    {
        // Создается каталог
        printf("Try create directory %s\n", dirName);
        int result=mkdir(dirName, 0755);

        if(result==0)
        {
            printf("Directory create OK.\n");
        }
        else
        {
            printf("Error if directory create.\n");
        }

        // Устанавливается правильный хозяин директории
        // Это нужно из-за того, что при запуске от sudo хозяин
        // ставится как root а не как пользователь
        chown(dirName, getuid(), getgid());
    }
}


// Выяснение имя каталога пользователя
char *Config::getUserDirectory()
{
    static char userHomeDir[STRING_LEN];

    char *userHome;

    userHome=getenv("HOME");
    if(userHome==NULL)
    {
        printf("Enviroment variable HOME is not set. Program was closed.\n");
        exit(1);
    }

    // Безопасное копирование значения
    strncpy(userHomeDir, userHome, STRING_LEN-1);
    userHomeDir[STRING_LEN-1]=0;

    return userHomeDir;
}


// Выяснение имени пользователя
char *Config::getUserName()
{
    static char userName[STRING_LEN];

    char *value;

    value=getenv("USER");
    if(value==NULL)
    {
        printf("Enviroment variable USER is not set. Program was closed.\n");
        exit(1);
    }

    // Безопасное копирование значения
    strncpy(userName, value, STRING_LEN-1);
    userName[STRING_LEN-1]=0;

    return userName;
}


char *Config::getInputDevice()
{
    return inputDevice;
}


int Config::getAllowWaitDeviceConnect()
{
    return allowWaitDeviceConnect;
}

int Config::getAllowDeviceReconnect()
{
    return allowDeviceReconnect;
}


int Config::getDeviceReconnectTime()
{
    return deviceReconnectTime;
}


int Config::getDeviceType()
{
    return deviceType;
}


int Config::getNumberOfLayout()
{
    return numberOfLayout;
}


int Config::getSwitchMethod()
{
    return switchMethod;
}


char *Config::getEventFilter()
{
    return eventFilter.regexp;
}


char *Config::getEventFilterCompile()
{
    return eventFilter.regexpCompile;
}


char *Config::getSequence(int n)
{
    if(n<0 or n>=CODES_LAYOUT_NUMBER)
    {
        return NULL;
    }

    return sequences[n].regexp;
}


char *Config::getSequenceCompile(int n)
{
    if(n<0 or n>=CODES_LAYOUT_NUMBER)
        return NULL;

    return sequences[n].regexpCompile;
}


char *Config::getCommand(int n)
{
    if(n<0 or n>=CODES_LAYOUT_NUMBER)
        return NULL;

    return commands[n].cmdText;
}
