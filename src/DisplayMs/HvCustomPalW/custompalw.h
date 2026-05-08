#ifndef CUSTOMPLAW_H
#define CUSTOMPLAW_H

#include <QWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QImage>
#include <QColorDialog>
#include <QCheckBox>

class ImgW : public QWidget
{
    //Q_OBJECT
public:
    ImgW( QWidget * parent = 0 );
    virtual ~ImgW();

    QImage img_pal;
    void SetPosLin(int l[7]);
//private slots:

private:
    int posYtxt[10];
    QString dBtext[10];
    int posYlin[7]; //2.46 

protected:
    void paintEvent(QPaintEvent *);

};

class CustomPalW : public QWidget
{
    Q_OBJECT
public:
    CustomPalW(int,int,QWidget * parent = 0);
    ~CustomPalW();

    void SetPaletteToDisplay();
    void SetPalSettings(QString);
    QString GetPalSettings()
    {
        QString s;
        for (int i =0; i<7; i++)
        {
            s.append(QString("%1").arg(z_all[i]));
            s.append("#");
        }
        for (int i =0; i<11; i++)
        {
            s.append(QString("%1").arg(collors[i].red()));
            s.append("#");
            s.append(QString("%1").arg(collors[i].green()));
            s.append("#");
            s.append(QString("%1").arg(collors[i].blue()));

            if (i==9)
            {
                s.append("#");
                s.append(QString("%1").arg(collors[i].alpha()));
            }

            if (i<11-1)
                s.append("#");

        }
        return s;
    };

signals:
    void EmitCustomPalette(QPixmap,QColor,QColor);

public slots:
    //void SetPaletteToDisplay();

private slots:
    void ZChanged(int);
    void OpenCDBox1();
    void OpenCDBox2();
    void OpenCDBox3();
    void OpenCDBox4();
    void OpenCDBox5();
    void OpenCDBox6();
    void OpenCDBox7();
    void OpenCDBox8();
    void OpenCDBox9();
    void OpenCDBoxW();
    void OpenCDBoxWL();
    void CbNoWave();
    void ColorChanged(QColor);
    void setDefColorsAndPos();
    //void SetPaletteToDisplay();

private:
    bool g_block_z;
    //int f_inc_z_range;
    int b_ident;
    QColor collors[11];
    int z_all[7];
    //QColor wave_collor;
    /*int z1;
    int z2;
    int z3;
    int z4;
    int z5;
    int z6;
    int z7;*/
    QPushButton *b_c[9];
    QPushButton *b_c_wave;
    QPushButton *b_c_wavel;
    QCheckBox *cb_no_wave;
    
    QSpinBox *SB_z[7];

    QColorDialog *CD_box;

    //QImage img_pal;
    //QPixmap s_pic;
    ImgW *TImgW;
    //QPushButton *b_set;
    QPushButton *b_set_default;
    void OpenCDBox_p();
    void setPalette();
    void setRegion(int,int,int,int,int,int,int,int);
    void RefreshAll();
    //void MoveDialog();



    //QPushButton *b_c7;


protected:


};



#endif




