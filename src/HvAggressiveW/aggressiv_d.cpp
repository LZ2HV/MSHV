/* MSHV AggressiveDialog
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */

#include "aggressiv_d.h"
//#include <QtGui>

AggressiveDialog::AggressiveDialog(bool indsty,QWidget * parent )
        : QDialog(parent)
{
    setWindowTitle (tr("Aggressive Levels")+" JT65");
    setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);// maha help butona
    this->setMinimumWidth(250);

    QVBoxLayout *VL = new QVBoxLayout(this);
    VL->setContentsMargins (10, 10, 10, 10);
    VL->setSpacing(5);

    l_ftd = new QLabel(tr("Default")+"=5  "+tr("Level")+": 5");       
    l_deeps = new QLabel(tr("Default")+"=95  "+tr("Level")+": 95"); 

    QPixmap p0,p1,p2,p3,p4,p5,p6;

    if (indsty)
    {
        p0 = QPixmap(":pic/sld_h_left_b.png");
        p1 = QPixmap(":pic/sld_h_track_b.png");
        p2 = QPixmap(":pic/sld_h_right_b.png");
        p3 = QPixmap(":pic/tumb_h_play_b.png");
        p4 = QPixmap(":pic/tumb_h_over_play_b.png");
    }
    else
    {
        p0 = QPixmap(":pic/sld_h_left.png");
        p1 = QPixmap(":pic/sld_h_track.png");
        p2 = QPixmap(":pic/sld_h_right.png");
        p3 = QPixmap(":pic/tumb_h_play.png");
        p4 = QPixmap(":pic/tumb_h_over_play.png");
    }

    sl_aggres_ftd = new HvSlider_H(10,0,0,p0,p1,p2,p3,p4);
    connect(sl_aggres_ftd, SIGNAL(SendValue(int)), this, SLOT(SetLevelAggres(int)));
    sl_aggres_ftd->SetValue(5);

    if (indsty)
    {
        p5 = QPixmap(":pic/tumb_h_rec_b.png");
        p6 = QPixmap(":pic/tumb_h_over_rec_b.png");
    }
    else
    {
        p5 = QPixmap(":pic/tumb_h_rec.png");
        p6 = QPixmap(":pic/tumb_h_over_rec.png");
    }

    sl_aggres_deeps = new HvSlider_H(100,0,0,p0,p1,p2,p5,p6);

    connect(sl_aggres_deeps, SIGNAL(SendValue(int)), this, SLOT(SetLevelDeepS(int)));
    sl_aggres_deeps->SetValue(95);

    l_min = new QLabel("MIN");
    l_max = new QLabel("MAX");
    l_mind = new QLabel("MIN");
    l_maxd = new QLabel("MAX");

    if (indsty)
    {
    	l_ftd->setStyleSheet("QLabel {color :rgb(0, 220, 0);font-weight: bold;}");
    	l_deeps->setStyleSheet("QLabel {color :rgb(0, 220, 0);font-weight: bold;}");
        l_min->setStyleSheet("QLabel {color :rgb(170, 170, 250);}");
        l_max->setStyleSheet("QLabel {color :rgb(250, 100, 100);}");
        l_mind->setStyleSheet("QLabel {color :rgb(170, 170, 250);}");
        l_maxd->setStyleSheet("QLabel {color :rgb(250, 100, 100);}");
    }
    else
    {
    	l_ftd->setStyleSheet("QLabel {color :rgb(0, 150, 0);font-weight: bold;}");
    	l_deeps->setStyleSheet("QLabel {color :rgb(0, 150, 0);font-weight: bold;}");
        l_min->setStyleSheet("QLabel {color :rgb(0, 0, 200);}");
        l_max->setStyleSheet("QLabel {color :rgb(150, 0, 0);}");
        l_mind->setStyleSheet("QLabel {color :rgb(0, 0, 200);}");
        l_maxd->setStyleSheet("QLabel {color :rgb(150, 0, 0);}");
    }

    QHBoxLayout *HL_aggftd = new QHBoxLayout();
    HL_aggftd->setContentsMargins (0, 0, 0, 0);
    HL_aggftd->setSpacing(5);
    HL_aggftd->addWidget(l_min);
    HL_aggftd->addWidget(sl_aggres_ftd);
    HL_aggftd->addWidget(l_max);
    HL_aggftd->setAlignment(Qt::AlignCenter);

    QHBoxLayout *HL_deeps = new QHBoxLayout();
    HL_deeps->setContentsMargins (0, 0, 0, 0);
    HL_deeps->setSpacing(5);
    HL_deeps->addWidget(l_mind);
    HL_deeps->addWidget(sl_aggres_deeps);
    HL_deeps->addWidget(l_maxd);
    HL_deeps->setAlignment(Qt::AlignCenter);

    QVBoxLayout *VL_aggftd = new QVBoxLayout();
    VL_aggftd->setContentsMargins (0, 5, 0, 5);
    VL_aggftd->setSpacing(5);
    VL_aggftd->addWidget(l_ftd);
    VL_aggftd->setAlignment(l_ftd,Qt::AlignCenter);
    VL_aggftd->addLayout(HL_aggftd);
    //VL_aggftd->setAlignment(Qt::AlignHCenter);

    QVBoxLayout *VL_deeps = new QVBoxLayout();
    VL_deeps->setContentsMargins (0, 5, 0, 5);
    VL_deeps->setSpacing(5);
    VL_deeps->addWidget(l_deeps);
    VL_deeps->setAlignment(l_deeps,Qt::AlignCenter);
    VL_deeps->addLayout(HL_deeps);
    //VL_deeps->setAlignment(Qt::AlignHCenter);

    QGroupBox *GB_aggftd =   new QGroupBox(tr("Decoder Aggressive Level")+":");       
    QGroupBox *GB_aggdeeps = new QGroupBox(tr("Deep Search Aggressive Level")+":");
    GB_aggftd->setLayout(VL_aggftd);
    GB_aggdeeps->setLayout(VL_deeps);
    VL->addWidget(GB_aggftd);
    VL->addWidget(GB_aggdeeps);

    //QLabel *l_devo;
    //l_devo = new QLabel();

    //connect(OutBuf, SIGNAL(currentIndexChanged(QString)), this, SLOT(OutDeviceChanged(QString)));*/
    //VL->addWidget(sl_aggres_ftd);
    //VL->addWidget(sl_aggres_deeps);
    this->setLayout(VL);
}
AggressiveDialog::~AggressiveDialog()
{}
void AggressiveDialog::SetFont(QFont f)
{
    l_min->setFont(f);
    l_max->setFont(f);
    l_mind->setFont(f);
    l_maxd->setFont(f);
    l_ftd->setFont(f);
    l_deeps->setFont(f);
}
void AggressiveDialog::SetAggresLevels(QString str)
{
    QStringList lstr = str.split("#");
    if (lstr.count()==2)
    {
        sl_aggres_ftd->SetValue(lstr.at(0).toInt());
        sl_aggres_deeps->SetValue(lstr.at(1).toInt());
    }
}
QString AggressiveDialog::GetAggresLevels()
{
    QString str = QString("%1").arg(sl_aggres_ftd->get_value())+"#"+QString("%1").arg(sl_aggres_deeps->get_value());
    return str;
}
void AggressiveDialog::SetLevelAggres(int val)
{
    l_ftd->setText(tr("Default")+"=5  "+tr("Level")+": "+QString("%1").arg(val));
    emit EmitLevelAggres(val);
}
void AggressiveDialog::SetLevelDeepS(int val)
{
    l_deeps->setText(tr("Default")+"=95  "+tr("Level")+": "+QString("%1").arg(val));
    emit EmitLevelDeepS(val);
}

