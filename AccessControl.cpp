#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "AccessControl.h"


AccessControl::AccessControl()
{
    init();
}


AccessControl::~AccessControl()
{

}


void AccessControl::init(void)
{
    // Вначале инитится флаг, что привелегии низкие
    accessLevel=0;

    // Выясняются ID пользователя и эффективный ID
    uid=getuid();
    euid=geteuid();

    gid=getgid();
    egid=getegid();

    // Проверяется, работает ли программа в режиме SUID
    if(uid != euid)
    {
        accessLevel=1; // Устанавливается, что доступны высокие привелегии
    }
}


int AccessControl::getUID(void)
{
    return uid;
}


int AccessControl::getGID(void)
{
    return gid;
}


// Повышение привелегий
void AccessControl::accessUp()
{
    // printf("\n%ld: Access up in %s\n", pthread_self(), place);
    // printf("%ld: Before access up:\n", pthread_self());
    // print();

    seteuid( euid );
    setegid( egid );

    accessLevel=1;

    // printf("%ld: After access up:\n", pthread_self());
    // print();
}

// Понижение привелегий
void AccessControl::accessDown()
{
    // printf("\n%ld: Access down in %s\n", pthread_self(), place);
    // printf("%ld: Before access down:\n", pthread_self());
    // print();

    seteuid( uid );
    setegid( gid );

    accessLevel=0;

    // printf("%ld: After access down:\n", pthread_self());
    // print();
}


void AccessControl::print(void)
{
    printf("%ld: Access level: %d \n", pthread_self(), accessLevel);
    printf("%ld: getuid()=%d, geteuid()=%d\n", pthread_self(), getuid(), geteuid());
}
