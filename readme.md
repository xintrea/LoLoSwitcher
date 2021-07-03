LoLo Switcher v0.x
================
The low level keyboard language switcher for X11

### First, install required packages:

    - libpcre              (or libpcre3)
    - libpcre-dev          (or libpcre3-dev)
    - libpthread-stubs     (or libpthread-stubs0)
    - libpthread-stubs-dev (or libpthread-stubs0-dev)
    - libx11-dev
    - build-essential

### For compile, run commands:
```
cmake .
make
```

### For install, run by root:
```
make install
```

### For start switcher, run command:
```
/usr/sbin/loloswitcher
```

Note! For get low level data from device file, set owner for 
LoLoSwitcher binary file as root:root and set SUID bit with chmod command:
```
chown root:root /usr/sbin/loloswitcher
chmod u+s /usr/sbin/loloswitcher
```

Check LoLoSwitcher binary file state:
```
ls -l /usr/sbin/loloswitcher
```
```
Output: $ -rwsr-xr-x 1 root root 39888 Jan 11 13:53 /usr/sbin/loloswitcher
```

For autostart LoLo Switcher, write starting command to Desktop
Enviroment (DE) autostart config area, or put this command
to file `~/.xprofile`

### For get help and other option, run:
```
/usr/sbin/loloswitcher -h
```

### For edit configuration, use configfile:
```
~/.config/loloswitcher/config.ini
```

By default, LoLo Switcher is configured to switch two languages:

    - First language:
    Left Shift clean press 
    (clean push and release Left Shift without any other keys or modifier)

    - Second language:
    Right Shift clean press 
    (clean push and release Right Shift without any other keys or modifier)

    ... and when switch languages, PC Speaker is produces short tone sound with different frequence for each language.
