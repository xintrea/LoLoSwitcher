 
LoLo Switcher v.0.x

The low level keyboard language switcher for X11


* First, install required packages:

  * libpcre              (or libpcre3)
  * libpcre-dev          (or libpcre3-dev)
  * libpthread-stubs     (or libpthread-stubs0)
  * libpthread-stubs-dev (or libpthread-stubs0-dev)


* For compile, run commands:

  cmake .    (Yes, with dot at end.)
  make


* For install, run by root:

  make install


* For running switcher, run command:

  /usr/sbin/loloswitcher

  Note! For get low level data from device file, running LoLoswitcher with
  SUID bit. Check SUID bit with chmod command.

  For autostart LoLo Switcher, write this command to Desktop Enviroment (DE)
  autostart config area, or put this command to file ~/.xprofile


* For get help and other option, run:

  /usr/sbin/loloswitcher -h


* For edit configuration, use configfile:

  ~/.config/loloswitcher/config.ini

  By default, LoLo Switcher is configured to switch two languages:

  Left Shift Released - first language
  Right Shift Released - second language

  ... and when switch languages, PC Speaker is produces short tone sound.
