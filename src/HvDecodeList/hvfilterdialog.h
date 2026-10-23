/* MSHV FilterDialog
 * Copyright 2020 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#ifndef HVFILTERDIALOG_H
#define HVFILTERDIALOG_H

#include <QDialog>
#include <QCheckBox>
#include <QComboBox>
#include "../HvTxW/hvinle.h"
class HvFilterDialog : public QDialog
{
    Q_OBJECT
public:
    HvFilterDialog(bool,QWidget * parent);
    virtual ~HvFilterDialog();
    void SetFont(QFont);
    void SetSettings(QString); //,bool
    void SetSettings0(QString);
    void SetSettings1(QString);
    void SetSettings2(QString);
    void SetSettings3(QString);
    void SetSettings4(QString);   
    void SetSettings5(QString);  
    void SetSettings6(QString);   
    QString GetSettings();
    QString GetSettings0();
    QString GetSettings1();
    QString GetSettings2();
    QString GetSettings3();
    QString GetSettings4();
    QString GetSettings5();   
    QString GetSettings6();
    void SetCountries(QStringList); 
    QPushButton *pb_fltrOnOff;
    void SetHidFLBtOnOff(bool);

signals:
    void EmitSetFilter(QStringList,bool*,QStringList,QStringList,QStringList,QStringList,
    					QStringList,QStringList);

public slots:
    void SetTextMark(bool*,QString);

private slots:
    void SetDefaultFilter();
    void ApplyChFilter();
    void CBCQChanged(bool);
    void CBCQ73Changed(bool);
    void CBCCQChanged(bool);
    void CBContmChanged1(bool);
    void CBContmChanged2(bool);
    void CBGOnOffChanged(bool);
    void ClrCountrys();
    void CbCountrysChanged(QString);
    void CbRemCountrysChanged(QString); 
    void ClrHidCountrys();   
    void CbHidCountrysChanged(QString);
    void CbHidRemCountrysChanged(QString);
    void PbSetOnOff();
    void CbHidFLBtOnOff(bool);

private:
	bool dsty;
    QLabel *ltext;    
    QCheckBox *cb_hacontt[7];
    QCheckBox *cb_hide_b4qso;
    QCheckBox *cb_cq;
    QCheckBox *cb_ccq;
    QCheckBox *cb_cq73;
    HvLeWithSpace *le_ccq;
    QCheckBox *cb_contm0;
    HvLeWithSpace *le_contm0;
    QCheckBox *cb_contm1;
    HvLeWithSpace *le_contm1;
    QCheckBox *cb_contm2;
    HvLeWithSpace *le_contm2;
    QCheckBox *cb_contm3;
    HvLeWithSpace *le_contm3;
    QPushButton *b_clr;
    QCheckBox *cb_contm4;
    QLineEdit *le_contm4;
    QComboBox *CbCountrys;
    QComboBox *CbRemCountrys;
    QPushButton *b_hidclr;
    QCheckBox *cb_contm6;
    QLineEdit *le_contm6;
    QComboBox *CbHidCountrys;
    QComboBox *CbHidRemCountrys;    
    QCheckBox *cb_pfx5;
    HvLeWithSpace *le_pfx5;            
    QCheckBox *cb_gonoff;
    QCheckBox *cb_usefudpdectxt; 
    QCheckBox *cb_filtered_answer;
    QCheckBox *cb_usebtflonoff;       
    void RefrCountrys();
    void RefrHidCountrys();
    QStringList GetLineParms(HvLeWithSpace *le);
    void SetFilter();
    QString CorrectSyntax(QString,bool);
    void SetSettings_p(QString s, QCheckBox *cb, HvLeWithSpace *le,bool);
    QString GetSettings_p(QCheckBox *cb, HvLeWithSpace *le,bool);
    void RefreshPbSetOnOff(bool);
    void closeEvent(QCloseEvent*);

};
#endif