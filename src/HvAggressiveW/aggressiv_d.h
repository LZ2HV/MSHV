/* MSHV AggressiveDialog
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#ifndef AGGRESSIVE_DIALOG_H
#define AGGRESSIVE_DIALOG_H

//#include "../config.h"

#include <QDialog>
#include <QLabel> 
#include <QVBoxLayout>
#include <QGroupBox>
#include "../HvSlider_H/hvslider_h.h"

class AggressiveDialog : public QDialog
{
    Q_OBJECT
public:
    AggressiveDialog(bool,QWidget * parent = 0);
    virtual ~AggressiveDialog();

    void SetAggresLevels(QString);
    QString GetAggresLevels();
    void SetFont(QFont);

signals:
    void EmitLevelAggres(int val);
    void EmitLevelDeepS(int val);
    
public slots:

private slots:
    void SetLevelAggres(int val);
    void SetLevelDeepS(int val);
   
private:
    QLabel *l_ftd;
    QLabel *l_deeps;
    HvSlider_H *sl_aggres_ftd;
    HvSlider_H *sl_aggres_deeps;
    QLabel *l_min;
    QLabel *l_max;
    QLabel *l_mind;
    QLabel *l_maxd;
    
};
#endif
