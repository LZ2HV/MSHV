/* MSHV CustomPalW
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */

#include "custompalw.h"

//#include <QDesktopWidget>
//#include <QApplication>
#include <QVBoxLayout>
#include <QPainter>
#include <math.h>  /* fabs */
//#include <QtGui>
 
ImgW::ImgW( QWidget * parent )
        : QWidget(parent)
{
    img_pal = QImage(20, 501, QImage::Format_RGB32);
    img_pal.fill(Qt::white);
    setFixedSize(100,501);
    //setStyleSheet("background-color:black;");
    //setStyleSheet("QWidget{background-color:black;}");

    /*
    posYtxt[0] = 220;//180;//220;//+20
    dBtext[0] = " +20 dB";
    posYtxt[1] = 262;//220;//+10
    dBtext[1] = " +10 dB";
    posYtxt[2] = 306;//265;           //0
    dBtext[2] = "    0 dB";
    posYtxt[3] = 350;//310;//282;//-10
    dBtext[3] = " -10 dB";
    posYtxt[4] = 400;//361;//300;//-20
    dBtext[4] = " -20 dB";
    posYtxt[5] = 450;//390;//340;//-30
    dBtext[5] = "";//" -30 dB";
    */

    posYtxt[0] = 150+4;
    dBtext[0] = " 150 pix";
    posYtxt[1] = 200+4;//180;//220;//+20
    dBtext[1] = " 200 pix";
    posYtxt[2] = 250+4;//220;//+10
    dBtext[2] = " 250 pix";
    posYtxt[3] = 300+4;//265;           //0
    dBtext[3] = " 300 pix";
    posYtxt[4] = 350+4;//310;//282;//-10
    dBtext[4] = " 350 pix";
    posYtxt[5] = 400+4;//361;//300;//-20
    dBtext[5] = " 400 pix";
    // posYtxt[5] = 450+4;//390;//340;//-30
    // dBtext[5] = " 450 pix";//" -30 dB";

	//2.46 
    posYlin[0] = 63;
    posYlin[1] = 126;
    posYlin[2] = 189;
    posYlin[3] = 252;
    posYlin[4] = 315;
    posYlin[5] = 378;
    posYlin[6] = 441;

    update();
}
ImgW::~ImgW()
{}
void ImgW::SetPosLin(int l[7])//2.46
{
    posYlin[0] = l[0];
    posYlin[1] = l[1];
    posYlin[2] = l[2];
    posYlin[3] = l[3];
    posYlin[4] = l[4];
    posYlin[5] = l[5];
    posYlin[6] = l[6];
}
void ImgW::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.save();
    //painter.fillRect(0, 0, width(), height(), QColor(0,0,0,255));
    painter.fillRect(0,0,width(),height(),QColor(255,255,255));
    painter.drawImage(7,0,img_pal);

	painter.setPen(QPen(QColor(0,0,0),3));//2.54

    for (int i=0; i < 6; i++)
        painter.drawText(20+20, posYtxt[i],dBtext[i]);
        
    //painter.setPen(QPen(QColor(0,0,0),3));    
    for (int i=0; i < 7; i++)
    {
        /*painter.drawLine(28, posYlin[i], 31, posYlin[i]);   
        painter.drawLine(2, posYlin[i], 5, posYlin[i]);*/
        painter.drawLine(29, posYlin[i], 30, posYlin[i]);   
        painter.drawLine(3, posYlin[i], 4, posYlin[i]);          	
   	}    

    //stop if make print screan for new palette //
    painter.setPen(QPen(QColor(0,0,255),0));
    painter.drawRect(0, 0, 100-1, 501-1);
    painter.setPen(QPen(QColor(255,255,255),4)); 
    //painter.drawRect(3, 3, 100-6, 501-6);
    painter.drawLine(8, 3, 26, 3);
    painter.drawLine(8, 501-3, 26, 501-3);
    //end stop if make print screan for new palette //

    painter.restore();
}

