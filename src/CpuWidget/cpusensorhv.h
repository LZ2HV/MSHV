/* MSHV
 *
 * By Hrisimir Hristov - LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */

#ifndef CPUSENSORHV_H
#define CPUSENSORHV_H

#include "../config.h"

//#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QRegExp>

#if defined _WIN32_
#include "windows.h"
#define MAX_PROC 34
typedef DWORD (__stdcall *LPFN_NtQuerySystemInformation)(DWORD, PVOID, DWORD, PDWORD);
#define SystemProcessorPerformanceInformation 0x8
#pragma pack(push,8)
typedef struct _SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION
{
    LARGE_INTEGER IdleTime;
    LARGE_INTEGER KernelTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER DpcTime;
    LARGE_INTEGER InterruptTime;
    ULONG InterruptCount;
}
SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION,
*PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION;
#pragma pack(pop)
extern int GetProc_Info_(SYSTEM_INFO &,LPFN_NtQuerySystemInformation &,SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION *,SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION *);
#endif

//#include <QtGui>

class CpuSensorHv //: public QObject
{
    //Q_OBJECT
public:
    CpuSensorHv(QString,int);
    virtual ~CpuSensorHv();

    int getCPULoad();

private:
#if defined _LINUX_
    long userTicks;
    long sysTicks;
    long niceTicks;
    long idleTicks;
    int user;
    int system;
    int nice;
    int idle;
    void getTicks(long &u,long &s,long &n,long &i);
#endif
    QString cpuNbr;
    int cpu_num;// for windows
#if defined _WIN32_
    SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION spi_old[MAX_PROC+2];
    SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION spi[MAX_PROC+2];
    SYSTEM_INFO   systeminfo;
    unsigned long bytesreturned;
    LPFN_NtQuerySystemInformation ntquerysysteminformation;
    int cpu_usage;
    bool f_function;
#endif

};
#endif




