/* MSHV Switcher Buttons
 * Copyright 2024 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#ifndef HVMODBTSW_H
#define HVMODBTSW_H

#include <QDialog>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QGroupBox>

class CbSwID : public QCheckBox
{
    Q_OBJECT
public:
    CbSwID(int id,QString s,QWidget * parent = 0);
    virtual ~CbSwID();

signals:
    void cbclicked(int,bool);

private slots:
    void sclicked(bool);

private:
    int sid;

};
#include <QPushButton>
#include <QFont>
class HvButton_ID : public QPushButton
{
    Q_OBJECT
public:
    HvButton_ID(int id, QString,QWidget * parent = 0);
    virtual ~HvButton_ID();
    int sid;

private:

signals:
    void btclicked(int);

public slots:
    void sclicked();

};
class HvDialogBtSw : public QDialog
{
    Q_OBJECT
public:
    HvDialogBtSw(int c_bt,QString *str_bt,QString tit,QString gbt,QString buse,QWidget * parent);
    virtual ~HvDialogBtSw();
    void SetDsettings(int,bool,int);

signals:
    void clicked(int,bool);

public slots:

private:   
    QList<CbSwID*> ListCbID;//QVBoxLayout *LV;
    int sid;
    CbSwID *cb220;
};
class HvWBtSw : public QWidget
{
    Q_OBJECT
public:
    HvWBtSw(QWidget *dp,int c_bt,int *bid,QString *str_bt,QString tit,QString gbt,QString buse,QString dss,bool,QWidget * parent = 0);
    virtual ~HvWBtSw();
    void SetSettings(QString);
    QString GetSettings();
    void SetFont(QFont);
    void SetActiveBt(int);
    //void SetDefault();

signals:
    void clicked(int);

public slots:
    void SetHidden(int,bool);
    void dexec();

private:
	bool dsty;
	int btprv;
	int sc_bt;
	//QString sstr_bt[MAX_BT+4];
	//int sbtid[MAX_BT+4];
    QHBoxLayout *H_bt_;
    HvDialogBtSw *D_bt_sw;

};
#endif