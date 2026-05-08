/* MSHV
 *
 * By Hrisimir Hristov - LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */

#ifndef HVSPINBOX_H
#define HVSPINBOX_H

#include <QWidget>
#include <QSpinBox>
#include <QTimer>
#include <QHBoxLayout>
#include "../HvButtons/hvbutton_left2.h"
class HvSpinBoxDt : public QWidget
{
    Q_OBJECT
public:
    HvSpinBoxDt(bool,QWidget *parent = 0);
    virtual ~HvSpinBoxDt();

    void SetFont(QFont);
    void SetHidden(bool f);
    void SetMode(int);         
signals:
    void EmitValueChanged(int);
public slots:
	
private: 
	bool s_indsty;
	int s_val;
	double val0;
    QTimer *tim;
    QDoubleSpinBox *sb_dt;
    HvButton_Left2 *bt_clr;
    void SetEmitValueChanged_p(int val);    
private slots:
    void Refresh();
    void SetValueChanged(double);
    void ClrDT();
protected:	
};

#include <QKeyEvent>
class HvSpinBox : public QSpinBox
{
    //Q_OBJECT
public:
    HvSpinBox(QWidget *parent = 0);
    virtual ~HvSpinBox();
    
//signals:

//public slots:

//private:

protected:
	void keyPressEvent(QKeyEvent*);

};
///////////////////////////////////////////////////////////////////
#include <QFont>
#include <QLabel>
class HvSpinBoxSn : public QWidget
{
    Q_OBJECT
public:
    HvSpinBoxSn(QString name,bool,QWidget *parent = 0);
    virtual ~HvSpinBoxSn();
    
    int Value();
    void SetValue(int);
    void SetRange(int);
    void SetFont(QFont);
    void SetHidden(bool f);
         
signals:
    void EmitValueChanged(int i);
public slots:
    //void SetFont(QFont);
    //void SetHidden(bool f);

private: 
    QLabel *l_name;
    HvSpinBox *sb_sn;
    
private slots:

protected:	
};
#include <QLineEdit>
#include "../config_str_all.h"

class HvSpinBoxDf : public QSpinBox
{
    Q_OBJECT
public:
    HvSpinBoxDf(bool,QWidget *parent = 0);
    virtual ~HvSpinBoxDf();
    
    void SetMode(int i);
    void SetDfAllModes(QString s);
    void SetValue(int);
    QString def_df_all_modes()
    {
        QString s;
        for (int i =0; i<COUNT_MODE; i++)
        {
        	s.append(ModeStr(i)+"=");
            s.append(QString("%1").arg(s_dftolerance[i]));
            if (i<COUNT_MODE-1)
                s.append("#");
        }
        return s;
    };
    void ExpandShrinkDf(bool f);//2.05
    //void SetFtDf1500(bool f);
  
signals:
    void EmitValueChanged(int i);
public slots:

private:
    //bool ft_df1500;
    int s_dftolerance[COUNT_MODE];
    //int s_def_tp_fast_modes;
    int s_mode;
    bool f_no_emit;
    
private slots:
    void Slot_valueChanged(int);

protected:
	void keyPressEvent(QKeyEvent*);

};
///////////////////////////////////////////////////////////
class HvSpinBox4per30s : public QSpinBox
{
    Q_OBJECT
public:
    HvSpinBox4per30s(bool,QWidget *parent = 0);
    virtual ~HvSpinBox4per30s();
    
    void SetMode(int i);
    void SetPiriodTimeAllModes(QString s);
    QString def_pt_all_modes()
    {
        QString s;
        for (int i =0; i<COUNT_MODE; i++)
        {
        	s.append(ModeStr(i)+"=");
            s.append(QString("%1").arg(s_pt_all_modes[i]));
            if (i<COUNT_MODE-1) s.append("#");
        }
        return s;
    };
    float get_period_time();
  
signals:
    void EmitValueChanged(float);
public slots:

private:
    int s_pt_all_modes[COUNT_MODE]; 
    //int s_def_tp_fast_modes;
    int s_mode;
    bool f_no_emit;
    
private slots:
    void Slot_valueChanged(int);

protected:
	void keyPressEvent(QKeyEvent*);

};
#endif
