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
    configVersion=0;

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
    this->init();

    // Определение версии конфига
    if( !this->readAllValues(fileName) )
    {
        return false;
    }

    // Если конфиг устаревший, он обновляется
    if(configVersion!=CURRENT_CONFIG_VERSION)
    {
        this->updateVersion(fileName, configVersion, CURRENT_CONFIG_VERSION);

        // Перечитывается конфиг-файл
        if( !this->readAllValues(fileName) )
        {
            return false;
        }
    }

    // Запоминается имя файла
    sprintf(configFileName, fileName);

    return true;
}


bool Config::readAllValues(const char *fileName)
{
    // Открытие файла
    FILE *uk;
    if((uk = fopen (fileName, "rt")) == NULL) // Если файл не был открыт
    {
        return false;
    }

    // Считывается файл
    char readLine[STRING_LEN];
    char tmpLine[STRING_LEN];
    char tmpLine2[STRING_LEN];
    bool remFlag, eqFlag;

    // Чтение файла по строкам
    while(!feof(uk))
    {
        // Обнуление строки
        readLine[0]='\0';

        // Чтение строки
        fgets(readLine, STRING_LEN-1, uk);

        // Для безопасности последующей работы со считанными строками
        readLine[STRING_LEN-1]=0;

        // Убираются ведущие и концевые пробелы
        allTrim(readLine);

        // Проверка, не является ли строка комментарием
        remFlag=false;
        if(readLine[0]=='#')
        {
            remFlag=true;
        }

        // Поиск символа равенства
         eqFlag=false;
        if(strchr(readLine, '=')!=NULL)
        {
             eqFlag=true;
        }

        // Если строка содержит присвоение и не является комментарием
        if( eqFlag==true && remFlag!=true)
        {
            // Получение имени параметра
            strcpy(tmpLine, readLine);
            getParameterName(tmpLine);

            // Получение значения параметра
            strcpy(tmpLine2, readLine);
            getParameterValue(tmpLine2);

            /*
            printf("readline: %s\n", readline);
            printf("tmpline : %s\n", tmpline);
            printf("tmpline2: %s\n", tmpline2);
            */


            // Загрузка переменной ConfigVersion
            if(strcmp(tmpLine, "ConfigVersion")==0)
            {
                configVersion=atoi(tmpLine2);

                if(configVersion<=0 or configVersion>CURRENT_CONFIG_VERSION)
                {
                    printf("Detect incorrect value for ConfigVersion\n");
                    exit(1);
                }
            }

            // Загрузка переменной InputDevice
            if(strcmp(tmpLine, "InputDevice")==0)
            {
                sprintf(inputDevice, "%s", tmpLine2);

                if(strlen(inputDevice)==0)
                {
                    printf("Detect empty value for InputDevice\n");
                    exit(1);
                }
            }


            // Загрузка переменной AllowWaitDeviceConnect
            if(strcmp(tmpLine, "AllowWaitDeviceConnect")==0)
            {
                allowWaitDeviceConnect=atoi(tmpLine2);

                if(allowWaitDeviceConnect<0 || allowWaitDeviceConnect>1)
                {
                    printf("Illegal value for AllowWaitDeviceConnect: %d\n", allowWaitDeviceConnect);
                    exit(1);
                }
            }


            // Загрузка переменной AllowDeviceReconnect
            if(strcmp(tmpLine, "AllowDeviceReconnect")==0)
            {
                allowDeviceReconnect=atoi(tmpLine2);

                if(allowDeviceReconnect<0 || allowDeviceReconnect>1)
                {
                    printf("Illegal value for AllowDeviceReconnect: %d\n", allowDeviceReconnect);
                    exit(1);
                }
            }


            // Загрузка переменной DeviceReconnectTime
            if(strcmp(tmpLine, "DeviceReconnectTime")==0)
            {
                deviceReconnectTime=atoi(tmpLine2);

                if(deviceReconnectTime<0 || deviceReconnectTime>10)
                {
                    printf("Illegal value for DeviceReconnectTime: %d\n", deviceReconnectTime);
                    exit(1);
                }
            }


            // Загрузка переменной DeviceType
            if(strcmp(tmpLine, "DeviceType")==0)
            {
                deviceType=atoi(tmpLine2);

                if(deviceType<0 || deviceType>1)
                {
                    printf("Illegal value for DeviceType: %d\n", deviceType);
                    exit(1);
                }
            }

            // Загрузка переменной NumberOfLayout
            if(strcmp(tmpLine, "NumberOfLayout")==0)
            {
                numberOfLayout=atoi(tmpLine2);

                if(numberOfLayout<1 || numberOfLayout>CODES_LAYOUT_NUMBER)
                {
                    printf("Illegal value for NumberOfLayout: %d\n", numberOfLayout);
                    exit(1);
                }
            }


            // Загрузка переменной SwitchMethod
            if(strcmp(tmpLine, "SwitchMethod")==0)
            {
                switchMethod=atoi(tmpLine2);

                if(switchMethod<0 || switchMethod>1)
                {
                    printf("Illegal value for SwitchMethod: %d\n", switchMethod);
                    exit(1);
                }
            }


            // Загрузка переменной EventFilter
            if(strcmp(tmpLine, "EventFilter")==0)
            {
                // Запоминается текст регулярного выражения
                sprintf(eventFilter.regexp, tmpLine2);

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
                if(strcmp(tmpLine, varName)==0)
                {
                    // Запоминается текст регулярного выражения
                    sprintf(sequences[n].regexp, tmpLine2);

                    // Компилируется регулярное выражение
                    regexpCompiling(sequences[n].regexp, sequences[n].regexpCompile);
                }


                // Загрузка переменной Command
                sprintf(varName, "Command%d", n);

                // Считывание переменной
                if(strcmp(tmpLine, varName)==0)
                {
                    sprintf(commands[n].cmdText, tmpLine2); // Запоминается текст команды
                }

            } // Закрылся цикл загрузки переменных Sequence
        }
    };

    fclose(uk);

    return true;
}


