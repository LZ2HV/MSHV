/* MSHV RadioAndNetW for Log program
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */

#ifndef HVINLE_H
#define HVINLE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QKeyEvent>


class HvLeWithSpace : public QLineEdit
{
    Q_OBJECT
public:
    HvLeWithSpace(QWidget *parent = 0);
    virtual ~HvLeWithSpace();

signals:
    //void EmitEnter();

public slots:
    void TextChanged(QString);

private:


protected:
    //void mousePressEvent(QMouseEvent*);
    void keyPressEvent(QKeyEvent*);

};


class HvLeNoSpace : public QLineEdit
{
    Q_OBJECT
public:
    HvLeNoSpace(QWidget *parent = 0);
    virtual ~HvLeNoSpace();

signals:
    void EmitEnter();
    //void STextChanged(QString);

public slots:
    void TextChanged(QString);

private:


protected:
    void mousePressEvent(QMouseEvent*);
    void keyPressEvent(QKeyEvent*);

};

#include <QValidator>
class HvInLe : public QWidget
{
    Q_OBJECT
public:
    HvInLe(QString type, QString name, bool,QWidget *parent = 0);
    virtual ~HvInLe();

    bool HasFocus()
    {
        return  le_in->hasFocus();
    };
    void SetFocus();
    //void SetErrotText(QString);
    void SetMask(QString);
    void SetText(QString);
    void setReadOnly(bool);
    void setError(bool);
    void SetTitle(QString);

    
    bool getError()
    {
        return f_error;
    };
    QString getText()
    {
        return le_in->text();
    };
    int getCount()
    {
        return le_in->text().count();
    };
    void setMaxLength(int l);
    void setFixedWidthLine(int w);
    void setErrorColorLe(bool);
    void SetValidatorHv(QValidator*);
    void SetFont(QFont);

signals:
    void EmitSndCheck(QString);
    void EmitTextChanged(QString);
    void EmitEntered();

public slots:
    void SndCheck_s(QString);

private:
	bool dsty;
    HvLeNoSpace *le_in;
    QString s_type;
    QString s_name;
    QLabel *l_n;
    //QLabel *l_error;
    QHBoxLayout *H_in;
    bool f_error;

};

#include "../HvButtons/hvbutton_left2.h"
#include <QTimer>

class HvRptLe : public QWidget
{ 
    Q_OBJECT
public:
    HvRptLe(QString type, QString name, bool,QWidget *parent = 0);
    virtual ~HvRptLe();

    bool HasFocus()
    {
        return  le_in->hasFocus();
    };
    void SetFocus();
    //void SetErrotText(QString);
    void SetMask(QString);
    void SetText(QString,bool);
    void setReadOnly(bool);
    void setError(bool);
    //void SetTitle(QString);
    void SetRptRsqSettings(QString title,int mode,bool rpt_rsq,bool sh_rpt,int);
    QString format_rpt(QString s);
    QString format_rpt_tx(QString s);

    
    bool getError()
    {
        return f_error;
    };
    QString getText()
    {
        return le_in->text();
    };
    int getCount()
    {
        return le_in->text().count();
    };
    void setMaxLength(int l);
    void setFixedWidthLine(int w);
    void setErrorColorLe(bool);
    void SetFont(QFont f);
    bool isRealRpt()
    {
        return is_real_rpt;
    };

signals:
    void EmitSndCheck(QString);//2.66
    void EmitEntered();
    //void EmitIncreaseRpt(int);

public slots:
    void SndCheck_s(QString);
    
private slots:  
    void up_rpt();  
    void down_rpt();
    void timer_inc_rpt();
    void stop_inc();

private:
	bool dsty;
	bool allq65;
	bool is_real_rpt;
    HvLeNoSpace *le_in;
    QString s_type;
    QString s_name;
    QLabel *l_n;
    //QLabel *l_error;
    QHBoxLayout *H_in;
    bool f_error;
    HvButton_Left2 *bt_up;
    HvButton_Left2 *bt_down;
    QWidget *mid_line;
    void IncreaseRpt(int);
    int s_rep_pos;
    int s_mode;      
    bool s_sh_rpt;
    int s_cont_type;
    void SetTitle(QString);  
    void SetSpinCtrl(bool f);    
    QTimer *timer_inc;
    int inc_up_down;
    int wait_tacts;
    bool s_rpt_rsq;    

};

class HvInLeFreq : public QWidget
{
    //Q_OBJECT
public:
    HvInLeFreq(QString name, QWidget *parent = 0);
    virtual ~HvInLeFreq();

    QString Text();
    void SetText(QString);

//signals:   

//public slots:
    
private:
    QLineEdit *le_frq;
};


class HvLabQrg : public QLabel
{
    Q_OBJECT
public:
    HvLabQrg(bool,QWidget * parent = 0);
    virtual ~HvLabQrg();
    int active_id;
    void SetActiveId(int);
    //void SetToolTipText(QString);

public slots:

signals:
	void EmitRefresh();

private:
	bool dsty;
	void RefreshAll();

protected:
    void mousePressEvent( QMouseEvent * ev );
    void mouseReleaseEvent(QMouseEvent * event);
};
class HvQrg : public QWidget
{
    Q_OBJECT
public:
    HvQrg(bool,QWidget *parent = 0);
    virtual ~HvQrg();
    QString text();
	void setText(QString);
	void SetQrgFromRig(QString);
	void SetQrgInfoFromCat(QString);
	void SetQrgActive(int);
	void SetReadOnly(bool);
	void SetFont(QFont f);
	int GetQrgActive()//2.51
	{
		return l_qrg->active_id;
	};

signals:  
	void EmitEnter();
	void EmitTextChanged(QString);
	void EmitQgrParms(QString,bool); 
	void EmitActiveId(int);//2.60

public slots:
	void SetEnter();	
	
private slots:
   
private:
	HvLabQrg *l_qrg;
    HvLeNoSpace *le_qrg;
    QString rx_sqrg;
    QString tx_sqrg;
    bool f_one_no_refr_from_rig;
    
};


#endif
