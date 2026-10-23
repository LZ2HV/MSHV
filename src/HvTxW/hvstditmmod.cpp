/* MSHV HvStandardItemModel
 * Copyright 2023 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#include "hvstditmmod.h"
//#include <QtGui>

class HvSortAorder
{
public:
    inline HvSortAorder(int i)
    {
        i_col = i;
    };
    inline bool operator()(const QStringList &d1,const QStringList &d2) const
    {
    	if (d1.at(i_col).isEmpty()) return true;  //Empty first
    	if (d2.at(i_col).isEmpty()) return false; //Empty first
        return d1.at(i_col).toInt() < d2.at(i_col).toInt();
    };
private:    
    int i_col;
};
class HvSortDorder
{
public:
    inline HvSortDorder(int i)
    {
        i_col = i;
    };
    inline bool operator()(const QStringList &d1,const QStringList &d2) const
    {
    	if (d1.at(i_col).isEmpty()) return true;  //Empty first
    	if (d2.at(i_col).isEmpty()) return false; //Empty first   	
        return d1.at(i_col).toInt() > d2.at(i_col).toInt();
    };
private:    
    int i_col;
};
HvStandardItemModel::HvStandardItemModel(QObject *parent)
        : QStandardItemModel(parent)
{
	sc0 = -12; //not exist
	sc1 = -12; //printf("Create\n"); 
}
HvStandardItemModel::~HvStandardItemModel()
{}
/*static int i_col_ = 1;
bool hv_sort_aorder_(const QStringList &d1,const QStringList &d2)
{
    return d1.at(i_col_).toInt() < d2.at(i_col_).toInt();
}
bool hv_sort_dorder_(const QStringList &d1,const QStringList &d2)
{
    return d1.at(i_col_).toInt() > d2.at(i_col_).toInt();
}*/
void HvStandardItemModel::SetValidSortColumns(int c0,int c1)
{
    sc0 = c0;
    sc1 = c1;
}
void HvStandardItemModel::sort(int column, Qt::SortOrder order)
{	
    if (column==sc0 || column==sc1)// distance snr special sort
    {
        //printf ("sc0=%d sc1=%d column=%d\n",sc0,sc1,column);
        //if (column >= columnCount()) return;
        if (rowCount() < 2) return; //if have min 2 item
        list_.clear();
        for (int i = 0; i < rowCount(); ++i)
        {
            QStringList itml;
            for (int j = 0; j < columnCount(); ++j)
            {
                itml << item(i,j)->text();
            }
            list_.append(itml);
        }        
        if (order == Qt::AscendingOrder) 
        {
        	HvSortAorder aord(column);
        	std::sort(list_.begin(),list_.end(),aord);//Ascending
       	}       	
        else 
        {
        	HvSortDorder dord(column);
        	std::sort(list_.begin(),list_.end(),dord);//Descending        	
       	}
       	/*i_col_ = column;
        if (order == Qt::AscendingOrder) std::sort(list_.begin(),list_.end(),hv_sort_aorder_);//Ascending
        else std::sort(list_.begin(),list_.end(),hv_sort_dorder_);//Descending*/
        for (int i = 0; i < rowCount(); ++i)
        {
            for (int j = 0; j < columnCount(); ++j)
            {
                QStringList l = list_.at(i);
                QString g = l.at(j);
                QStandardItem *item = itemFromIndex(index(i, j));
                item->setText(g);
                setItem(i,j,item);
            }
        } //end bug qt5.7 if setItem 0 row show hiden columns
        emit EmitSortEnd();
    }
    else QStandardItemModel::sort(column,order);
}

