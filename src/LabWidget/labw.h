/* MSHV
 *
 * By Hrisimir Hristov - LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */

#ifndef LABW_H
#define LABW_H

#include <QWidget>
#include <QLabel>
//#include <QPainter>
#include <QHBoxLayout>

class ChangedLab : public QLabel
{
    //Q_OBJECT
public:
    ChangedLab(QString c_1,QString s_1,QString c_2,QString s_2,QString c_3,QString s_3,int w,int h,QWidget *parent = 0);
    virtual ~ChangedLab();
    
    void SetOnOff(int);
    void SetWidthFromFont(int);

private:
    //bool back_gr_flag;
    QString c1;
    QString s1;
    QString c2;
    QString s2;
    QString c3;
    QString s3;
    //QLabel *label;
    //QPixmap pix_1;
    //QPixmap pix_2;
    //QPixmap drow_pix;
    

//protected:
    //void paintEvent(QPaintEvent *);

};

class LabW : public QWidget
{
    //Q_OBJECT
public:
    LabW(bool,QWidget *parent = 0);
    virtual ~LabW();

    void SetRxMon(int);
    void SetFont(QFont);

    
//public slots:
    void SetDecode(int);
  
private:
    ChangedLab *TxRxLab;
    ChangedLab *DecodeLab;

protected:
   

};
#endif
