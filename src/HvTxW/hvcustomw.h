/* MSHV HvCustomW
 * Copyright 2017 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#ifndef HVCUSTOMW_H
#define HVCUSTOMW_H
#include <QWidget>

#include <QPushButton>
class HvTxRxEqual : public QPushButton
{
    //Q_OBJECT //2.65 <- for tr() Q_OBJECT
public:
    HvTxRxEqual(QString,QString,bool trx,QWidget * parent = 0);
    //virtual ~HVequal();
//public slots:
//signals:
//private slots:
private:
	bool trx;
protected:
    void paintEvent(QPaintEvent *);
};

#include <QLineEdit>
#include "../HvButtons/hvbutton_left2.h"
class HvQueuedCallW : public QWidget
{
    Q_OBJECT
public:
    HvQueuedCallW(bool,QWidget *parent = 0);
    virtual ~HvQueuedCallW();
    void SetQueuedCall(QString,QString,QString,QString,QString,QString,QString);
    bool haveQueuedCall()
    {
    	return f_haveq;
    };
    QStringList GetQueuedCallData();
    bool isLastFromQueued()
    {
        return f_last_from_queued;
    };
    bool isUseQueueCont()
    {
        return f_use_queue_cont;
    };
    
    void ClrQueuedCall(bool);
    void SetFont(QFont);
    void SetUseQueueCont(bool);

signals:
	void EmidButClrQueuedCall();

private slots:
    void ClrQueuedCallB();

private:
	bool f_use_queue_cont;
	bool dsty;
    bool f_haveq;
    bool f_last_from_queued;
    QString s_my_rpt;
    QString s_his_rpt;
    QString s_exc;
    QString s_sn;
    QString s_loc;
    QString s_freq;
    HvButton_Left2 *bt_clr;
    QLineEdit *le_call;
    void SetLineColor(bool);
    void SetBackColor(bool);
    //void mousePressEvent(QMouseEvent *event);
};

class HvLeFreq : public QLineEdit
{
    Q_OBJECT
public:
    HvLeFreq(QWidget *parent = 0);
    virtual ~HvLeFreq();

signals:
    void EmitMousePress();
    void EmitEnterPress();
public slots:

private:

protected:
    void mousePressEvent(QMouseEvent*);
    void keyPressEvent(QKeyEvent*);
};

#include <QLabel>
class HvCatDispW : public QWidget
{
    Q_OBJECT
public:
    HvCatDispW(bool,QWidget *parent = 0);
    virtual ~HvCatDispW();
    void SetFreq(QString);
    void SetMode(QString);
    void SetFont(QFont f);
    void SetTxActive(int);

signals:
    void EmitSetDefFreqGlobal(int,QString);

private slots:
    void SetDefFreqGlobal();
    void ResizeLineEdit();
    void RefreshTimerEdit();
    void LeFreqMousePressed();

private:
	bool dsty;
    bool g_block_set_frq;
    QString s_prev_freq_global;
    bool f_edit_mod;
    int c_timer;
    QTimer *timer_edit;
    int w_mod_but;
    HvButton_Left2 *bt_def_freq;
    //QLabel *l_freq;
    QLabel *l_mode;
    HvLeFreq *le_freq;
    void SetEditAndBackColor(int);
    //void mousePressEvent(QMouseEvent *event);

};
#endif
