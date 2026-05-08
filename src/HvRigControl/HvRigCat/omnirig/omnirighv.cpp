/* MSHV Part from RigControl
 * Copyright 2019 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#include "omnirighv_def.h" 
#include "omnirighv.h"
#if defined _WIN32_
//#include <QtGui>

OmniRigHV::OmniRigHV(int ModelID,QWidget *parent)
        : QWidget(parent)
{
    s_ModelID = ModelID;
    s_rig_name = rigs_omnirighv[s_ModelID].name;
    s_CmdID = -1;
    //s_read_array.clear();
    ptt_dtr = false;
    ptt_rts = false;
    ptt_cat = false;
    ptt_off = false;
    rts_on = false;
    dtr_on = false;
    GetFreqMethod = -1;//-1=false 0=freq 1=freqa
    SetFreqMethod = -1;//-1=false 0=freq 1=freqa
    TOmniRig = NULL;
    TRigX = NULL;
    TPortBits = NULL;
    //qDebug()<<"CreateOmniRigHV"<<rigs_omnirighv[s_ModelID].name;
}
OmniRigHV::~OmniRigHV()
{
    //qDebug()<<"DeleteOmniRigHV"<<rigs_omnirighv[s_ModelID].name;
}
bool OmniRigHV::InitOmniRigHV()
{
    TOmniRig = new OmniRigX(this);
    if (TOmniRig->isNull())
    {
        emit EmitOmniRigActive(false,"");
        QMessageBox::critical(this, "MSHV Fatal error","Failed to start OmniRig COM Server");
        return false;
    }
    if (s_rig_name=="Rig 1")
        TRigX = new OmniRig::RigX(TOmniRig->Rig1());
    if (s_rig_name=="Rig 2")
        TRigX = new OmniRig::RigX(TOmniRig->Rig2());
    if (TRigX->isNull())
    {
        emit EmitOmniRigActive(false,"");
        QMessageBox::critical(this, "MSHV Fatal error","Failed to Create RigX");
        return false;
    }
    TPortBits = new OmniRig::PortBits(TRigX->PortBits());
    if (TPortBits->isNull())
    {
        emit EmitOmniRigActive(false,"");
        QMessageBox::critical(this, "MSHV Fatal error","Failed to Create PortBits");
        return false;
    }

    //connect (&*TOmniRig, SIGNAL (exception (int, QString, QString, QString)), this, SLOT (SetException (int, QString, QString, QString)));
    connect (TOmniRig, SIGNAL (RigTypeChange(int)), this, SLOT(SetTypeChange(int)));//2.12
    //connect (&*TOmniRig, SIGNAL (StatusChange(int)), this, SLOT(SetStatusChange(int)));//no
    //connect (&*TOmniRig, SIGNAL (ParamsChange(int, int)), this, SLOT(ParamsChange(int, int)));

    if (TRigX->IsParamWriteable(OmniRig::PM_FREQA))
    {
        //qDebug()<<"VFO Set PM_FREQA";
        SetFreqMethod = 1;//-1=false 0=freq 1=freqa
    }
    else if (TRigX->IsParamWriteable(OmniRig::PM_FREQ))
    {
        //qDebug()<<"VFO Set PM_FREQ";
        SetFreqMethod = 0;//-1=false 0=freq 1=freqa
    }
    else
    {
        //qDebug()<<"VFO Set ERROR";
        SetFreqMethod = -1;//-1=false 0=freq 1=freqa
    }

    if (TRigX->IsParamReadable(OmniRig::PM_FREQA))
    {
        //qDebug()<<"VFO Get PM_FREQA";
        GetFreqMethod = 1;//-1=false 0=freq 1=freqa
    }
    else if (TRigX->IsParamReadable(OmniRig::PM_FREQ))
    {
        //qDebug()<<"VFO Get PM_FREQ";
        GetFreqMethod = 0;//-1=false 0=freq 1=freqa
    }
    else
    {
        //qDebug()<<"VFO Get ERROR";
        GetFreqMethod = -1;//-1=false 0=freq 1=freqa
    }
    //qDebug()<<"VFO Set METHOD="<<SetFreqMethod<<"VFO Get METHOD="<<GetFreqMethod;
    emit EmitOmniRigActive(true,"OmniRig "+s_rig_name+" = "+TRigX->RigType());//2.76.1 for pskreporter (=) <- is ID
    return true;
}
void OmniRigHV::SetTypeChange(int numb)
{	
	if(s_rig_name.midRef(4,1).toInt()==numb)
	{
	   emit EmitOmniRigChanged();
	   //qDebug()<<"SetTypeChange="<<numb;
	}
}
void OmniRigHV::SetOmniRigPttMethod(bool *f)
{
    ptt_off = f[0];
    ptt_rts = f[1];
    ptt_dtr = f[2];
    ptt_cat = f[3];
    rts_on =  f[4];
    dtr_on =  f[5]; 		
    if (ptt_dtr) TPortBits->SetDtr(false);    	    
    if (ptt_rts) TPortBits->SetRts(false); 	    		
    if (rts_on) TPortBits->SetRts(true);
    else TPortBits->SetRts(false);
    if (dtr_on) TPortBits->SetDtr(true);
    else TPortBits->SetDtr(false);       		   	
    //qDebug()<<"SetOmniRigPttMethod"<<f[0]<<f[1]<<f[2]<<f[3]<<f[4];
}
void OmniRigHV::SetCmd(CmdID i,ptt_t ptt,QString str)
{
    switch (i)
    {
    case GET_SETT:
        emit EmitRigSet(rigs_omnirighv[s_ModelID]);
        break;
    case SET_PTT:
        set_ptt(ptt);
        break;
    case SET_FREQ:
        set_freq(str.toLongLong());
        break;
    case GET_FREQ:
        s_CmdID = GET_FREQ;
        get_freq();
        break;
    case SET_MODE:
        set_mode(str);
        break;
    case GET_MODE:
        s_CmdID = GET_MODE;
        get_mode();
        break;
    }
}
void OmniRigHV::set_ptt(ptt_t ptt)
{
    char *a = (char*)"1221";
    emit EmitWriteCmd(a,4);//fictive stop poping timer
    if (ptt_cat)
    {
        //qDebug()<<"SetPTT_VIA_CAT"<<ptt;
        if (ptt==RIG_PTT_ON)
            TRigX->SetTx(OmniRig::PM_TX);
        else
            TRigX->SetTx(OmniRig::PM_RX);
    }
    else if (ptt_rts)
    {
        //qDebug()<<"SetPTT_VIA_RTS"<<ptt;
        if (ptt==RIG_PTT_ON)
            TPortBits->SetRts(true);
        else
            TPortBits->SetRts(false);
    }
    else if (ptt_dtr)
    {
        //qDebug()<<"SetPTT_VIA_DTR"<<ptt;
        if (ptt==RIG_PTT_ON)
            TPortBits->SetDtr(true);
        else
            TPortBits->SetDtr(false);
    }
}
////////////////////////////////////////////////////////new read com
void OmniRigHV::set_freq(unsigned long long freq)
{
    char *a = (char*)"1221";
    emit EmitWriteCmd(a,4);//fictive stop poping timer
    if (SetFreqMethod==1)//-1=false 0=freq 1=freqa
        TRigX->SetFreqA(freq);
    else if (SetFreqMethod==0)//-1=false 0=freq 1=freqa
        TRigX->SetFreq(freq);
    //qDebug()<<"SetFreq="<<freq;
}
void OmniRigHV::set_mode(QString str)
{
    char *a = (char*)"1221";
    emit EmitWriteCmd(a,4);//fictive stop poping timer
    if 		(str=="USB" ) TRigX->SetMode(OmniRig::PM_SSB_U);
    else if (str=="LSB" ) TRigX->SetMode(OmniRig::PM_SSB_L);
    else if (str=="DIGU") TRigX->SetMode(OmniRig::PM_DIG_U);
    //qDebug()<<"SetMode="<<str;
}
void OmniRigHV::get_freq()
{
    unsigned long long f = 0;
    if (GetFreqMethod==1)//-1=false 0=freq 1=freqa
        f = TRigX->FreqA();
    else if (GetFreqMethod==0)//-1=false 0=freq 1=freqa
        f = TRigX->Freq();
    //qDebug()<<"GetFreq"<<f;
    if(f<10) return;//protect -22 or 0 
    emit EmitReadedInfo(GET_FREQ,QString("%1").arg(f));
    /*EmitWriteCmd(cmdnc,frq.count());*/
}
void OmniRigHV::get_mode()
{
    QString smode = "WRONG_MODE";
    if 		(TRigX->Mode()==OmniRig::PM_SSB_L) smode = "LSB";
    else if (TRigX->Mode()==OmniRig::PM_SSB_U) smode = "USB"; 
    else if (TRigX->Mode()==OmniRig::PM_CW_L)  smode = "CWL";
    else if (TRigX->Mode()==OmniRig::PM_CW_U)  smode = "CWU";
    else if (TRigX->Mode()==OmniRig::PM_DIG_L) smode = "DIGL";        
    else if (TRigX->Mode()==OmniRig::PM_DIG_U) smode = "DIGU";
    else if (TRigX->Mode()==OmniRig::PM_AM)    smode = "AM";
    else if (TRigX->Mode()==OmniRig::PM_FM)    smode = "FM";
    	    	  
    emit EmitReadedInfo(GET_MODE,smode);
    //qDebug()<<"GetMode"<<smode;
    /*EmitWriteCmd(cmdnc,mod.count());*/
}
/*void OmniRigHV::SetReadyRead(QByteArray,int)
{

}*/
////////////////////////////////////////////////////////end new read com
#endif