/* MSHV FontDialog
 * Copyright 2019 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#ifndef HVFONTDIALOG_H
#define HVFONTDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QFont>
#include <QFontComboBox>
#include <QFontDatabase>
#include <QSpinBox>
#include <QLabel>
#include <QGroupBox>
#include <QApplication>
#include <QPushButton>
class HvFontDialog : public QDialog
{
    Q_OBJECT
public:
    HvFontDialog(QString path,QWidget * parent);
    virtual ~HvFontDialog();

    void SetFontsAll(QString);
    QString GetFontsAll();

signals:
    void EmitFontList(QFont);
    void EmitFontApp(QFont);

public slots:
   void SetDefFont();
    
private slots:
   void FontAppChange();
   void FontListChange();

private:
   QFontComboBox *comboFontApp;
   QSpinBox *fontSizeApp;
   QFontComboBox *comboFontList;
   QSpinBox *fontSizeList;
   QPushButton *pb_SetDefault;
   bool block_emit_fonts;

};
#endif