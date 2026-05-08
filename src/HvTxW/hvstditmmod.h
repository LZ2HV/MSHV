/* MSHV HvStandardItemModel
 * Copyright 2023 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#ifndef HVSTDITMMOD_H
#define HVSTDITMMOD_H
#include <QStandardItemModel>

class HvStandardItemModel : public QStandardItemModel
{
    Q_OBJECT
public:
    HvStandardItemModel(QObject *parent = 0);
    virtual ~HvStandardItemModel();
    void SetValidSortColumns(int,int);
    void sort(int column,Qt::SortOrder order);
private:
    int sc0;
    int sc1;
    QList<QStringList>list_;
signals:
    void EmitSortEnd();
};

#endif
