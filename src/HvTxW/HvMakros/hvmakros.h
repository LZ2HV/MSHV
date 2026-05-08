/* MSHV
 *
 * By Hrisimir Hristov - LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */

#ifndef HVMAKROS_H
#define HVMAKROS_H

#include <QDialog>
#include <QLabel>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QFile>
#include <QTextStream>
#include <QPushButton>
#include <QRadioButton>
#include <QGroupBox>
#include <QCheckBox>
#include <QComboBox>


//#include "../config.h"
#include "../hvinle.h"

//#include <QtGui>

class HvMakrIn : public QWidget
{
    //Q_OBJECT
public:
    HvMakrIn(int ident, QWidget * parent = 0);
    virtual ~HvMakrIn();

    HvLeWithSpace *line_txt;
//public slots:

//signals:

//private slots:

private:
    int s_ident;

//protected:

};

#include "../hvqthloc.h"

class HvMakros : public QWidget
{
    Q_OBJECT
public:
    HvMakros(QString AppP, bool,QWidget *parent = 0);
    virtual ~HvMakros();

    void SetMacros();
    void ModeChanget(int mode);
    void SetFont(QFont);
    
signals:
    void EmitMacros(QStringList,int cont_id,QString my_cont_exch,QString);//2.32
    void EmitDistUnit(bool);
    void EmitRptRsq(bool);

//slots:

private slots:
    void Check(QString);
    void gen_mess();
    void MacrChanged(bool);
    void SetDefaultMacros_a();
    void SetDefaultMacros_b();
    void DistanceChanged(bool);
    void CbContNameChanged(int);
    void BExchangeChanged(QString);
    void CbContTrmNChanged(int);
    void SetClose();
    void TwTabClicked(int);

private:
	QWidget *w_parent;
	QString sr_path;
	bool dsty;
	bool allq65;
    int s_mode;
    QRadioButton *rb_eu;
    QRadioButton *rb_na;
    QRadioButton *rb_au;
    QRadioButton *rb_drid;
    QRadioButton *rb_contest;
    QRadioButton *rb_rep;

    int id_boption;
    QComboBox *cb_contests;
    HvLeWithSpace *le_exchfd;
    QLabel *l_exchru;
    HvLeWithSpace *le_exchru;
    QComboBox *cb_cabrillo_trmN;

    bool f_block_gen_msg;
    QRadioButton *rb_km;
    QRadioButton *rb_mi;

    HvInLe *le_loc;
    HvInLe *le_mycall;
    QVBoxLayout *V_la;
    QVBoxLayout *V_lb;
    int count_tx_widget;
    //QString AppPath;
    
    void SetDefaultMacrosRep();
    void SetDefaultMacrosGrid();
    void SetDefaultMacrosContest();
    
    void CheckAllowedModesActivity();//2.51
    void SendMacros_p();   
    bool isFindId(QString id,QString line,QString &res); 
    void ReadSettings();    
    void SaveSettings();
    bool isErrorRUExchDig(QString);
    
    //QLineEdit *le_mycall;
    //QLineEdit *le_loc;
    
    /*void closeEvent(QCloseEvent*)//2.60 stop
    {
        SaveSettings();
        SetMacros();
    }*/
    HvQthLoc THvQthLoc;
};
#endif