// Действия над файлом конфига, которые надо произвести для обновления версии
void Config::updateVersion(const char *fileName, const int versionFrom, const int versionTo)
{
    for(int v=versionFrom+1; v<=versionTo; ++v)
    {
        // printf("Update config file %s to version %d ...\n", fileName, v);

        // Действия для обновления на версию 2
        if(v==2)
        {
            // Открытие файла
            FILE *uk;
            if((uk = fopen (fileName, "at")) == NULL) // Если файл не был открыт
            {
                printf("Can not open config file %s for update to version %d\n", fileName, v);
                return;
            }

            fputs("\n", uk);
            fputs("# Auto append for config version 2\n", uk);
            fputs("AllowWaitDeviceConnect=1\n", uk);
            fputs("AllowDeviceReconnect=1\n", uk);
            fputs("DeviceReconnectTime=3\n", uk);

            fclose(uk);
        }

        // Обновление номера версии конфига в файле
        char versionString[STRING_LEN];
        sprintf(versionString, "%d", v);
        this->updateValue(fileName, "ConfigVersion", versionString);

        // printf("Update to version %d successfull\n", v);
    }
}


// Функция обновляет значение в конфиг-файле (но не в представлении конфига в памяти)
bool Config::updateValue(const char *fileName, const char *name, const char *value)
{
    char toFileName[STRING_LEN];
    sprintf(toFileName, "%s.new", fileName);
    unlink(toFileName);

    FILE *fromFile; // Исходный файл конфига
    FILE *toFile; // Новый файл конфига

    // Открытие конфига на чтение
    if((fromFile = fopen (fileName, "rt")) == NULL) // Если файл не был открыт
    {
        printf("Can not open config file %s for update value %s\n", fileName, name);
        return false;
    }

    // Открытие нового конфига на запись
    if((toFile = fopen (toFileName, "at")) == NULL) // Если файл не был открыт
    {
        printf("Can not open config file %s for update value %s\n", fileName, name);
        return false;
    }

    // Считывается исходный файл
    char readLine[STRING_LEN];
    char tmpLine[STRING_LEN];
    char tmpLine2[STRING_LEN];
    char resultLine[STRING_LEN];
    bool remFlag, eqFlag;

    // Чтение исходного файла по строкам
    while(!feof(fromFile))
    {
        // Обнуление строки
        readLine[0]='\0';

        // Чтение строки
        fgets(readLine, STRING_LEN-1, fromFile);

        // Для безопасности последующей работы со считанными строками
        readLine[STRING_LEN-1]=0;

        // Убираются ведущие и концевые пробелы
        allTrim(readLine);

        // Проверка, не является ли строка комментарием
        remFlag=false;
        if(readLine[0]=='#')
        {
            remFlag=true;
        }

        // Поиск символа равенства
        eqFlag=false;
        if(strchr(readLine, '=')!=NULL)
        {
            eqFlag=true;
        }

        // Если строка содержит присвоение и не является комментарием
        if(eqFlag==true && remFlag!=true)
        {
            // Получение имени параметра
            strcpy(tmpLine, readLine);
            getParameterName(tmpLine);

            // Получение значения параметра
            strcpy(tmpLine2, readLine);
            getParameterValue(tmpLine2);

            // Если параметр является заменяемым
            if(strcmp(tmpLine, name)==0)
            {
                sprintf(resultLine, "%s=%s\n", name, value); // Параметр заменяется
            }
            else
            {
                sprintf(resultLine, "%s\n", readLine); // Параметр печатается как есть
            }
        }
        else
        {
            if(strcmp(readLine, "\n")==0)
            {
                sprintf(resultLine, "\n"); // Чтобы не выводилось два переноса если строка пустая
            }
            else
            {
                sprintf(resultLine, "%s\n", readLine); // Строка выводится без изменений
            }
        }

        fputs(resultLine, toFile);
    }

    fclose(fromFile);
    fclose(toFile);

    unlink(fileName);
    rename(toFileName, fileName);

    // Устанавливается правильный хозяин файла
    // Это нужно из-за того, что при запуске от sudo хозяин
    // ставится как root а не как пользователь
    chown(fileName, getuid(), getgid());

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
    fputs("ConfigVersion=2\n",                                                        uk);
    fputs("\n",                                                                       uk);

    fputs("# Input device\n",                                                         uk);
    fputs("# Example: /dev/input/event0\n",                                            uk);
    fputs("# But recommendet set by ID: /dev/input/by-id/usb-SIGMACH1P_USB_Keykoard-event-kbd\n",               uk);
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
    fputs("# 1 - enable (strong recommendet for KVM-switch)",                         uk);
    fputs("Note: if you have bad USB cable (or very long nonstandart USB-cable)",     uk);
    fputs("and you see trouble at dmesg, set this option to enable\n",                uk);
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
    {
        return NULL;
    }

    return sequences[n].regexpCompile;
}


char *Config::getCommand(int n)
{
    if(n<0 or n>=CODES_LAYOUT_NUMBER)
    {
        return NULL;
    }

    return commands[n].cmdText;
}
