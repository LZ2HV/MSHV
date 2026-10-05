/* MSHV
 *
 * By Hrisimir Hristov - LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */

#ifndef CPUWUDGET_H
#define CPUWUDGET_H

#include "../config.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QDir>
#include <QTimer>
#include "HvProgBarSlowH/hvprogbarslowh.h"
#include "cpusensorhv.h"

//#include <QtGui>

class CpuWudget_W : public QWidget
{
    Q_OBJECT
public:
    CpuWudget_W(int CpuN, int cpu_count,/*bool all_in_one,*/QWidget *parent = 0);
    virtual ~CpuWudget_W();

public slots:
    //void SetValue();
    void SetValueAllInOne();

private:
    //HvProgBarSlowH *THvProgBarSlowH;
    QLabel *l_cpu0;
    CpuSensorHv *TCpuSensorHv;
    int s_cpu_count;
    QList<CpuSensorHv *> ListCpuSensor;
    QList<HvProgBarSlowV *> ListProgBarV;
};

class CpuWudget : public QWidget
{
    Q_OBJECT
public:
    CpuWudget(QWidget *parent = 0);
    virtual ~CpuWudget();
    int GetThrCount()
    {
    	if (s_cpu_c>0) return s_cpu_c;//protection    		
    	else return 1;	
   	}

signals:
    void Refresh();
    //void EmitCpuCount(int);

private:
	int s_cpu_c;
    void ScanForCpus();
    //void AddWidgets(int number);
    //void AddWidgetForAll(int cpu_count);
    QHBoxLayout *H_B;
    QTimer *timer_refreshcpu;


};
#endif
