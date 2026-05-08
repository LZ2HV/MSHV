/* MSHV FontDialog
 * Copyright 2019 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#include "../config.h"
#include "hvfontdialog.h"
#include <QLineEdit>
//#include <QtGui>

HvFontDialog::HvFontDialog(QString path,QWidget *parent)
        : QDialog(parent)
{ 
	setWindowTitle (tr("Font Settings"));
    setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);// maha help butona
	QVBoxLayout *LV = new QVBoxLayout();
    LV->setContentsMargins(10, 10, 10, 10);
    LV->setSpacing(5);
    
    QGroupBox *GB_APP = new QGroupBox(tr("Application Font")+":");
    QLabel *lfapp = new QLabel(tr("Family")+":");
    comboFontApp = new QFontComboBox();
    comboFontApp->lineEdit()->setFrame(true); 
    fontSizeApp = new QSpinBox();
    fontSizeApp->setRange(6,13);
    QHBoxLayout *happ = new QHBoxLayout();
    happ->setContentsMargins ( 1, 1, 1, 1);
    happ->setSpacing(5);
    happ->addWidget(lfapp);
    happ->addWidget(comboFontApp);
    happ->addWidget(fontSizeApp);
    GB_APP->setLayout(happ);
    
    QGroupBox *GB_LIST = new QGroupBox(tr("List Font")+":");
    QLabel *lflist = new QLabel(tr("Family")+":");
    comboFontList = new QFontComboBox();
    comboFontList->lineEdit()->setFrame(true);
    fontSizeList = new QSpinBox();
    fontSizeList->setRange(6,14);
    QHBoxLayout *hlist = new QHBoxLayout();
    hlist->setContentsMargins ( 1, 1, 1, 1);
    hlist->setSpacing(5);
    hlist->addWidget(lflist);
    hlist->addWidget(comboFontList);
    hlist->addWidget(fontSizeList);
    GB_LIST->setLayout(hlist);    
    
    comboFontApp->clear(); 
    comboFontList->clear(); 
    
    QFontDatabase db;     
#if defined _WIN32_
    db.addApplicationFont(path + "/settings/resources/font/cour.ttf");   //:font/cour.ttf  arial.ttf 
#endif
#if defined _LINUX_
    db.addApplicationFont(path + "/settings/resources/font/DejaVuSansMono.ttf");
#endif
    db.addApplicationFont(path + "/settings/resources/font/arial.ttf"); 
    
    //QStringList fontList = db.applicationFontFamilies(nFontList);
    //fontList << db.applicationFontFamilies(nFontApp);
    //qDebug()<<fontList;
    //int nFontList = db.addApplicationFont(":font/DejaVuSansMono.ttf");
    //QStringList fontList = db.applicationFontFamilies(nFontList);
    //qDebug()<<fontList; 

    
    //QFont f(fontList[1], 9);
    comboFontApp->addItems(db.families());
    //comboFontApp->setCurrentIndex(comboFontApp->findText(f.family()));    
    //fontSizeApp->setValue(f.pointSize());
    
    //QFont flist(fontList[0], 10);
    comboFontList->addItems(db.families());
    //comboFontList->setCurrentIndex(comboFontList->findText(flist.family()));    
    //fontSizeList->setValue(flist.pointSize());
    
    
    pb_SetDefault = new QPushButton(tr("SET DEFAULT FONT"));
    LV->addWidget(GB_APP);
    LV->addWidget(GB_LIST);
    LV->addWidget(pb_SetDefault);
    this->setLayout(LV);
    
    block_emit_fonts = false;
    
    connect(comboFontApp, SIGNAL(currentFontChanged(QFont)), this, SLOT(FontAppChange()));
    connect(fontSizeApp, SIGNAL(valueChanged(int)), this, SLOT(FontAppChange()));
    connect(comboFontList, SIGNAL(currentFontChanged(QFont)), this, SLOT(FontListChange()));
    connect(fontSizeList, SIGNAL(valueChanged(int)), this, SLOT(FontListChange()));
    connect(pb_SetDefault, SIGNAL(clicked(bool)), this, SLOT(SetDefFont()));
    //FontAppChange();
    //FontListChange();  "Courier New", "Arial"  DejaVu Sans Mono
}
HvFontDialog::~HvFontDialog()
{	
}
void HvFontDialog::SetDefFont()
{
#if defined _WIN32_
    SetFontsAll("Arial#9#Courier New#10");
#endif
#if defined _LINUX_
    SetFontsAll("Arial#9#DejaVu Sans Mono#10");
#endif
}
QString HvFontDialog::GetFontsAll()
{
	QString res =comboFontApp->currentText();
	res.append("#");
	res.append(QString("%1").arg(fontSizeApp->value()));
    res.append("#");
	res.append(comboFontList->currentText());
	res.append("#");
	res.append(QString("%1").arg(fontSizeList->value()));
	return res;
}
void HvFontDialog::SetFontsAll(QString s)
{
	bool have_change_app = false;
	bool have_change_list = false;
	
	if(s.isEmpty())
		return;
	QStringList ls = s.split("#");
	if(ls.count()!=4)
		return;					
			
	block_emit_fonts = true;			
	if(comboFontApp->findText(ls.at(0))>-1)
	{
	   comboFontApp->setCurrentIndex(comboFontApp->findText(ls.at(0)));
	   fontSizeApp->setValue(ls.at(1).toInt());
	   have_change_app = true;
	}
	
	if(comboFontList->findText(ls.at(2))>-1)
	{
		comboFontList->setCurrentIndex(comboFontApp->findText(ls.at(2)));
		fontSizeList->setValue(ls.at(3).toInt());	
		have_change_list = true;	
	}	
	block_emit_fonts = false;
	
	if(have_change_app)		
		FontAppChange();
	if(have_change_list)
		FontListChange();
}
void HvFontDialog::FontAppChange()
{
	if(block_emit_fonts) return;	
	QFont f(comboFontApp->currentFont());
	f.setPointSize(fontSizeApp->value());	
	QApplication::setFont(f);	
	emit EmitFontApp(f);
}
void HvFontDialog::FontListChange()
{
	if(block_emit_fonts) return;	
	QFont f(comboFontList->currentFont());
	f.setPointSize(fontSizeList->value());	
	emit EmitFontList(f);	
}
