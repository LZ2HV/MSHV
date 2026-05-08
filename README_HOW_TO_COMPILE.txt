The visual interface is QT4.8.6. 
All required libraries are compiled statically and are embedded in the body of the software. 
Additional libraries are not necessary. 
All settings and configurations happen immediately and do not need to be restarted, 
for example changing the sound settings or rig control. 

Requirements: 
MinGw recommended Tdm Gcc 4.9.2 thread model posix.
QT4.8.6 library.
Alsa library.

Above v 1.27 included option to compile with QT5 library.

How to compile above v 1.27:
Old method for configure Operating System is included in to *.pro files
and removed from /src/config.h file.
To configure QT library in file /src/config.h
uncomment line #define MSHV_QT4 if use QT4 
#define MSHV_QT4
//#define MSHV_QT5
or if use QT5 uncomment line  #define MSHV_QT5
//#define MSHV_QT4
#define MSHV_QT5

How to compile up to v1.26:
In file /src/config.h if using Windows uncomment line #define _WIN32_
#define _WIN32_
//#define _LINUX_
if Linux uncomment line  #define _LINUX_
//#define _WIN32_
#define _LINUX_

On Ubuntu 14.04 32bit and higher
In file MSHV_I686.pro change path /usr/lib/libasound.so to /usr/lib/i386-linux-gnu/libasound.so
On Ubuntu 14.04 64bit and higher
In file MSHV_x86_64.pro change path /usr/lib64/libasound.so to /usr/lib/x86_64-linux-gnu/libasound.so

example In file MSHV_x86_64.pro, for LM19.n and Ubuntu18, maybe 19 as well
from
LIBS = /usr/lib64/libasound.so src/Hv_Lib_fftw/lin_x86_64/libfftw3v337_gcc731_thread_posix.a
to
LIBS = /usr/lib/x86_64-linux-gnu/libasound.so /usr/lib/x86_64-linux-gnu/libfftw3.so 

In various Linux distributions please find your path to your libasound.so library and change it in
MSHV_I686.pro for 32bit and MSHV_x86_64.pro for 64bit.

Windows:
1. qmake.exe MSHV_WIN32.pro
2. make.exe
Linux:
1. qmake MSHV_I686.pro "for 32Bit" or MSHV_x86_64.pro "for 64Bit"
2. make

If have to clean and recompile:
Windows:
1. make.exe clean
2. qmake.exe MSHV_WIN32.pro
3. make.exe
Linux:
1. make clean
2. qmake MSHV_I686.pro "for 32Bit" or MSHV_x86_64.pro "for 64Bit"
3. make
