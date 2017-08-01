#ifndef _ACCESSCONTROL_H_
#define _ACCESSCONTROL_H_

class AccessControl
{
public:
 AccessControl();
 ~AccessControl();

 void accessUp();
 void accessDown();

 int getUID(void);
 int getGID(void);

 void print(void);

protected:

 void init(void);

 int accessLevel;

 int uid; // ID пользователя, от которого запущена программа
 int euid; // Эффективный ID, при запуске с SUID он равен ID рута

 int gid;
 int egid;

};

#endif // _ACCESSCONTROL_H_
