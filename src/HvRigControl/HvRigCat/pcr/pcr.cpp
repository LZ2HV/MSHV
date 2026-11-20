/* MSHV Part from RigControl
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#include "pcr_def.h"
#include "pcr.h"
//#include <QtGui>

Pcr::Pcr(int ModelID,QWidget *parent)
        : QWidget(parent)
{
    s_ModelID = ModelID;
    ////////////////////////////////////////////////////////new read com
    //s_rig_name = rigs_kenwood[s_ModelID].name;
    s_CmdID = -1;
    s_read_array.clear();
    ////////////////////////////////////////////////////////end new read com
}

Pcr::~Pcr()
{
    //qDebug()<<"Delete"<<rigs_Elecraft[s_ModelID].name;
}
void Pcr::SetCmd(CmdID i,ptt_t /*ptt*/,QString str)
{
    switch (i)
    {
    case GET_SETT:
        emit EmitRigSet(rigs_pcr[s_ModelID]);
        break;
    case SET_PTT:
        //set_ptt(ptt);
        break;
        ////////////////////////////////////////////////////////new read com
    case SET_FREQ:
        set_freq(str.toLongLong());
        break;
    case GET_FREQ:
        //s_CmdID = GET_FREQ;
        //get_freq();
        break;
    case SET_MODE:
        set_mode(str);
        break;
    case GET_MODE:
        //s_CmdID = GET_MODE;
        //get_mode();
        break;
        ////////////////////////////////////////////////////////end new read com
    }
}
////////////////////////////////////////////////////////new read com
//#define PRIll "lld"
/*
 * pcr_set_freq
 * Assumes rig!=NULL 
 *
 * K0GMMMKKKHHHmmff00
 * GMMMKKKHHH is frequency GHz.MHz.KHz.Hz
 * mm is the mode setting
 *  00 = LSB
 *  01 = USB
 *  02 = AM
 *  03 = CW
 *  04 = Not used or Unknown
 *  05 = NFM
 *  06 = WFM
 * ff is the filter setting
 *  00 = 2.8 Khz (CW USB LSB AM)
 *  01 = 6.0 Khz (CW USB LSB AM NFM)
 *  02 = 15  Khz (AM NFM)
 *  03 = 50  Khz (AM NFM WFM)
 *  04 = 230 Khz (WFM)
 *
 */
 //tova sa priemnici PC triabva on/off i samo da se setva display e tova koeto si setnal
 //za sega ne
void Pcr::set_freq(unsigned long long /*freq*/)
{
 
}
void Pcr::set_mode(QString /*str*/)
{

}
/*
void Pcr::get_freq()
{
}
void Pcr::get_mode()
{
}
void Pcr::SetReadyRead(QByteArray ar,int size)
{
}
*/