CustomPalW::CustomPalW(int x,int y,QWidget * parent)
        : QWidget(parent)
{

    //App_Name = "Empty Project v.001";
    setWindowTitle(tr("Custom Palette Editor"));
    setWindowFlags(windowFlags() &~ Qt::WindowMaximizeButtonHint &~ Qt::WindowContextHelpButtonHint);
    setWindowIcon(QPixmap(":pic/ms_ico.png"));
    //setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);
    //setWindowFlags(windowFlags() ^ Qt::WindowMaximizeButtonHint);
    //QPixmap image;
    //setFixedSize(200,650);
    //this->setMinimumWidth(270);

    TImgW = new ImgW();
    //TImgW->setStyleSheet("background-color:black;");
    //TImgW->setContentsMargins(0,0,30,0);

    g_block_z = false;
    //f_inc_z_range = 0;
    z_all[0] = 63;
    z_all[1] = 126;
    z_all[2] = 189;
    z_all[3] = 252;
    z_all[4] = 315;
    z_all[5] = 378;
    z_all[6] = 441;
    
    for (int i = 0; i < 7; ++i) 
    {
    	SB_z[i] = new QSpinBox();
    	SB_z[i]->setSingleStep(1);
    	SB_z[i]->setSuffix(" pix");
    	SB_z[i]->setRange(0,500);
    	SB_z[i]->setValue(z_all[i]);
   	}

    for (int i = 0; i < 7; ++i) connect(SB_z[i], SIGNAL(valueChanged(int)), this, SLOT(ZChanged(int))); 

    b_ident = -1;
    int but_height = 20;
    /*b_c[0]= new QPushButton(" "+tr("COLOR")+" 1 ");
    b_c[0]->setFixedHeight(but_height);
    b_c[1]= new QPushButton(" "+tr("COLOR")+" 2 ");
    b_c[1]->setFixedHeight(but_height);
    b_c[2]= new QPushButton(" "+tr("COLOR")+" 3 ");
    b_c[2]->setFixedHeight(but_height);
    b_c[3]= new QPushButton(" "+tr("COLOR")+" 4 ");
    b_c[3]->setFixedHeight(but_height);
    b_c[4]= new QPushButton(" "+tr("COLOR")+" 5 ");
    b_c[4]->setFixedHeight(but_height);
    b_c[5]= new QPushButton(" "+tr("COLOR")+" 6 ");
    b_c[5]->setFixedHeight(but_height);
    b_c[6]= new QPushButton(" "+tr("COLOR")+" 7 ");
    b_c[6]->setFixedHeight(but_height);
    b_c[7]= new QPushButton(" "+tr("COLOR")+" 8 ");
    b_c[7]->setFixedHeight(but_height);
    b_c[8]= new QPushButton(" "+tr("COLOR")+" 9 ");
    b_c[8]->setFixedHeight(but_height);*/    
    for (int i = 0; i < 9; ++i) 
    {
    	b_c[i]= new QPushButton(" "+tr("COLOR")+" "+QString("%1").arg(i+1)+" ");
    	b_c[i]->setFixedHeight(but_height);    	
   	}     
    b_c_wave= new QPushButton(tr("WAVE COLOR"));
    b_c_wave->setFixedHeight(but_height);
    b_c_wavel= new QPushButton(tr("WAVE LINE"));
    b_c_wavel->setFixedHeight(but_height);
    cb_no_wave = new QCheckBox(tr("OFF"));
    cb_no_wave->setFixedWidth(50);
    connect(b_c[0], SIGNAL(clicked(bool)), this, SLOT(OpenCDBox1()));
    connect(b_c[1], SIGNAL(clicked(bool)), this, SLOT(OpenCDBox2()));
    connect(b_c[2], SIGNAL(clicked(bool)), this, SLOT(OpenCDBox3()));
    connect(b_c[3], SIGNAL(clicked(bool)), this, SLOT(OpenCDBox4()));
    connect(b_c[4], SIGNAL(clicked(bool)), this, SLOT(OpenCDBox5()));
    connect(b_c[5], SIGNAL(clicked(bool)), this, SLOT(OpenCDBox6()));
    connect(b_c[6], SIGNAL(clicked(bool)), this, SLOT(OpenCDBox7()));
    connect(b_c[7], SIGNAL(clicked(bool)), this, SLOT(OpenCDBox8()));
    connect(b_c[8], SIGNAL(clicked(bool)), this, SLOT(OpenCDBox9())); 
    connect(b_c_wave, SIGNAL(clicked(bool)), this, SLOT(OpenCDBoxW()));
    connect(b_c_wavel, SIGNAL(clicked(bool)), this, SLOT(OpenCDBoxWL()));
    connect(cb_no_wave, SIGNAL(toggled(bool)), this, SLOT(CbNoWave()));

    QHBoxLayout *H_wave = new QHBoxLayout();
    H_wave->addWidget(cb_no_wave);
    H_wave->addWidget(b_c_wave);
    //H_wave->addWidget(cb_no_wave);
    H_wave->addWidget(b_c_wavel);

    CD_box = new QColorDialog(this);
    CD_box->setOption(QColorDialog::NoButtons);
    connect(CD_box, SIGNAL(currentColorChanged(QColor)), this, SLOT(ColorChanged(QColor)));
    //void colorSelected ( const QColor & color )
    //void currentColorChanged ( const QColor & color )

    //b_set_default = new QPushButton("SET DEFAULT PALETTE");
    b_set_default = new QPushButton(tr("SET DEFAULT"));
    b_set_default->setFixedHeight(but_height);
    connect(b_set_default, SIGNAL(clicked(bool)), this, SLOT(setDefColorsAndPos()));

    QVBoxLayout *v_ls = new QVBoxLayout();
    v_ls->setContentsMargins(25,0,0,0);
    v_ls->addWidget(b_set_default);
    /*v_ls->addWidget(b_c[0]);
    v_ls->addWidget(SB_z[0]);
    v_ls->addWidget(b_c[1]);
    v_ls->addWidget(SB_z[1]);
    v_ls->addWidget(b_c[2]);
    v_ls->addWidget(SB_z[2]);
    v_ls->addWidget(b_c[3]);
    v_ls->addWidget(SB_z[3]);
    v_ls->addWidget(b_c[4]);
    v_ls->addWidget(SB_z[4]);
    v_ls->addWidget(b_c[5]);
    v_ls->addWidget(SB_z[5]);
    v_ls->addWidget(b_c[6]);
    v_ls->addWidget(SB_z[6]);*/
    for (int i = 0; i < 9; ++i)
    {
    	if (i<7)
    	{
    		v_ls->addWidget(b_c[i]);
    		v_ls->addWidget(SB_z[i]);    		
   		}
		else v_ls->addWidget(b_c[i]);
   	}
    /*v_ls->addWidget(b_c[7]);
    v_ls->addWidget(b_c[8]);*/

    QHBoxLayout *H_l = new QHBoxLayout();
    //H_l->setContentsMargins(30,0,5,0);
    //H_l->setSpacing(30);
    H_l->addWidget(TImgW);
    H_l->addLayout(v_ls);

    //TImgW->setStyleSheet("QWidget{background-color:black;}");


    //b_set = new QPushButton(" SET PALETTE TO DISPLAYS ");
    //connect(b_set, SIGNAL(clicked(bool)), this, SLOT(SetPaletteToDisplay()));

    QVBoxLayout *V_l = new QVBoxLayout(this);
    V_l->setContentsMargins(10,5,10,5);
    // V_l->addWidget(b_set_default);
    V_l->addLayout(H_l);
    V_l->addLayout(H_wave);
    //V_l->addWidget(b_c_wave);
    //V_l->addWidget(b_set);

    this->setLayout(V_l);

    move(x,y);

    setDefColorsAndPos();
    /*for (int i=0; i < 11; i++)
    {
        b_ident = i;
        ColorChanged(collors[i]);
    }*/

    //setPalette();
}
CustomPalW::~CustomPalW()
{}
void CustomPalW::OpenCDBox_p()
{
    CD_box->move(pos().x()+width(),pos().y()+80);
    CD_box->open();
    CD_box->setCurrentColor(collors[b_ident].rgb());	
}
void CustomPalW::OpenCDBox1()
{
    b_ident = 0;
    OpenCDBox_p();
}
void CustomPalW::OpenCDBox2()
{
    b_ident = 1;
    OpenCDBox_p();
}
void CustomPalW::OpenCDBox3()
{
    b_ident = 2;
    OpenCDBox_p();
}
void CustomPalW::OpenCDBox4()
{
    b_ident = 3;
    OpenCDBox_p();
}
void CustomPalW::OpenCDBox5()
{
    b_ident = 4;
    OpenCDBox_p();
}
void CustomPalW::OpenCDBox6()
{
    b_ident = 5;
    OpenCDBox_p();
}
void CustomPalW::OpenCDBox7()
{
    b_ident = 6;
    OpenCDBox_p();
}
void CustomPalW::OpenCDBox8()
{
    b_ident = 7;
    OpenCDBox_p();
}
void CustomPalW::OpenCDBox9()
{
    b_ident = 8;
    OpenCDBox_p();
}
void CustomPalW::OpenCDBoxW()
{
    b_ident = 9;
    OpenCDBox_p();
}
void CustomPalW::OpenCDBoxWL()
{
    b_ident = 10;
    OpenCDBox_p();
}
void CustomPalW::CbNoWave()
{
    if (cb_no_wave->isChecked()) collors[9].setAlpha(0);
    else collors[9].setAlpha(255);
    RefreshAll();
}
void CustomPalW::ColorChanged(QColor c)
{
    //"QLabel{background-color :rgb(145, 220, 255);}"
    //QString s ="QPushButton{background-color :rgb("+QString("%1").arg(c.red())+","+QString("%1").arg(c.green())+","+QString("%1").arg(c.blue())+");}";
    QString s ="QPushButton{border-color :rgb("+QString("%1").arg(c.red())+","+QString("%1").arg(c.green())+","+QString("%1").arg(c.blue())+");border-width: 4px;border-style: outset;border-radius: 5px;}";
    
    for (int i = 0; i < 9; ++i)
    {
    	if (b_ident==i)
    	{
    		b_c[i]->setStyleSheet(s);
    		break;
   		}
   	}
    switch (b_ident)
    {
    /*case 0:
        b_c1->setStyleSheet(s);
        break;
    case 1:
        b_c2->setStyleSheet(s);
        break;
    case 2:
        b_c3->setStyleSheet(s);
        break;
    case 3:
        b_c4->setStyleSheet(s);
        break;
    case 4:
        b_c5->setStyleSheet(s);
        break;
    case 5:
        b_c6->setStyleSheet(s);
        break;
    case 6:
        b_c7->setStyleSheet(s);
        break;
    case 7:
        b_c8->setStyleSheet(s);
        break;
    case 8:
        b_c9->setStyleSheet(s);
        break;*/
    case 9:
        b_c_wave->setStyleSheet(s);
        break;
    case 10:
        b_c_wavel->setStyleSheet(s);
        break;
    }

    collors[b_ident]=c;
    if (b_ident==9)
    {
        if (cb_no_wave->isChecked())
            collors[b_ident].setAlpha(0);
        else
            collors[b_ident].setAlpha(255);
    }

    if (!g_block_z)
    {
        setPalette();
        SetPaletteToDisplay();
    }
    //qDebug()<<c.red()<<c.green()<<c.blue();
}
void CustomPalW::ZChanged(int)
{
    if (!g_block_z)
    {
        //qDebug()<<"ppppppp";
        for (int i = 0; i < 7; ++i) z_all[i] = SB_z[i]->value(); 

        //z_all[3] = SB_z3->value(); // for test dB

        setPalette();
        SetPaletteToDisplay();
    }
}
void CustomPalW::RefreshAll()
{
    g_block_z = true;

    for (int i = 0; i < 7; ++i) SB_z[i]->setValue(z_all[i]);

    for (int i=0; i < 11; i++)
    {
        b_ident = i;
        ColorChanged(collors[i]);
    }

    /*if (collors[9].alpha()==0)
        cb_no_wave->setChecked(true);
    else
        cb_no_wave->setChecked(false);*/

    g_block_z = false;
    ZChanged(0);
}
void CustomPalW::SetPalSettings(QString s)
{
    QStringList ls=s.split("#");
    //qDebug()<<ls.count()<<ls;
    if (ls.count()==41)//???
    {
        for (int i =0; i<7; i++)
            z_all[i] = ls[i].toInt();
        int k = 7;
        for (int i =0; i<11; i++)
        {
            collors[i].setRed(ls[k].toInt());
            k++;
            collors[i].setGreen(ls[k].toInt());
            k++;
            collors[i].setBlue(ls[k].toInt());
            k++;
            if (i==9)
            {
                collors[i].setAlpha(ls[k].toInt());
                k++;
            }
        }
    }
    RefreshAll();
}
void CustomPalW::setDefColorsAndPos()
{
    z_all[0] = 200;
    z_all[1] = 240;
    z_all[2] = 255;
    z_all[3] = 283; //0
    z_all[4] = 300;
    z_all[5] = 370;
    z_all[6] = 415;

    collors[0] = QColor(240,240,240); // 0
    collors[1] = QColor(255,0,0);
    collors[2] = QColor(255,240,0);
    collors[3] = QColor(235,235,235);
    collors[4] = QColor(0,55,206);
    collors[5] = QColor(0,53,138);
    collors[6] = QColor(0,20,39);
    collors[7] = QColor(20,20,20);
    collors[8] = QColor(0,0,0);       // 500

    collors[9] = QColor(0,40,134,255);
    cb_no_wave->setChecked(false);

    collors[10] = QColor(255,255,255);

    RefreshAll();
}
void CustomPalW::setRegion(int z1,int z2,int r0,int g0,int b0,int r1,int g1,int b1)
{
    //double inc = 31.0/(double)(z2-z1);
    //qDebug()<<inc;
    //double i = 31.7;
    bool f_r = false;
    bool f_g = false;
    bool f_b = false;

    if (r0-r1<0)
        f_r = true;
    if (g0-g1<0)
        f_g = true;
    if (b0-b1<0)
        f_b = true;

    double inc_r = (double)fabs(r0-r1)/(double)(z2-z1);// qDebug()<<inc_r;
    double inc_g = (double)fabs(g0-g1)/(double)(z2-z1);// qDebug()<<inc_g;
    double inc_b = (double)fabs(b0-b1)/(double)(z2-z1);// qDebug()<<inc_b;
    double r = r0;
    double g = g0;
    double b = b0;

    for (int k=z1; k < z2; k++)
    {
        for (int j=0; j < 20; j++)
            TImgW->img_pal.setPixel(j,k,QColor((int)r,(int)g,(int)b).rgb()); //378

        if (f_r)
            r = r + inc_r;
        else
            r = r - inc_r;

        if (f_g)
            g = g + inc_g;
        else
            g = g - inc_g;

        if (f_b)
            b = b + inc_b;
        else
            b = b - inc_b;
        //i = i - inc;
        //qDebug()<<(int)r<<(int)g<<(int)b;
    }
}
void CustomPalW::setPalette()
{
    /* for test dB
    for (int k=0; k < 500; k++)
    	{
         for (int j=0; j < 20; j++)
         	TImgW->img_pal.setPixel(j,k,QColor(255,255,255).rgb()); 
    	}
    setRegion(z_all[3], z_all[3]+20,collors[3].red(),collors[3].green(),collors[3].blue(),collors[4].red(),collors[4].green(),collors[4].blue());	
    */

    //qDebug()<<"tttttt"<<collors[1].red()<<collors[1].green()<<collors[1].blue();
    //qDebug()<<"uuuuuu"<<collors[2].red()<<collors[2].green()<<collors[2].blue();
    setRegion(0,z_all[0],collors[0].red(),collors[0].green(),collors[0].blue(),collors[1].red(),collors[1].green(),collors[1].blue());
    setRegion(z_all[0], z_all[1],collors[1].red(),collors[1].green(),collors[1].blue(),collors[2].red(),collors[2].green(),collors[2].blue());
    setRegion(z_all[1], z_all[2],collors[2].red(),collors[2].green(),collors[2].blue(),collors[3].red(),collors[3].green(),collors[3].blue());
    setRegion(z_all[2], z_all[3],collors[3].red(),collors[3].green(),collors[3].blue(),collors[4].red(),collors[4].green(),collors[4].blue());
    setRegion(z_all[3], z_all[4],collors[4].red(),collors[4].green(),collors[4].blue(),collors[5].red(),collors[5].green(),collors[5].blue());
    setRegion(z_all[4], z_all[5],collors[5].red(),collors[5].green(),collors[5].blue(),collors[6].red(),collors[6].green(),collors[6].blue());
    setRegion(z_all[5], z_all[6],collors[6].red(),collors[6].green(),collors[6].blue(),collors[7].red(),collors[7].green(),collors[7].blue());
    setRegion(z_all[6],501,collors[7].red(),collors[7].green(),collors[7].blue(),collors[8].red(),collors[8].green(),collors[8].blue());
    TImgW->SetPosLin(z_all);//2.46

    update();
}
void CustomPalW::SetPaletteToDisplay()
{
    QPixmap pic = QPixmap::fromImage(TImgW->img_pal);
    emit EmitCustomPalette(pic,collors[9],collors[10]);
}


