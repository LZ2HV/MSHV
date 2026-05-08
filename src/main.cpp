/* MSHV Main
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */

/*
#include <iostream>
#include <exception>
#include <stdexcept>
#include <string>
*/

#include <QApplication>
#include <QSplashScreen>
#include "main_ms.h"
#include "config.h"
//#include <unistd.h> //usleep

//#define TXT_CODEC_LIN "UTF-8"
//#define TXT_CODEC_LIN "Windows-1251"

//#include <QtGui>
/* no used just test
namespace
{
class ExceptionCatchingApplication final
            : public QApplication
{
public:
    explicit ExceptionCatchingApplication (int& argc, char * * argv)
            : QApplication {argc, argv}
    {
    }
    bool notify (QObject * receiver, QEvent * e) override
    {
        try
        {
            return QApplication::notify (receiver, e);
        }
        catch (std::exception const& e)
    {
        QMessageBox::critical(nullptr, "MSHV Fatal error", e.what ());
            throw;
        }
        catch (...)
    {
        QMessageBox::critical(nullptr, "MSHV Unexpected fatal error","...");
            throw;
        }
    }
};
}
*/ 
QSplashScreen *splash = 0;
int main(int argc, char ** argv)
{
    //QApplication::setDesktopSettingsAware(false); remove desktop settings
    QApplication app(argc,argv);
    /* no used just test
    ExceptionCatchingApplication app(argc, argv);
    try
    {
    */
    // for RUS ver
    /*QTextCodec::setCodecForLocale (QTextCodec::codecForName (TXT_CODEC_LIN)) ;// UTF-8 hv zaradi imena na failove na BG
    QTextCodec::setCodecForCStrings (QTextCodec::codecForName(TXT_CODEC_LIN));    
    QTextCodec::setCodecForTr(QTextCodec::codecForName(TXT_CODEC_LIN));*/

    _ReadSSAndSet_();
    QString inst0 = "";
    if (argc>1)//multi-instance
    {
        QString err = ""; //QString oko = "";
        for (int i = 1; i<argc; ++i)
        {
            QString tst = argv[i]; //qDebug()<<tst;
            if (!tst.startsWith("-")) continue;
            if (tst.startsWith("--inst-name=")) inst0 = tst.mid(12,tst.count()-12);//else if (tst.startsWith("--rig-name=" )) inst0 = tst.mid(11,tst.count()-11);//N1MM Eception           	
            else
            {
                err.append(tst);
                if (i<argc-1) err.append(", ");
            }
        }
        if (!err.isEmpty()) 
        {
        	/*QString ttxt = "Command line error\n\nUnknown option: "+err+"\n";
        	if (!inst0.isEmpty()) ttxt.append("Correct option: --inst-name="+inst0+"\n");
        	ttxt.append("\nValid options is:\n--inst-name=\nExample: MSHV_XXX... --inst-name=VHF");
            QMessageBox::critical(nullptr,(QString)APP_NAME,ttxt);*/                                 
            //if (inst0.isEmpty()) return 0;
            QMessageBox::critical(nullptr,(QString)APP_NAME,"Command line error\nUnknown option: "+err+
                                  "\nValid options is:\n--inst-name=\nExample: MSHV_XXX... --inst-name=VHF");
            //return 0;//-1;
        }
        /*if (fver)
        {
        	QMessageBox::information(nullptr,"Application version",(QString)APP_NAME);
        	return 0;  		
        }*/
        if (!inst0.isEmpty()) inst0 = " "+inst0;
    }
    bool f_once = true;
    int time_ms = 3000;
    splash = new QSplashScreen(QPixmap(":/pic/mshv_splash.png"));
    QLabel *label = new QLabel(app.translate("main","Software version")+" "+(QString)VER_MS);
    label->setStyleSheet("QLabel {color :rgb(158, 204, 241); font: 10pt;}");
    QVBoxLayout *vl = new QVBoxLayout();
    vl->setContentsMargins (5, 22, 5, 5);
    //vl->setContentsMargins (142, 22, 5, 265);
    vl->addWidget(label);
    vl->setAlignment(label,Qt::AlignHCenter | Qt::AlignTop);
    //vl->setAlignment(label,Qt::AlignTop);
    //label->move(100,100);
    splash->setLayout(vl);

    //QFont font;
    //font.setPointSize(10);
    //splash->setFont( font );
    splash->show();
    app.processEvents();//2.60 for LINUX qt5 > 5.15.x
    //splash->showMessage(QObject::tr("Loading:")+" "+QObject::tr("Interface translation"), Qt::AlignRight | Qt::AlignTop,  Qt::white);
    //splash->showMessage("<h2>"+QObject::tr("version ")+VER_MS+"</h2>", Qt::AlignRight | Qt::AlignTop,  Qt::white);
    //splash->showMessage("\n"+QObject::tr("Software version ")+VER_MS, Qt::AlignHCenter  | Qt::AlignTop,  Qt::white);*/

    //app.processEvents();//This is used to accept a click on the screen so that user can cancel the screen

    QElapsedTimer time5;//2.56 stop QTimer
    time5.start();
    //app.processEvents();

    Main_Ms win(inst0);
    //app.processEvents();

    //QString dot = "";
    while (time5.elapsed()<time_ms)
    {
        usleep(300000);	// 10 times
        if (time5.elapsed()>time_ms/2 && f_once) //if (time5.elapsed()>time_ms/2)
        {
            label->setText(app.translate("main","Starting")+" MSHV");//label->setText(dot+app.translate("main","Starting")+" MSHV"+dot);
            f_once = false;
            app.processEvents(); //dot.append('.');
        }
    }
    win.show();

    if (win.is_active_astro_w) win.Start_astro_m->setChecked(true);
 
    splash->finish(&win);
    delete splash;
    splash = 0;
  
    app.connect( &app, SIGNAL( lastWindowClosed() ), &app, SLOT( quit() ) );
    //qDebug()<<"ffff"<<argc<<argv[0]<<argv[1];
    //FreeConsole();//for windows detached from console
    return app.exec();
    /* no used just test
    }
    catch (std::exception const& e)
    {
        std::cerr << "Error: " << e.what () << '\n';
    }
    catch (...)
    {
        std::cerr << "Unexpected fatal error\n";
        throw;			
    }
    return -1;
    */
}
