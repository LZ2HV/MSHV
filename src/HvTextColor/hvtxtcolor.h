/* MSHV
 * By Hrisimir Hristov - LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#ifndef HVTXTCOLOR_H
#define HVTXTCOLOR_H

#include <QDialog>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QColor>
#include <QColorDialog>
#include <QGroupBox>
#define C_COLORS 7
#define C_SHEM 2
#define C_CB 29
#define C_BT 7

#include "../HvTxW/hvinle.h"
//#include "../config.h"
//#include <QtGui>

class HvTxtColor : public QDialog
{
    Q_OBJECT
public:
    HvTxtColor(bool,QWidget *parent = 0);
    virtual ~HvTxtColor();
    QString GetTextMark();
    void SetTextMark(QString);
    void SetTextColor(QString);
    QString GetTextColor()
    {
        QString s;
        for (int j =0; j<C_SHEM; j++)
        {
            for (int i =0; i<C_COLORS; i++)
            {
                s.append(QString("%1").arg(collors[j][i].red()));
                s.append("#");
                s.append(QString("%1").arg(collors[j][i].green()));
                s.append("#");
                s.append(QString("%1").arg(collors[j][i].blue()));

                if (i<C_COLORS-1)
                    s.append("#");
            }
            if (j<C_SHEM-1) s.append("|");
        }
        return s;
    };
    void SetFont(QFont f);

signals:
    void EmitTextMark(bool *f,QString);
    void EmitTextMarkColors(QColor*,int,bool,bool);

private slots:
    void TextMarkChanged(bool);
    void TxTextMarkChanged(bool);
    void OpenCDBox1();
    void OpenCDBox2();
    void OpenCDBox3();
    void OpenCDBoxT();
    void OpenCDBoxQ();
    void OpenCDBoxMC();
    void OpenCDBoxHC();
    void setDefColors();
    void ColorChanged(QColor);
    //void LeTextChanged(QString);

private:
    int dsty;
    QCheckBox *cb[C_CB];
    HvLeWithSpace *le_cals_msg;    
    QColor collors[C_SHEM][C_COLORS];
    QColor collors0[C_SHEM][C_COLORS];
    int b_ident;
    QPushButton *b_[C_BT];
    QColorDialog *CD_box;
    void RefreshAll();
    QString CorrectSyntax(QString txt ,bool cordot);

protected:
    void closeEvent(QCloseEvent*);

};
#endif
