/* The algorithms, source code, look-and-feel of WSJT-X and related
 * programs, and protocol specifications for the modes FSK441, FT8, JT4,
 * JT6M, JT9, JT65, JTMS, QRA64, ISCAT, MSK144, are Copyright © 2001-2017
 * by one or more of the following authors: Joseph Taylor, K1JT; Bill
 * Somerville, G4WJS; Steven Franke, K9AN; Nico Palermo, IV3NWV; Greg Beam,
 * KI7MT; Michael Black, W9MDB; Edson Pereira, PY2SDR; Philip Karn, KA9Q;
 * and other members of the WSJT Development Group.
 *
 * MSHV RadioAndNetW
 * Rewritten into C++ and modified by Hrisimir Hristov, LZ2HV 2015-2017
 * May be used under the terms of the GNU General Public License (GPL)
 */
// KISS Interface for posting spots to PSK Reporter web site
// Implemented by Edson Pereira PY2SDR
//
// Reports will be sent in batch mode every 5 minutes.

#include "radionetw.h"

//#include <QHostInfo>//HV is here QtGui
//#include <QTimer>

#include "MessageClient.h"

//#include <QtGui>
HvTcpClient::HvTcpClient(QObject *parent) : QObject(parent)
{
	//stat_disp = 0;
    tray_once_agen = false;
    socket = new QTcpSocket(this);
    connect(socket, SIGNAL(readyRead()), this, SLOT(readTelnet()));
    connect(socket, SIGNAL(connected()), this, SLOT(connected_s()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected_s()));
    s_pass = "";
    rx_login.setPattern("(login:|callsign:|login :|callsign :)");
    rx_login.setCaseSensitivity(Qt::CaseInsensitive);
    rx_passw.setPattern("(password:|password :)");
    rx_passw.setCaseSensitivity(Qt::CaseInsensitive);
}
#include "../../pfx_sfx.h"
bool HvTcpClient::is_pfx(QString s)
{
    bool res = false;
    for (int i = 0; i < NZ; i++)
    {
        if (s==pfx[i])
        {
            res = true;
            break;
        }
    }
    return res;
}
QString HvTcpClient::FindBaseCallRemAllSlash(QString str)
{
    QString res;
    if (str.contains("/"))
    {
        QStringList list_str = str.split("/");
        if (list_str.count() >= 3) res = list_str.at(1);//more then 2 shlashes SP9/LZ2HV/QRP
        else
        {// only 2 count
            bool call_0 = false;
            bool call_1 = false;
            if (!is_pfx(list_str.at(0)) && THvQthLoc.isValidCallsign(list_str.at(0))) call_0 = true;
            if (!is_pfx(list_str.at(1)) && THvQthLoc.isValidCallsign(list_str.at(1))) call_1 = true;
            if 		(call_0 &&  call_1) res = list_str.at(1);	//bin=11 int=3
            else if (call_0 && !call_1) res = list_str.at(0);	//bin=10 int=2
            else if (!call_0 && call_1) res = list_str.at(1);	//bin=01 int=1
            else res = list_str.at(0);							//bin=00 int=0
        }
    }
    else res = str;
    return res;
}
void HvTcpClient::Connect(QString host,QString port,QString login_name,QString pas)
{
	//stat_disp = 0;
    s_tcphost = host;
    s_tcpport = port;
    s_login_name = FindBaseCallRemAllSlash(login_name);
    s_pass = pas;
    connectToHost();
}
/*void HvTcpClient::SetData(QString data)
{
    s_tcpdata = data;
}*/
//#include <unistd.h>
void HvTcpClient::connected_s()
{
    //emit ConectionInfo("<font color='#00b300'>"+tr("Connected to")+" "+s_tcphost+" "+tr("and logged in as")+" "+s_login_name+"</font>");
    //stat_disp = 0;
    emit ConectionInfo("<font color='#00b300'>"+tr("Connected to")+" "+s_tcphost+"</font>");
}
void HvTcpClient::disconnected_s()
{
    //stat_disp = 0;
    emit ConectionInfo("<font color='red'>"+tr("Disconnected")+itcp_call_err+"</font>");
    if (tray_once_agen)
    {
        tray_once_agen = false;
        connectToHost();
    }//qDebug()<<"disconnected_s=";
}
void HvTcpClient::DisconectOnly()
{
	//stat_disp = 0;
    if (socket->state() == QAbstractSocket::ConnectedState)
    {
        writeData("bye"); //qDebug()<<"DisconectOnly=";
    }
}
void HvTcpClient::connectToHost()
{
	//stat_disp = 0;
    if (socket->state() == QAbstractSocket::ConnectedState)
    {
        writeData("bye");
        tray_once_agen = true; //qDebug()<<"Disconnect="<<s_tcphost<<s_tcpport<<s_login_name<<tray_once_agen;
        return;
    } //qDebug()<<"connectToHost="<<s_tcphost<<s_tcpport<<s_login_name<<tray_once_agen;
    int p1 = s_tcpport.toInt();
    socket->connectToHost(s_tcphost, p1);
    socket->waitForConnected(350);//importent for TCP hv
}
void HvTcpClient::readTelnet()
{
	static uint8_t stat_disp = 0;
	//if (stat_disp > 10) return;	//end refresh read
    QByteArray ba = socket->readAll();
    QString itcptxt = ba.data();
    itcptxt.remove("\r");
    itcptxt.remove("\n");
    itcptxt.remove("\007");
	//if (itcptxt.startsWith("DX de")) return;
    //login: LZ2HV/P Sorry LZ2HV/P is an invalid callsign
    if (itcptxt.contains("invalid callsign")) itcp_call_err = " "+s_login_name+" is an invalid callsign";        
    else itcp_call_err="";
    //itcptxt= "callsign:"; 
    //printf("%s\n",qPrintable(ba.data())); printf("------------- RX MSG END ---------------\n");  
    if (itcptxt.contains(rx_login))//if ((itcptxt.contains("login",Qt::CaseInsensitive) || itcptxt.contains("callsign",Qt::CaseInsensitive)) && itcptxt.contains(":"))
    {
    	writeData(s_login_name); //printf("   --MSHV SEND login Call-->%s\n",qPrintable(s_login_name));
    	stat_disp = 1;               	
   	} 	
   	if (itcptxt.contains(rx_passw))//if (itcptxt.contains("password",Qt::CaseInsensitive) && itcptxt.contains(":"))
    {
    	writeData(s_pass); //printf("   --MSHV SEND Password-->%s\n",qPrintable(s_pass));
    	stat_disp = 2;
   	}
   	//LZ2HV\n Hello Unknown Name.<-for this exception  stop-> itcptxt.contains("Hello "+s_login_name) 
   	if (itcptxt.contains("Hello ",Qt::CaseInsensitive) && itcptxt.contains(s_login_name,Qt::CaseInsensitive))
   	{
   		//if (itcptxt.contains("not registered",Qt::CaseInsensitive) stat_disp == 1;
   		QString sci = "<font color='#00b300'>"+tr("Connected to")+" "+s_tcphost+" "+tr("and logged in as")+" "+s_login_name;
   		if (stat_disp == 1) 
   		{
   			//stat_disp = 6;//end refresh read after 4 reads
   			emit ConectionInfo(sci+"</font>");   			
  		} 
   		if (stat_disp == 2)
   		{   			
   			//stat_disp = 6;//end refresh read after 4 reads
			emit ConectionInfo(sci+" "+tr("Registered")+"</font>");   			
  		}		
  	} 
  	//if (stat_disp >= 6) stat_disp++; 
  	//qDebug()<<"stat_disp="<<stat_disp;  	    		 	    
    /*switch (socket->state ())
    {
     case QAbstractSocket::UnconnectedState:
        qDebug()<< (tr("unconnected state"));
        break;
     case QAbstractSocket::HostLookupState:
        qDebug()<< (tr("host lookup state"));
        break;
     case QAbstractSocket::ConnectingState:
        qDebug()<< (tr("connecting state"));
        break;
     case QAbstractSocket::ConnectedState:
        qDebug()<< (tr("connected state"));
        break;
     case QAbstractSocket::BoundState:
        qDebug()<< (tr("bound state"));
        break;
     case QAbstractSocket::ClosingState:
        qDebug()<< (tr("Closing state"));
        break;
     case QAbstractSocket::ListeningState:
        qDebug()<< (tr("Listening  state"));
        break;
     default:
        qDebug()<< (tr("Unknown state"));
        break;
    }*/
}
bool HvTcpClient::writeData(QString str)
{   
	//qDebug()<<"   --writeData-->"<<str;
    if (socket->state() == QAbstractSocket::ConnectedState)
    {
        str.append("\r\n");//end of msg
        QByteArray data = str.toUtf8();
        //socket->write(IntToArray(data.size())); //write size of data
        socket->write(data); //write the data itself
        //socket->write(result.toLocal8Bit());
        //qDebug()<<"close"<<socket->readAll();
        return socket->waitForBytesWritten(300); //500
    }
    else return false;
}

SpotDialog::SpotDialog(bool indsty,QWidget * parent)
        : QDialog(parent)
{
    f_km_mi = true;
    send_spot_result = false;
    //this->setMinimumSize(100,100);

    this->setWindowTitle(tr("Spot Dialog"));
    setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);

    le_my_call = new HvInLe("call", tr("My Call")+":",indsty);
    le_my_call->setMinimumWidth(126);
    le_my_call->setMaxLength(15);
    le_my_call->setReadOnly(true);
    //le_my_call->setMaximumWidth(220);

    le_my_loc = new HvInLe("locator", tr("My Locator")+":",indsty);
    le_my_loc->setMinimumWidth(126);
    le_my_loc->setMaxLength(6);
    le_my_loc->setReadOnly(true);
    //le_my_loc->setMaximumWidth(220);

    le_freq = new HvInLe("freq", tr("Frequency In")+" kHz:",indsty);
    //QRegExp rx("[0-9]*");
    QRegExp rx("^[1-9][0-9]*$");//^[2-9][0-9]{6}$ Out of 7 digits 1 is consumed by first position 2-9 and then next 6 digits can be from 0-9

    QValidator *validator = new QRegExpValidator(rx, this);
    le_freq->SetValidatorHv(validator);// only digits
    le_freq->setMinimumWidth(126);
    le_freq->setMaxLength(9);
    connect(le_freq, SIGNAL(EmitSndCheck(QString)), this, SLOT(CheckForWalidSpot(QString)));
    connect(le_freq, SIGNAL(EmitSndCheck(QString)), this, SLOT(Check(QString)));
    connect(le_freq, SIGNAL(EmitEntered()), this, SLOT(FreqEntered()));

    le_dx_call = new HvInLe("call", tr("DX Call")+":",indsty);
    le_dx_call->setMinimumWidth(126);
    le_dx_call->setMaxLength(15);
    connect(le_dx_call, SIGNAL(EmitSndCheck(QString)), this, SLOT(Check(QString)));
    connect(le_dx_call, SIGNAL(EmitEntered()), this, SLOT(DxCallEntered()));
    //le_dx_call->setMaximumWidth(220);

    QLabel *l_loc_fr_db =  new QLabel("DB: ");
    l_loc_from_db = new QLabel();
    l_loc_from_db->setText("NA");
    QPushButton *b_lookup_loc = new QPushButton(tr("LOOKUP"));
    b_lookup_loc->setDefault(false);
    b_lookup_loc->setAutoDefault(false);//stop enter event
    b_lookup_loc->setFixedHeight(22);
    b_lookup_loc->setFixedWidth(85);
    //connect(b_lookup_loc, SIGNAL(clicked(bool)), this, SLOT(LookupLocFromDb()));
    connect(b_lookup_loc, SIGNAL(released()), this, SLOT(LookupLocFromDb()));
    le_dx_loc = new HvInLe("locator", tr("DX Locator")+":",indsty);
    le_dx_loc->setMinimumWidth(126);
    //le_dx_loc->setFixedWidth(130);
    le_dx_loc->setMaxLength(6);
    connect(le_dx_loc, SIGNAL(EmitSndCheck(QString)), this, SLOT(Check(QString)));
    connect(le_dx_loc, SIGNAL(EmitEntered()), this, SLOT(LookupLocFromDb()));
    QHBoxLayout *H_ldb = new QHBoxLayout();
    H_ldb->setContentsMargins ( 1, 1, 1, 1);
    H_ldb->setSpacing(4);
    H_ldb->addWidget(le_dx_loc);
    H_ldb->addWidget(l_loc_fr_db);
    H_ldb->addWidget(l_loc_from_db);
    H_ldb->addWidget(b_lookup_loc);
    H_ldb->setAlignment(Qt::AlignRight);

    //le_dx_loc->setMaximumWidth(220);

    //Propagation mode:
    QLabel *l_prop = new QLabel();
    l_prop->setText(tr("Propagation Mode")+":");
    //l_prop->setFixedWidth(110);
    Cb_prop = new QComboBox();
    Cb_prop->setMinimumWidth(140);
    QStringList lst_m;
    lst_m<<"Non"<<"Unknown"<<"Aircraft Scatter"<<"Aurora"<<"Aurora-E"<<"Back Scatter"<<"FAI"<<"F2"<<"Iono Scatter"
    <<"Meteor Scatter"<<"Moon Bounce"<<"Rain Scatter"<<"Satellite"<<"Sporadic-E"<<"Transequatorial"<<"Tropo";
    id_prop<<""<<""<<"ACS"<<"AUR"<<"AUE"<<"BS"<<"FAI"<<"F2"<<"ION"<<"MS"<<"EME"<<"RS"<<"SAT"<<"ES"<<"TEP"<<"TR";
    /*for (int i = 0; i < COUNT_PROP; ++i)
    {
    	if (i==1)
    	{
    		lst_m<<"Unknown";
    		id_prop<<"";    
    	}
    	lst_m<<s_prop_mod[i];
    	id_prop<<s_id_prop_mod[i];   	
    }*/

    Cb_prop->addItems(lst_m);
    Cb_prop->setCurrentIndex(0);
    //Cb_prop->setFixedWidth(140);
    QHBoxLayout *H_prop = new QHBoxLayout();
    H_prop->setContentsMargins ( 1, 1, 1, 1);
    H_prop->setSpacing(3);
    H_prop->addWidget(l_prop);
    //H_prop->setAlignment(l_prop,Qt::AlignLeft);
    H_prop->addWidget(Cb_prop);
    //H_prop->setAlignment(Cb_prop,Qt::AlignLeft);
    //H_prop->setAlignment(Qt::AlignLeft);

    //Remarks:
    QLabel *l_rem = new QLabel();
    l_rem->setText(tr("Remarks")+":");
    //l_rem->setFixedWidth(60);
    le_remarks = new QLineEdit();
    le_remarks->setMaxLength(30);
    connect(le_remarks, SIGNAL(returnPressed()), this, SLOT(RemarksEntered()));

    QHBoxLayout *H_rem = new QHBoxLayout();
    H_rem->setContentsMargins ( 1, 1, 1, 1);
    H_rem->setSpacing(3);
    H_rem->addWidget(l_rem);
    //H_prop->setAlignment(l_prop,Qt::AlignLeft);
    H_rem->addWidget(le_remarks);

    l_tcpcon_info = new QLabel(tr("Status")+": <font color='red'>"+tr("Disconnected")+"</font>");
    //l_tcpcon_info->setFont(f_t);
    l_tcpcon_info->setContentsMargins(5,5,5,5);
    b_recon_host = new QPushButton(tr("Try To Connect"));
    b_recon_host->setFixedWidth(135);
    QHBoxLayout *H_host_info= new QHBoxLayout();
    H_host_info->setContentsMargins ( 0, 0, 0, 0);
    H_host_info->setSpacing(10);
    H_host_info->addWidget(l_tcpcon_info);
    H_host_info->addWidget(b_recon_host);
    H_host_info->setAlignment(Qt::AlignCenter);

    b_send_spot = new QPushButton(tr("Send Spot"));
    b_send_spot->setFixedWidth(95);
    b_cancel_spot = new QPushButton(tr("Cancel"));
    b_cancel_spot->setFixedWidth(95);
    QHBoxLayout *H_spot_bt= new QHBoxLayout();
    H_spot_bt->setContentsMargins ( 0, 0, 0, 0);
    H_spot_bt->setSpacing(10);
    H_spot_bt->addWidget(b_send_spot);
    H_spot_bt->addWidget(b_cancel_spot);
    H_spot_bt->setAlignment(Qt::AlignCenter);

    b_send_spot->setEnabled(false);//default
    //b_recon_host->setHidden(false);//141 ne puskai vizda se pri zarezdaneto default

    connect(b_send_spot, SIGNAL(released()), this, SLOT(SendSpot()));
    connect(b_cancel_spot, SIGNAL(released()), this, SLOT(CancelSpot()));

    connect(le_dx_loc, SIGNAL(EmitSndCheck(QString)), this, SLOT(InfoChanged(QString)));
    //connect(le_my_loc, SIGNAL(SndCheck(QString)), this, SLOT(InfoChanged(QString)));

    connect(Cb_prop, SIGNAL(currentIndexChanged(QString)), this, SLOT(InfoChanged(QString)));
    connect(Cb_prop, SIGNAL(currentIndexChanged(QString)), this, SLOT(PropChanged(QString)));//for set fokus

    connect(b_recon_host, SIGNAL(released()), this, SIGNAL(EmitReconnect()));

    QVBoxLayout *V_l = new QVBoxLayout(this);
    V_l->setContentsMargins ( 5, 5, 5, 5);
    V_l->setSpacing(5);
    V_l->addWidget(le_my_call);
    V_l->addWidget(le_my_loc);
    V_l->addWidget(le_freq);
    V_l->addWidget(le_dx_call);
    V_l->addLayout(H_ldb);
    //V_l->addWidget(pb_lookup_loc);
    //V_l->addWidget(le_dx_loc);
    V_l->addLayout(H_prop);
    V_l->addLayout(H_rem);
    V_l->addLayout(H_host_info);
    //V_l->addWidget(l_tcpcon_info);
    //V_l->setAlignment(l_tcpcon_info,Qt::AlignCenter);
    V_l->addLayout(H_spot_bt);
    //this->setMinimumWidth(340);

    Check("call");
    Check("locator");
    block_loc = true;
    s_f_offset = 0;
    //qDebug()<<"NetworkMessage::pulse * 1000";
}
SpotDialog::~SpotDialog()
{}
void SpotDialog::SetFont(QFont f)
{
    le_my_call->SetFont(f);
    le_my_loc->SetFont(f);
    le_freq->SetFont(f);
    le_dx_call->SetFont(f);
    le_dx_loc->SetFont(f);
    l_tcpcon_info->setFont(f);
}
void SpotDialog::FreqEntered()
{
    le_dx_call->SetFocus();
}
void SpotDialog::DxCallEntered()
{
    le_dx_loc->SetFocus();
}
void SpotDialog::RemarksEntered()
{
    if (b_send_spot->isEnabled())
        SendSpot();
}

void SpotDialog::Check(QString s_type)
{
    if (s_type == "call")
    {
        //if (le_dx_call->getText().count()>1)
        if (THvQthLoc.isValidCallsign(le_dx_call->getText()))
        {
            le_dx_call->setError(false);
            le_dx_call->setErrorColorLe(false);
        }
        else
        {
            le_dx_call->setError(true);
            le_dx_call->setErrorColorLe(true);
        }
        emit FindLocFromDB(le_dx_call->getText());
        CheckForWalidSpot("");
    }

    if (s_type == "locator")
    {
        if (!le_dx_loc->getText().isEmpty() && THvQthLoc.isValidLocator(le_dx_loc->getText()))
        {
            le_dx_loc->setError(false);
            le_dx_loc->setErrorColorLe(false);
            block_loc = false;
        }
        else
        {
            le_dx_loc->setError(true);
            le_dx_loc->setErrorColorLe(true);
            block_loc = true;
        }
    }

    if (s_type == "freq")
    {
        //qDebug()<<le_freq->getText().toLong()<<le_freq->getText().toLongLong();
        if (le_freq->getText().toLongLong()<100)
        {
            le_freq->setError(true);
            le_freq->setErrorColorLe(true);
        }
        else
        {
            le_freq->setError(false);
            le_freq->setErrorColorLe(false);
        }
    }
}
void SpotDialog::SetDistUnit(bool f_)
{
    f_km_mi = f_;
}
QString SpotDialog::CalcDistance(QString myl, QString hisl)
{
    QString s_dist;
    QString c_test_loc = THvQthLoc.CorrectLocator(hisl);
    QString c_my_loc = THvQthLoc.CorrectLocator(myl);

    double dlong1 = THvQthLoc.getLon(c_my_loc);
    double dlat1  = THvQthLoc.getLat(c_my_loc);
    double dlong2 = THvQthLoc.getLon(c_test_loc);
    double dlat2 = THvQthLoc.getLat(c_test_loc);

    int dist_km = THvQthLoc.getDistanceKilometres(dlong1,dlat1,dlong2,dlat2);

    if (!f_km_mi)
        s_dist = QString("%1").arg(dist_km)+" km";
    else
        s_dist = QString("%1").arg(THvQthLoc.getDistanceMilles(dlong1,dlat1,dlong2,dlat2))+" mi";

    return s_dist;
}
void SpotDialog::LookupLocFromDb()
{
    //qDebug()<<"ENTER";
    if (l_loc_from_db->text()!="NA")
        le_dx_loc->SetText(l_loc_from_db->text());
    le_remarks->setFocus();
}
void SpotDialog::SetLocFromDB(QString loc)
{
    if (!loc.isEmpty())
        l_loc_from_db->setText(loc);
    else
        l_loc_from_db->setText("NA");
}
void SpotDialog::PropChanged(QString)
{
    le_remarks->setFocus();
}
void SpotDialog::InfoChanged(QString)
{
    QString dist = "";
    if (s_mode=="PI4" && !le_my_loc->getText().isEmpty() && !le_dx_loc->getText().isEmpty() && !block_loc)
        dist = " "+CalcDistance(le_my_loc->getText(),le_dx_loc->getText());

    QString frreq_offset = "";
    if (s_f_offset>0)
        frreq_offset = " "+QString("%1").arg(s_f_offset)+" Hz";

    int id = Cb_prop->currentIndex();
    if (id!=0)
    {
        QString prop = id_prop[id];
        if (!block_loc)
        {
            if (s_mode=="PI4")
                le_remarks->setText(le_my_loc->getText().mid(0,4)+"<"+prop+">"+le_dx_loc->getText().mid(0,4)+" "+s_end_info+dist);
            else
            {
                if (s_f_offset>0)
                    le_remarks->setText("<"+prop+"> "+s_end_info+dist+frreq_offset);
                else
                    le_remarks->setText(le_my_loc->getText()+"<"+prop+">"+le_dx_loc->getText()+" "+s_end_info+dist);
            }
        }
        else
        {
            if (s_f_offset>0)
                le_remarks->setText("<"+prop+"> "+s_end_info+dist+frreq_offset);
            else
                le_remarks->setText(s_end_info+dist);
        }
    }
    else
        le_remarks->setText(s_end_info+dist+frreq_offset);
}
void SpotDialog::SetTcpconInfo(QString str)
{
    l_tcpcon_info->setText(str);
    if (str.contains(tr("Connected to")))
    {
        CheckForWalidSpot("");
        b_recon_host->setHidden(true);
    }
    else
    {
        CheckForWalidSpot("");
        b_recon_host->setHidden(false);
    }
}
void SpotDialog::CheckForWalidSpot(QString)
{
    if (l_tcpcon_info->text().contains(tr("Connected to"))
            && le_dx_call->getText().count()>2 && le_my_call->getText().count()>2
            && le_freq->getText().toLongLong()>99) //here in khz min is 100khz  le_freq->getText().count()>2
        b_send_spot->setEnabled(true);
    else
        b_send_spot->setEnabled(false);
}
void SpotDialog::setAllEdit(QStringList list, int id,int f_offset)//id 0=psk 1=dx_spot  10=empty
{
    //http://www.dxsummit.fi/SendSpot.aspx?callSign=LZ2HV&dxCallSign=OZ2M&frequency=70220&info=JO65FR<ES>KN23SF Test Spot
    send_spot_result = false;
    //list<<s_myCall<<s_myLoc<<hisCall<<hisLoc<<sfrq<<mode<<QString("%1").arg(snr);
    le_my_call->SetText(list[0]);
    le_my_loc->SetText(list[1]);
    le_dx_call->SetText(list[2]);
    le_dx_loc->SetText(list[3]);
    le_freq->SetText(list[4]);
    s_mode = list[5];
    //int id = Cb_prop->currentIndex();

    if (id==10)//id 0=psk 1=dx_spot  10=empty
        s_end_info = "";
    else
        s_end_info =s_mode+" "+list[6];

    s_f_offset = f_offset;

    InfoChanged("");
    /*if (id!=0)
    {
        QString prop = id_prop[id];
        //le_remarks->setText(list[3]+"<"+prop+">"+list[1]+" "+s_end_info);
        le_remarks->setText(list[1]+"<"+prop+">"+list[3]+" "+s_end_info);
    }
    else
        le_remarks->setText(s_end_info);*/

    le_freq->SetFocus();
}
QStringList SpotDialog::getAllEdit()
{
    QStringList list;
    //list<<""<<""<<""<<""<<""<<""<<""<<""<<""<<""<<"";
    for (int i = 0; i<12; ++i) list.append("");
    list[0]= le_my_call->getText();
    list[1]= le_my_loc->getText();
    list[2]= le_dx_call->getText();
    list[3]= le_dx_loc->getText();
    list[4]= le_freq->getText();
    //int id = Cb_prop->currentIndex();
    //list[5] = id_prop[id];
    list[5] = le_remarks->text();
    return list;
}
void SpotDialog::SendSpot()
{
    send_spot_result = true;
    close();
}
void SpotDialog::CancelSpot()
{
    send_spot_result = false;
    close();
}

#define _BANDS_H_
#define _BCNBAND_H_
#define _MODESTRFORFREQ_H_
#define _ALLBANDSMODSFRQ_H_
#include "../../config_band_all.h"
EditRadInfo::EditRadInfo(QWidget * parent )
        : QDialog(parent)
{
    //setMinimumSize(300,100);
    setWindowTitle(tr("Edit Radio And Frequencies"));
    setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);

    QLabel *l_lband = new QLabel(tr("Band")+": ");
    l_band = new QLabel("0 MHz");
    QHBoxLayout *hba = new QHBoxLayout();
    hba->setContentsMargins(0,0,0,0);
    hba->setSpacing(5);
    hba->addWidget(l_lband);
    hba->addWidget(l_band);
    hba->setAlignment(Qt::AlignCenter);

    QLabel *l_ant = new QLabel(tr("Antenna")+": ");
    ant = new QLineEdit();
    QHBoxLayout *v_ha = new QHBoxLayout();
    v_ha->setContentsMargins(0,0,0,0);
    v_ha->setSpacing(5);
    v_ha->addWidget(l_ant);
    v_ha->addWidget(ant);

    VB_Freq = new QVBoxLayout();
    for (int i = 0; i<COUNT_FREQ_MODES; i++)
    {
        if (i==0)//"MSK144","MSKMS"
        {
            HvInLeFreq *le_frq  = new HvInLeFreq("MSK144,MSKMS "+tr("Frequency In")+" Hz:");
            VB_Freq->addWidget(le_frq);
        }
        else if (i==1)//"JTMS","FSK441","FSK315","ISCAT-A","ISCAT-B","JT6M"
        {
            HvInLeFreq *le_frq  = new HvInLeFreq("JTMS,FSK,ISCAT,JT6M "+tr("Frequency In")+" Hz:");
            VB_Freq->addWidget(le_frq);
        }
        else
        {
            HvInLeFreq *le_frq  = new HvInLeFreq(ModeStrForFerq[pos_mod_rea_frq[i]]+" "+tr("Frequency In")+" Hz:");
            VB_Freq->addWidget(le_frq);
        }
    }

    pb_applay_info = new QPushButton(tr("Apply Changes"));//Apply
    pb_cancel_info = new QPushButton(tr("Cancel")) ;
    QHBoxLayout *h_b = new QHBoxLayout();
    h_b->setContentsMargins(0, 0, 0, 0);
    h_b->setSpacing(5);
    h_b->addWidget(pb_applay_info);
    h_b->addWidget(pb_cancel_info);
    connect(pb_cancel_info, SIGNAL(clicked(bool)), this, SLOT(close()));
    connect(pb_applay_info, SIGNAL(clicked(bool)), this, SLOT(ApplayInfo()));

    QVBoxLayout *v_b = new QVBoxLayout(this);
    v_b->setContentsMargins(5, 5, 5, 5);
    v_b->setSpacing(5);
    v_b->addLayout(hba);
    v_b->addLayout(v_ha);
    v_b->addLayout(VB_Freq);
    //v_b->addWidget(frq);

    v_b->addLayout(h_b);

    s_edit_index = -1;

    setLayout(v_b);
    setMinimumSize(350,215);
}
EditRadInfo::~EditRadInfo()
{}
void EditRadInfo::ApplayInfo()
{
    QStringList list;
    QString freqs;
    for (int i = 0; i<COUNT_FREQ_MODES; i++)
    {
        HvInLeFreq *le_frq = (HvInLeFreq*)VB_Freq->itemAt(i)->widget();
        freqs.append(le_frq->Text());
        if (i<COUNT_FREQ_MODES-1) freqs.append("\n");
    }
    list <<l_band->text() << ant->text() << freqs;
    emit EmitSetInfo(list ,s_edit_index);
}
void EditRadInfo::SetEditStInfo(QStringList list, int index)
{
    ant->setText(list.at(1));

    QString tfreq = list.at(2);
    QStringList ltfreq = tfreq.split("\n");
    for (int i = 0; i<COUNT_FREQ_MODES; i++)
    {
        HvInLeFreq *le_frq = (HvInLeFreq*)VB_Freq->itemAt(i)->widget();
        le_frq->SetText(ltfreq.at(i));
    }
    l_band->setText(list.at(0));
    s_edit_index = index;
    exec();
}

//#include <QTextStream>
//#undef _LAMBDA_H_
//#undef _BCNBAND_H_
//#undef _BANDTOFREQ_H_
/*
#define _BANDS_H_
#define _BCNBAND_H_
#define _MODESTRFORFREQ_H_
#define _ALLBANDSMODSFRQ_H_
#include "../../config_band_all.h"
*/
#include <QHeaderView>
HvRadList::HvRadList(bool f,QWidget *parent)
        : QTreeView(parent)
{
    dsty = f;
    setMinimumSize(100,522);//2.68 522
    setRootIsDecorated(false);
    setModel(&model);
    //HvHeaderStInfo *THvHeader = new HvHeaderStInfo(Qt::Horizontal);
    QHeaderView *THvHeader = new QHeaderView(Qt::Horizontal);
    setHeader(THvHeader);
    setAllColumnsShowFocus(true);  // za da o4ertae delia row
    //setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setDragDropMode(QAbstractItemView::NoDragDrop);//QAbstractItemView::InternalMove
    setDragEnabled (false);
    // setAcceptDrops(true);
    setVerticalScrollMode(QAbstractItemView::ScrollPerItem);// tova e za na mi6kata whella i dragdrop QAbstractItemView::ScrollPerPixel
    setAutoScroll(false); // s tova tarsi samo v 1 colona i za scroll pri drag
    /*   m_file = new QMenu(this);
       QAction *file_info = new QAction("File Info", this);
       m_file->addAction(file_info);
       connect(file_info, SIGNAL(triggered()), this, SLOT(File_Info()));
       m_file->addSeparator();
       QAction *clr_sel = new QAction("Clear Selected", this);
       m_file->addAction(clr_sel);
       connect(clr_sel, SIGNAL(triggered()), this, SLOT(DellBtSelItems()));
       TFileEditInfoHV = new FileEditInfoHV(this);
       connect(TFileEditInfoHV, SIGNAL(TagChange(QString)), this, SLOT(TagChange_lis(QString)));*/
    QPalette PaletteL = palette();
    PaletteL.setColor(QPalette::Shadow, QColor(120,120,120));// zaradi bordera okantovaneto na lista
    setPalette(PaletteL);

    QStringList list_A;
    list_A <<tr("Band")<<tr("Antenna Description")<<tr("Mode")<<tr("Frequency In")+" Hz";
    model.setHorizontalHeaderLabels(list_A);
    THvHeader->setSectionResizeMode(QHeaderView::Fixed);//qt5
    //THvHeader->setResizeMode(2, QHeaderView::Fixed);
    THvHeader->resizeSection(0, 100);//80
    THvHeader->resizeSection(1, 150);//150
    THvHeader->resizeSection(2, 70);//70
    THvHeader->resizeSection(3, 155);//120
    //bavi THvHeader->setSectionResizeMode(0, QHeaderView::Stretch);//1.81 bavi for font
    THvHeader->setSectionResizeMode(1, QHeaderView::Stretch);//qt5
    //bavi THvHeader->setSectionResizeMode(3, QHeaderView::Stretch);//1.81 bavi for font
    setUniformRowHeights(true);//2.30 fast view
    f_row_color = false;
}
HvRadList::~HvRadList()
{}
void HvRadList::paintEvent(QPaintEvent *event)
{
    QPainter painter(viewport());
    //QModelIndex index = model.index(ActiveIndex, 1, QModelIndex());
    //painter.fillRect(QRect(0,visualRect(index).y(),width(),visualRect(index).height()), ActiveRowBackg);
    QTreeView::paintEvent(event);


    //QTreeView::paintEvent(event);
    //QPainter painter(viewport());

    painter.save();
    QColor cc = QColor(200,200,200);
    if (dsty) cc = QColor(80,80,80);
    painter.setPen(QPen(cc, 1, Qt::DashLine));

    //qDebug()<<model.columnCount();
    for (int i = 1; i<model.columnCount(); ++i)
        painter.drawLine(header()->sectionPosition(i),1,header()->sectionPosition(i),height());

    painter.restore();
}
void HvRadList::InsertItem_hv(QStringList list)
{
    if (!list.isEmpty())
    {
        QList<QStandardItem *> qlsi;
        int k = 0;
        for (QStringList::iterator it =  list.begin(); it != list.end(); ++it)
        {
            QStandardItem *item = new QStandardItem(QString(*it));

            if (k==0)
                item->setTextAlignment(Qt::AlignCenter);
            item->setEditable(false);

            if (!f_row_color)
            {
                QColor cc = QColor(255,255,255);
                if (dsty) cc = QColor(25,25,25);
                item->setBackground(cc);
            }

            else
            {
                if (dsty) item->setBackground(QColor(45,45,45));
                else item->setBackground(QColor(212,222,242)); //rgb
            }
            qlsi.append(item);

            if (k==1)//sluzebno
            {
                k++;
                QStandardItem *item_mod;
                QString mod;
                for (int i  = 0; i < COUNT_FREQ_MODES; i++)
                {
                    mod.append(ModeStrForFerq[pos_mod_rea_frq[i]]);
                    if (i<COUNT_FREQ_MODES-1) mod.append("\n");
                }
                item_mod = new QStandardItem(mod);
                if (!f_row_color)
                    item_mod->setBackground(QColor(255,255,255));
                else
                    item_mod->setBackground(QColor(212,222,242));
                qlsi.append(item_mod);
            }

            k++;
        }
        model.insertRow(model.rowCount(), qlsi);

        if (!f_row_color) f_row_color = true;
        else f_row_color = false;
        //this->setCurrentIndex(model.index(model.rowCount()-1,0));
        //this->scrollTo(model.index(model.rowCount()-1,0));
    }
    //this->setCurrentIndex(model.index(model.rowCount(), 1, QModelIndex()));
}
void HvRadList::SetItem_hv(QStringList list,int index_edit)//2.57 remove  new QStandardItem
{
    int column_numb = 0;
    for (QStringList::iterator it =  list.begin(); it != list.end(); ++it)
    {
        //QStandardItem *item = new QStandardItem(QString(*it));
        QStandardItem *item = model.itemFromIndex(model.index(index_edit, column_numb));
        item->setText(QString(*it));

        if (column_numb==0)
            item->setTextAlignment(Qt::AlignCenter);
        item->setEditable(false);

        QBrush bcolor = model.item(index_edit,column_numb)->background();
        item->setBackground(bcolor);
        model.setItem(index_edit,column_numb,item);

        if (column_numb==1)//sluzebno
        {
            column_numb++;
            QStandardItem *item_mod = model.itemFromIndex(model.index(index_edit, column_numb));
            QString mod;
            for (int i  = 0; i < COUNT_FREQ_MODES; i++)
            {
                mod.append(ModeStrForFerq[pos_mod_rea_frq[i]]);
                if (i<COUNT_FREQ_MODES-1) mod.append("\n");
            }
            //item_mod = new QStandardItem(mod);
            item_mod->setText(mod);
            item_mod->setBackground(bcolor);
            model.setItem(index_edit,column_numb,item_mod);
        }
        column_numb++;
    }
    //this->setCurrentIndex(model.index(index_edit, 1, QModelIndex()));// ina4e ne refreshva 1 row
}
void HvRadList::Clear_List()
{
    for (int i = model.rowCount()-1; i >= 0; --i) // ok pravilno hv
        model.removeRow(i);
}
void HvRadList::mouseDoubleClickEvent(QMouseEvent * event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (selectionModel()->selection().empty())
        {
            return;
        }
        QModelIndex index = selectionModel()->currentIndex();
        int index_n = index.row();
        QStringList list;

        // no crash ->model.item(index.row(),1)->text();//2.48
        list << index.sibling(index.row(),0).data().toString(); //band
        list << index.sibling(index.row(),1).data().toString(); //ant
        list << index.sibling(index.row(),3).data().toString(); //frq
        /*list << model.item(index.row(),0)->text(); //band
        list << model.item(index.row(),1)->text(); //ant
        list << model.item(index.row(),3)->text(); //frq*/

        emit EmitDoubleClick(list,index_n);
    }
    QTreeView::mousePressEvent(event);
}

#include "../../config_str_all.h"
#define _CONT_NAME_
#include "../../config_str_con.h"
QString _kkwe2_;
#define BASE32_CHARSET "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567"
RadioAndNetW::RadioAndNetW(QString inst,QString path,bool indsty,int x,int y,QWidget *parent )
        : QWidget(parent)
{
    w_parent = parent;
    f_mods_accept_cmd = false;
    s_mode = "FSK441";
    s_smode = "";
    s_rigInfo = "No CAT control";
    //prev_pos_dec = 0;
    id_activ_upd = 0;//0=no 1=decoded 2=replay
    pos_dec = 0;
    pos_upd = -1;
    s_dx_call = "";
    s_report = "";
    s_dx_grid = "";
    s_auto = false;
    s_tx = false;
    s_tx_msg = "";
    s_cont_id = 0;

    FREQ_GLOBAL = "70230000";//2.74
    //this->setMinimumSize(100,100);
    s_mode_str_for_ferq = "FSK";
    //this->setMinimumWidth(480);
    setWindowTitle(tr("Network Configuration"));//2.68
    //setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);

    QVBoxLayout *V_l = new QVBoxLayout();
    V_l->setContentsMargins(4,4,4,4);//(5,4,5,4);
    V_l->setSpacing(2);

    //QGroupBox *GB_nets = new QGroupBox("Network Services:");   //Network Services:
    //GB_nets->setLayout(h_nets);

    QGroupBox *GB_psk_rep_settings = new QGroupBox(tr("PSK Reporter Settings")+":");
    QHBoxLayout *h_nets = new QHBoxLayout();
    h_nets->setContentsMargins(5,0,0,0);
    h_nets->setSpacing(5);
    cb_start_stop_psk_rpt = new QCheckBox(tr("Enable PSK Reporter Spotting"));
    cb_start_stop_psk_rpt->setChecked(false);
    h_nets->addWidget(cb_start_stop_psk_rpt);

    cb_udp_tcp_psk_rpt = new QCheckBox(tr("Use TCP/UDP Protocol"));
    cb_udp_tcp_psk_rpt->setChecked(false);
    cb_udp_tcp_psk_rpt->setEnabled(false);
    h_nets->addWidget(cb_udp_tcp_psk_rpt);

    //QLabel *l_DefHostInfo = new QLabel("Default  Server: report.pskreporter.info  Port: 4739");
    //l_DefHostInfo->setContentsMargins(5,5,5,0);
    //l_DefHostInfo->setAlignment(Qt::AlignCenter);

    QLabel *l_udps = new QLabel(tr("Server")+":");//UDP
    UDPServer = new QLineEdit();
    UDPServer->setText("127.0.0.1");
    pb_re_conect = new QPushButton(tr("Reconnect"));
    //pb_re_conect->setFixedHeight(20);
    //   pb_re_conect->setFixedSize(90,20);
    pb_re_conect->setFixedHeight(20);
    pb_re_conect->setEnabled(false);

    QHBoxLayout *h_net = new QHBoxLayout();
    h_net->setContentsMargins(5, 5, 5, 0);
    h_net->setSpacing(5);
    h_net->addWidget(l_udps);
    h_net->addWidget(UDPServer);

    QLabel *l_udpp = new QLabel(tr("Port")+":");//UDP
    UDPPort = new QLineEdit();
    UDPPort->setText("2237");
    UDPPort->setInputMask("9999999999");
    /*QHBoxLayout *h_p = new QHBoxLayout();
    h_p->setContentsMargins ( 5, 5, 5, 5);
    h_p->setSpacing(5);
    h_p->addWidget(l_udpp);
    h_p->addWidget(UDPPort);*/
    h_net->addWidget(l_udpp);
    h_net->addWidget(UDPPort);
    h_net->addWidget(pb_re_conect);

    //v1.77 stop stop changing
    UDPPort->setText("4739");
    UDPServer->setText("report.pskreporter.info");
    UDPServer->setEnabled(false);
    UDPPort->setEnabled(false);
    //v1.77 stop stop changing

    QFont f_t = font();
    f_t.setPointSize(10);
    //l_con_info = new QLabel("Status: <font color='red'>UDP server lookup failed: Host not found</font>");
    l_con_info = new QLabel(tr("Status")+": <font color='red'>"+tr("PSK Reporter Is Disabled And Disconnected")+"</font>");
    l_con_info->setFont(f_t);
    l_con_info->setContentsMargins(5,0,5,0);

    QVBoxLayout *V_net = new QVBoxLayout();
    V_net->setContentsMargins(5,2,5,2);
    V_net->setSpacing(0);
    V_net->addLayout(h_nets);
    //V_net->addWidget(l_DefHostInfo);
    V_net->addWidget(l_con_info);
    V_net->addLayout(h_net);
    //V_net->addLayout(h_p);
    GB_psk_rep_settings->setLayout(V_net);
    V_l->addWidget(GB_psk_rep_settings);

    //V_l->addWidget(GB_nets);

    ////////////TCP//////////////////////////////////////////////////////////////////
    QGroupBox *GB_spot_settings = new QGroupBox(tr("DX-Spot Settings")+":");   //Network Services:
    l_tcpcon_info = new QLabel(tr("Status")+": <font color='red'>"+tr("Disconnected")+"</font>");
    l_tcpcon_info->setFont(f_t);
    l_tcpcon_info->setContentsMargins(5,0,5,0);

    QLabel *l_spot_host = new QLabel(tr("Server")+":");//HTTP
    TCPServer = new QLineEdit();
    TCPServer->setText("db0sue.de");//9a0dxc.hamradio.hr
    pb_tcp_conect = new QPushButton(tr("Press To Connect"));
    tcp_conect_stat = false;
    global_tcp_stat = false;
    //pb_tcp_conect->setFixedSize(130,20);
    pb_tcp_conect->setFixedHeight(20);
    QHBoxLayout *h_tp = new QHBoxLayout();
    h_tp->setContentsMargins(5, 5, 5, 5);
    h_tp->setSpacing(5);
    h_tp->addWidget(l_spot_host);
    h_tp->addWidget(TCPServer);

    QLabel *l_httpp = new QLabel(tr("Port")+":");//HTTPport
    TCPPort= new QLineEdit();
    TCPPort->setText("8000");
    TCPPort->setInputMask("9999999999");      
    QLabel *l_tcpp = new QLabel(tr("Password")+":");
    TCPPass = new QLineEdit();
    TCPPass->setEchoMode(QLineEdit::Password);
    
    /*QHBoxLayout *h_hp = new QHBoxLayout();
    h_hp->setContentsMargins ( 5, 5, 5, 5);
    h_hp->setSpacing(5);
    h_hp->addWidget(l_httpp);
    h_hp->addWidget(TCPPort);*/
    h_tp->addWidget(l_httpp);
    h_tp->addWidget(TCPPort);
    h_tp->addWidget(pb_tcp_conect);

    QVBoxLayout *V_spot = new QVBoxLayout();
    V_spot->setContentsMargins(5,2,5,2);
    V_spot->setSpacing(0);
    V_spot->addWidget(l_tcpcon_info);
    V_spot->addLayout(h_tp);
    //V_spot->addLayout(h_hp);

    QLabel *l_telnet = new QLabel();
    l_telnet->setText(tr("Telnet Clusters")+":");
    //l_telnet->setFixedWidth(100);
    Cb_telnet = new QComboBox();
    //Cb_telnet->setMinimumWidth(140);
    ReadTelnetList(path+"/settings/database/mstn_db.dbtn");
    Cb_telnet->setCurrentIndex(0);
    //Cb_telnet->setFixedWidth(140);
    QHBoxLayout *H_telnet = new QHBoxLayout();
    H_telnet->setContentsMargins(5, 1, 5, 1);
    H_telnet->setSpacing(3);
    H_telnet->addWidget(l_telnet);
    H_telnet->addWidget(Cb_telnet); //H_telnet->setAlignment(Cb_telnet,Qt::AlignLeft);
    H_telnet->addWidget(l_tcpp);//2.76.3
    H_telnet->addWidget(TCPPass);    
    V_spot->addLayout(H_telnet);

    GB_spot_settings->setLayout(V_spot);
    V_l->addWidget(GB_spot_settings);
    connect(pb_tcp_conect, SIGNAL(clicked(bool)), this, SLOT(ConDiscon()));
    connect(TCPServer, SIGNAL(textChanged(QString)), this, SLOT(TCPServPortChanged(QString)));
    connect(TCPPort, SIGNAL(textChanged(QString)), this, SLOT(TCPServPortChanged(QString)));
    connect(Cb_telnet, SIGNAL(currentIndexChanged(QString)), this, SLOT(CbTelnetChanged(QString)));
    /////////////END TCP///////////////////////////////////////////
    QGroupBox *GB_udp_broad_settings = new QGroupBox(tr("UDP Broadcast Settings")+":");   //Network Services:
    QVBoxLayout *V_udp = new QVBoxLayout();
    V_udp->setContentsMargins(5,2,5,2);//(int left, int top, int right, int bottom)
    V_udp->setSpacing(0);

    l_udp_broad_info = new QLabel(tr("Status")+": <font color='red'>"+tr("UDP Broadcast Is Disabled And Disconnected")+"</font>");
    l_udp_broad_info->setFont(f_t);
    l_udp_broad_info->setContentsMargins(5,0,5,0);

    cb_udp_broad_log_qso = new QCheckBox(tr("Enable Logged QSO"));//Enable  (Logger32)
    cb_udp_broad_log_qso->setChecked(false);
    cb_udp_broad_log_qso->setToolTip("Logger32, etc.");
    cb_udp_broad_log_adif = new QCheckBox(tr("Enable Logged QSO ADIF"));
    cb_udp_broad_log_adif->setChecked(false);
    cb_udp_broad_log_adif->setToolTip("N1MM, etc.");
    cb_udp_broad_decod = new QCheckBox(tr("Enable Decoded Text"));
    cb_udp_broad_decod->setChecked(false);
    cb_udp_broad_decod->setToolTip("DX Aggregator, N1MM, Logger32, etc.");

    QHBoxLayout *h_qsoadif = new QHBoxLayout();
    h_qsoadif->setContentsMargins(5, 0, 0, 0);
    h_qsoadif->setSpacing(0);
    h_qsoadif->addWidget(cb_udp_broad_log_qso);
    h_qsoadif->addWidget(cb_udp_broad_log_adif);
    h_qsoadif->addWidget(cb_udp_broad_decod);

    QLabel *l_udp_broad_host = new QLabel(tr("Server")+":");
    UDPServerBroad = new QLineEdit();
    UDPServerBroad->setText("127.0.0.1");
    QLabel *l_udp_p = new QLabel(tr("Port")+":");//HTTPport
    UDPPortBroad= new QLineEdit();
    UDPPortBroad->setText("2237");
    UDPPortBroad->setInputMask("9999999999");

    pb_re_conect_udp_broad = new QPushButton(tr("Reconnect"));
    pb_re_conect_udp_broad->setFixedHeight(20);
    pb_re_conect_udp_broad->setEnabled(false);

    QHBoxLayout *h_udp = new QHBoxLayout();
    h_udp->setContentsMargins(5,5,5,0);
    h_udp->setSpacing(5);
    h_udp->addWidget(l_udp_broad_host);
    h_udp->addWidget(UDPServerBroad);
    h_udp->addWidget(l_udp_p);
    h_udp->addWidget(UDPPortBroad);
    h_udp->addWidget(pb_re_conect_udp_broad);

    AppPath = path;
    cb_wr_status = new QCheckBox(tr("Write Status Info In To File")+"    (settings/mshv_status.txt)");
    QHBoxLayout *h_udpw = new QHBoxLayout();
    h_udpw->setContentsMargins(5,1,0,0);
    h_udpw->setSpacing(5);
    h_udpw->addWidget(cb_wr_status);

    //////////2ndUDP////////////////////////////////////////////
    QHBoxLayout *h_udp2a = new QHBoxLayout();
    h_udp2a->setContentsMargins(0,5,0,0);
    h_udp2a->setSpacing(4);
    QFrame *lineH = new QFrame();
    lineH->setFrameShape(QFrame::HLine);
    lineH->setFrameShadow(QFrame::Sunken);
    //lineH->setContentsMargins(0,0,0,0);
    h_udp2a->addWidget(lineH);
    QLabel *l_udp2_srv = new QLabel(tr("Server")+":");
    udp2_Server = new QLineEdit();
    udp2_Server->setText("127.0.0.1");
    QLabel *l_udp2_p = new QLabel(tr("Port")+":");
    udp2_Port= new QLineEdit();
    udp2_Port->setText("2233");
    udp2_Port->setInputMask("9999999999");
    cb_udp2adif = new QCheckBox(tr("Enable Logged QSO ADIF"));
    cb_udp2adif->setChecked(false);
    cb_udp2adif->setToolTip("N1MM, etc.");
    QLabel *l_udp2 = new QLabel(tr("Simplified UDP Broadcast")+":");
    QHBoxLayout *h_udp2b = new QHBoxLayout();
    h_udp2b->setContentsMargins(5,0,0,0);
    h_udp2b->setSpacing(5);
    h_udp2b->addWidget(l_udp2);
    h_udp2b->addWidget(cb_udp2adif);
    QHBoxLayout *h_udp2c = new QHBoxLayout();
    h_udp2c->setContentsMargins(5,5,5,0);
    h_udp2c->setSpacing(5);
    h_udp2c->addWidget(l_udp2_srv);
    h_udp2c->addWidget(udp2_Server);
    h_udp2c->addWidget(l_udp2_p);
    h_udp2c->addWidget(udp2_Port);
    socet_udp2_broad = new QUdpSocket(this);
    connect(cb_udp2adif, SIGNAL(toggled(bool)), this, SLOT(StartStopUdp2Broad(bool)));
    ///////////END 2ndUDP/////////////////////////////////////////////

    V_udp->addLayout(h_qsoadif);
    V_udp->addWidget(l_udp_broad_info);
    V_udp->addLayout(h_udp);
    V_udp->addLayout(h_udpw);
    V_udp->addLayout(h_udp2a);
    V_udp->addLayout(h_udp2b);
    V_udp->addLayout(h_udp2c);
    GB_udp_broad_settings->setLayout(V_udp);
    V_l->addWidget(GB_udp_broad_settings);

    /////// tcp broud ///////////////////
    socet_tcp_broad = new QTcpSocket(this);
    //connect(socet_tcp_broad, SIGNAL(connected()), this, SLOT(connected_stcpb()));
    //connect(socet_tcp_broad, SIGNAL(disconnected()), this, SLOT(disconnected_sstcpb()));
    QGroupBox *GB_tcp_broad_settings = new QGroupBox(tr("TCP Broadcast Settings")+":   - "+tr("DXKeeper Formatted Message")+" -   ");
    QVBoxLayout *V_tcp = new QVBoxLayout();
    V_tcp->setContentsMargins(5,2,5,2);
    V_tcp->setSpacing(0);
    //l_tcp_broad_info = new QLabel("Status: <font color='red'>TCP Broadcast Is Disabled And Disconnected</font>");
    //l_tcp_broad_info->setFont(f_t);
    //l_tcp_broad_info->setContentsMargins(5,2,5,0);
    QLabel *l_tcp_broad_host = new QLabel(tr("Server")+":");
    TCPServerBroad = new QLineEdit();
    TCPServerBroad->setText("127.0.0.1");
    QLabel *l_tcp_p = new QLabel(tr("Port")+":");
    TCPPortBroad= new QLineEdit();
    TCPPortBroad->setText("52001");
    TCPPortBroad->setInputMask("9999999999");
    cb_tcp_broad_log_adif = new QCheckBox(tr("Enable Logged QSO"));
    cb_tcp_broad_log_adif->setChecked(false);
    cb_tcp_broad_log_adif->setToolTip(tr("DXKeeper Formatted Message"));
    QHBoxLayout *h_tcp = new QHBoxLayout();
    h_tcp->setContentsMargins(5,1,5,0);
    h_tcp->setSpacing(5);
    h_tcp->addWidget(l_tcp_broad_host);
    h_tcp->addWidget(TCPServerBroad);
    h_tcp->addWidget(l_tcp_p);
    h_tcp->addWidget(TCPPortBroad);
    h_tcp->addWidget(cb_tcp_broad_log_adif);
    //V_tcp->addWidget(l_tcp_broad_info);
    V_tcp->addLayout(h_tcp);
    GB_tcp_broad_settings->setLayout(V_tcp);
    V_l->addWidget(GB_tcp_broad_settings);
    connect(cb_tcp_broad_log_adif, SIGNAL(toggled(bool)), this, SLOT(StartStopTCPBroad(bool)));
    /////// end tcp broud ///////////////////

    //ClubLog///////////////////////////////////////////
    socet_tcp_clublog = new QSslSocket(this);
    //connect(socet_tcp_clublog, SIGNAL(connected()), this, SLOT(connected_qrzlog()));
    //connect(socet_tcp_clublog, SIGNAL(disconnected()), this, SLOT(disconnected_qrzlog()));
    QGroupBox *GB_tcp_clublog = new QGroupBox(tr("Club Log Real-Time Upload Logged QSO")+":");
    QVBoxLayout *V_tcp_cl = new QVBoxLayout();
    V_tcp_cl->setContentsMargins(5,2,5,2);
    V_tcp_cl->setSpacing(0);
    //l_tcp_clublog_info = new QLabel(tr("Status")+": <font color='red'>"+tr("Disabled And Disconnected")+"</font>");
    //l_tcp_clublog_info->setContentsMargins(5,2,5,0);
    QLabel *l_ser_cl = new QLabel(tr("Server")+":");
    LeClubLogServer = new QLineEdit();
    //LeClubLogServer->setMaximumWidth(75);
    QLabel *l_po_cl = new QLabel(tr("Port")+":");
    LeClubLogPort = new QLineEdit();
    LeClubLogPort->setInputMask("99999");
    LeClubLogPort->setMaximumWidth(50);
    QLabel *l_pos0_cl = new QLabel(tr("Post")+":");
    LeClubLogPost0 = new QLineEdit();
    //LeClubLogPost0->setMaximumWidth(80);
    //QLabel *l_pos1_cl = new QLabel(tr("Post")+":");
    LeClubLogPost1 = new QLineEdit();
    //LeClubLogPost1->setMaximumWidth(80);
    QLabel *l_ma_cl = new QLabel(tr("E-Mail")+":");
    LeClubLogMail = new QLineEdit();
    QLabel *l_pa_cl = new QLabel(tr("Password")+":");
    LeClubLogPass = new QLineEdit();
    LeClubLogPass->setEchoMode(QLineEdit::Password);
    QLabel *l_cl_cl = new QLabel(tr("Callsign")+":");
    LClubLogCall = new QLabel();
    LClubLogCall->setContentsMargins(0,0,5,0);
    LClubLogCall->setStyleSheet("font-weight:bold;");//color: blue;
    cb_clublog = new QCheckBox(tr("Enable"));
    QHBoxLayout *h_cl = new QHBoxLayout();
    h_cl->setContentsMargins(5,0,5,0);
    h_cl->setSpacing(5);
    h_cl->addWidget(l_ser_cl);
    h_cl->addWidget(LeClubLogServer);
    h_cl->addWidget(l_po_cl);
    h_cl->addWidget(LeClubLogPort);
    h_cl->addWidget(l_pos0_cl);
    h_cl->addWidget(LeClubLogPost0);
    //h_cl->addWidget(l_pos1_cl);
    h_cl->addWidget(LeClubLogPost1);
    //h_cl->addWidget(cb_clublog);
    QHBoxLayout *h_clmpc = new QHBoxLayout();
    h_clmpc->setContentsMargins(5,4,5,0);
    h_clmpc->setSpacing(5);
    h_clmpc->addWidget(l_ma_cl);
    h_clmpc->addWidget(LeClubLogMail);
    h_clmpc->addWidget(l_pa_cl);
    h_clmpc->addWidget(LeClubLogPass);
    h_clmpc->addWidget(l_cl_cl);
    h_clmpc->addWidget(LClubLogCall);
    h_clmpc->addWidget(cb_clublog);
    //V_tcp_cl->addWidget(l_tcp_clublog_info);
    V_tcp_cl->addLayout(h_cl);
    V_tcp_cl->addLayout(h_clmpc);
    GB_tcp_clublog->setLayout(V_tcp_cl);
    V_l->addWidget(GB_tcp_clublog);
    file_cl_error = path+"/log/mshvlog_clublog_error.adi";

    //v2.68 stop stop changing
    LeClubLogServer->setText("clublog.org");
    //LeClubLogServer->setText("eqsl.cc");
    LeClubLogServer->setEnabled(false);
    LeClubLogPort->setText("443");//443 80
    LeClubLogPort->setEnabled(false);
    LeClubLogPost0->setText("realtime.php");//443 80
    LeClubLogPost0->setEnabled(false);
    LeClubLogPost1->setText("putlogs.php");//443 80
    LeClubLogPost1->setEnabled(false);
    //v2.68 stop stop changing

    connect(cb_clublog, SIGNAL(toggled(bool)), this, SLOT(cb_clublog_toggled()));
    connect(socet_tcp_clublog, SIGNAL(readyRead()), this, SLOT(readClubLog()));
    //ClubLog END///////////////////////////////////////////

    //QRZ///////////////////////////////////////////////
    socet_tcp_qrzlog = new QSslSocket(this);
    //connect(socet_tcp_qrzlog, SIGNAL(connected()), this, SLOT(connected_qrzlog()));
    //connect(socet_tcp_qrzlog, SIGNAL(disconnected()), this, SLOT(disconnected_qrzlog()));
    QGroupBox *GB_tcp_qrzlog = new QGroupBox(tr("QRZ Logbook Real-Time Upload Logged QSO")+":");
    QVBoxLayout *V_tcp_qrz = new QVBoxLayout();
    V_tcp_qrz->setContentsMargins(5,2,5,2);
    V_tcp_qrz->setSpacing(0);
    QLabel *l_ser_qrz = new QLabel(tr("Server")+":");
    LeQRZLogServer = new QLineEdit();
    QLabel *l_po_qrz = new QLabel(tr("Port")+":");
    LeQRZLogPort = new QLineEdit();
    LeQRZLogPort->setInputMask("99999");
    LeQRZLogPort->setMaximumWidth(60);
    QLabel *l_pos_qrz = new QLabel(tr("Post")+":");
    LeQRZLogPost = new QLineEdit();
    QLabel *l_api_qrz = new QLabel(tr("API Key")+":");
    LeQRZLogApi = new QLineEdit();
    LeQRZLogApi->setContentsMargins(0,0,5,0);
    LeQRZLogApi->setEchoMode(QLineEdit::Password);
    cb_qrzlog = new QCheckBox(tr("Enable"));
    QHBoxLayout *h_qrz = new QHBoxLayout();
    h_qrz->setContentsMargins(5,0,5,0);
    h_qrz->setSpacing(5);
    h_qrz->addWidget(l_ser_qrz);
    h_qrz->addWidget(LeQRZLogServer);
    h_qrz->addWidget(l_po_qrz);
    h_qrz->addWidget(LeQRZLogPort);
    h_qrz->addWidget(l_pos_qrz);
    h_qrz->addWidget(LeQRZLogPost);
    //h_qrz->addWidget(cb_qrzlog);
    QHBoxLayout *h_qrzmpc = new QHBoxLayout();
    h_qrzmpc->setContentsMargins(5,4,5,0);
    h_qrzmpc->setSpacing(5);
    h_qrzmpc->addWidget(l_api_qrz);
    h_qrzmpc->addWidget(LeQRZLogApi);
    h_qrzmpc->addWidget(cb_qrzlog);
    V_tcp_qrz->addLayout(h_qrz);
    V_tcp_qrz->addLayout(h_qrzmpc);
    GB_tcp_qrzlog->setLayout(V_tcp_qrz);
    V_l->addWidget(GB_tcp_qrzlog);
    LeQRZLogServer->setText("logbook.qrz.com");
    LeQRZLogServer->setEnabled(false);
    LeQRZLogPort->setText("443");//443 80
    LeQRZLogPort->setEnabled(false);
    LeQRZLogPost->setText("api");
    LeQRZLogPost->setEnabled(false);
    connect(cb_qrzlog, SIGNAL(toggled(bool)), this, SLOT(cb_qrzlog_toggled()));
    connect(socet_tcp_qrzlog, SIGNAL(readyRead()), this, SLOT(readQRZLog()));
    qrz_upd_timer = new QTimer();
    connect(qrz_upd_timer, SIGNAL(timeout()), this, SLOT(UplQRZLogAdif()));
    qrz_upd_timer->setSingleShot(true);
    qrz_error_str = "Unknown Error";
    qrz_once_data_err_msg = false;
    qrz_err_flag = false;
    qrz_pos_ul_writ = 0;
    qrz_pos_ul_read = 0;
    qrz_is_act_send_check = 0;
    //end QRZ///////////////////////////////////////////

    QVBoxLayout *V_2 = new QVBoxLayout();
    V_2->setContentsMargins(4,4,4,4);//(5,4,5,4);
    V_2->setSpacing(2);
	
    //EQSL///////////////////////////////////////////////
    socet_tcp_eqsl = new QSslSocket(this); //QSslSocket QTcpSocket
    //connect(socet_tcp_eqsl, SIGNAL(connected()), this, SLOT(connected_eqsl()));
    //connect(socet_tcp_eqsl, SIGNAL(disconnected()), this, SLOT(disconnected_eqsl()));
    QGroupBox *GB_tcp_EQSL = new QGroupBox(tr("eQSL Real-Time Upload Logged QSO")+":");
    QVBoxLayout *V_tcp_eqsl = new QVBoxLayout();
    V_tcp_eqsl->setContentsMargins(5,2,5,2);
    V_tcp_eqsl->setSpacing(0);
    QLabel *l_ser_eqsl = new QLabel(tr("Server")+":");
    LeEQSLServer = new QLineEdit();
    //LeEQSLServer->setMaximumWidth(60);
    QLabel *l_po_eqsl = new QLabel(tr("Port")+":");
    LeEQSLPort = new QLineEdit();
    LeEQSLPort->setInputMask("99999");
    LeEQSLPort->setMaximumWidth(60);
    QLabel *l_pos_eqsl = new QLabel(tr("Post")+":");
    LeEQSLPost = new QLineEdit();
    QLabel *l_msg_eqsl = new QLabel(tr("eQSL MSG")+":");
    LeEQSLmsg = new QLineEdit();
    LeEQSLmsg->setMaxLength(239);
    QHBoxLayout *h_eqsl = new QHBoxLayout();
    h_eqsl->setContentsMargins(5,0,5,0);
    h_eqsl->setSpacing(5);
    h_eqsl->addWidget(l_ser_eqsl);
    h_eqsl->addWidget(LeEQSLServer);
    h_eqsl->addWidget(l_po_eqsl);
    h_eqsl->addWidget(LeEQSLPort);
    h_eqsl->addWidget(l_pos_eqsl);
    h_eqsl->addWidget(LeEQSLPost);
    //h_eqsl->addWidget(l_msg_eqsl);
    //h_eqsl->addWidget(LeEQSLmsg);
    QLabel *l_ma_eqsl = new QLabel(tr("User")+":");
    LeEQSLUser = new QLineEdit();
    QLabel *l_pa_eqsl = new QLabel(tr("Password")+":");
    LeEQSLPass = new QLineEdit();
    LeEQSLPass->setEchoMode(QLineEdit::Password);
    QLabel *l_nc_eqsl = new QLabel(tr("QTH Nickname")+":");
    LeEQSLQTHNick = new QLineEdit();
    cb_eqsl = new QCheckBox(tr("Enable"));
    QHBoxLayout *h_eqslmpc = new QHBoxLayout();
    h_eqslmpc->setContentsMargins(5,4,5,0);
    h_eqslmpc->setSpacing(5);
    h_eqslmpc->addWidget(l_ma_eqsl);
    h_eqslmpc->addWidget(LeEQSLUser);
    h_eqslmpc->addWidget(l_pa_eqsl);
    h_eqslmpc->addWidget(LeEQSLPass);
    h_eqslmpc->addWidget(l_nc_eqsl);
    h_eqslmpc->addWidget(LeEQSLQTHNick);
    //h_eqslmpc->addWidget(cb_eqsl);
    QHBoxLayout *h_eqslm = new QHBoxLayout();
    h_eqslm->setContentsMargins(5,4,5,0);
    h_eqslm->setSpacing(5);
    h_eqslm->addWidget(l_msg_eqsl);
    h_eqslm->addWidget(LeEQSLmsg);
    h_eqslm->addWidget(cb_eqsl);
    V_tcp_eqsl->addLayout(h_eqsl);
    V_tcp_eqsl->addLayout(h_eqslmpc);
    V_tcp_eqsl->addLayout(h_eqslm);
    GB_tcp_EQSL->setLayout(V_tcp_eqsl);
    V_2->addWidget(GB_tcp_EQSL);
    LeEQSLServer->setText("eqsl.cc");
    LeEQSLServer->setEnabled(false);
    LeEQSLPort->setText("443");//443 80
    LeEQSLPort->setEnabled(false);
    LeEQSLPost->setText("qslcard/importADIF.cfm");
    LeEQSLPost->setEnabled(false);
    LeEQSLmsg->setText("TNX For QSO 73!");
    eqsl_upd_timer = new QTimer();
    connect(socet_tcp_eqsl,SIGNAL(readyRead()),this,SLOT(readEQSL()));
    connect(eqsl_upd_timer,SIGNAL(timeout()),this,SLOT(UplEQSLAdif()));
    connect(cb_eqsl,SIGNAL(toggled(bool)),this,SLOT(cb_eqsl_toggled()));
    eqsl_once_data_err_msg = false;
    eqsl_err_flag = false;
    eqsl_error_str = "Unknown Error";
    eqsl_pos_ul_writ = 0;
    eqsl_pos_ul_read = 0;
    eqsl_is_act_send_check = 0;
    //EQSL END ///////////////////////////////////////////////

    //Otp///////////////////////////////////////////////2.76
    socet_tcp_otp = new QSslSocket(this);
    //connect(socet_tcp_sfox, SIGNAL(connected()),this,SLOT(connected_sfox()));
    //connect(socet_tcp_sfox, SIGNAL(disconnected()),this,SLOT(disconnected_sfox()));
    //connect(socet_tcp_sfox, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(sslError(QList<QSslError>)) );
    QGroupBox *GB_tcp_otp = new QGroupBox(tr("OTP Settings")+":");//2.76sf for remove un comment
    //GB_tcp_otp = new QGroupBox(tr("OTP Settings")+":");//2.76sf for remove
    QVBoxLayout *V_tcp_otp = new QVBoxLayout();
    V_tcp_otp->setContentsMargins(5,2,5,2);
    V_tcp_otp->setSpacing(0);
    QLabel *l_ser_otp = new QLabel(tr("Server")+":");
    LeOtpServer = new QLineEdit();
    QLabel *l_po_otp = new QLabel(tr("Port")+":");
    LeOtpPort = new QLineEdit();
    LeOtpPort->setInputMask("99999");
    LeOtpPort->setMaximumWidth(60);
    CbOTPServers = new QComboBox();
    QStringList l00;
    l00<<tr("List Servers")<<"www.9dx.cc"<<"hamdx.org";//<<"ecqsl.com"; //l00<<" - Choice OTP Server - "<<"www.9dx.cc"<<"hamdx.org"; 
    CbOTPServers->addItems(l00);
    CbOTPServers->setCurrentIndex(0); 
    QLabel *l_key_otp = new QLabel(tr("Key")+":");
    LeOtpKey = new QLineEdit();
    LeOtpKey->setContentsMargins(0,0,5,0);
    const QRegularExpression b32(QString("(^[") + QString(BASE32_CHARSET)+QString(BASE32_CHARSET).toLower() + QString("]{16}$)|(^$)"));
    LeOtpKey->setValidator(new QRegularExpressionValidator(b32, this));
    //LeSFoxKey->setEchoMode(QLineEdit::Password);
    cb_otp_msg = new QCheckBox(tr("Show OTP Messages"));
    cb_otp_key = new QCheckBox(tr("TX OTP Key Super Fox"));
    //cb_otp_key->setChecked(false);//2.76.2
    //LeOtpKey->setEnabled(false);
    //cb_otp_mamd_key = new QCheckBox("TX OTP MADXpedition");
    QHBoxLayout *h_otp = new QHBoxLayout();
    h_otp->setContentsMargins(5,0,5,0);
    h_otp->setSpacing(5);
    h_otp->addWidget(l_ser_otp);
    h_otp->addWidget(LeOtpServer);
    h_otp->addWidget(CbOTPServers);
    h_otp->addWidget(l_po_otp);
    h_otp->addWidget(LeOtpPort);
    h_otp->addWidget(cb_otp_msg);
    //h_otp->addWidget(l_pos_qrz);
    //h_otp->addWidget(LeSFoxPost);
    QHBoxLayout *h_otp1 = new QHBoxLayout();
    h_otp1->setContentsMargins(5,4,5,0);
    h_otp1->setSpacing(5);
    //h_otp1->addWidget(cb_otp_mamd_key);
    h_otp1->addWidget(cb_otp_key);
    h_otp1->addWidget(l_key_otp);
    h_otp1->addWidget(LeOtpKey);
    //h_sfox1->addWidget(cb_otp_msgs);
    V_tcp_otp->addLayout(h_otp);
    //V_tcp_otp->addWidget(CbOTPServers);
    V_tcp_otp->addLayout(h_otp1);
    GB_tcp_otp->setLayout(V_tcp_otp);
    V_2->addWidget(GB_tcp_otp);
    LeOtpServer->setText("www.9dx.cc");
    //LeSFoxServer->setEnabled(false);
    LeOtpPort->setText("443");//443 80
    LeOtpPort->setEnabled(false);
    connect(CbOTPServers,SIGNAL(currentIndexChanged(QString)),this,SLOT(CbOTPServersChanged(QString)));       
    connect(cb_otp_key,SIGNAL(toggled(bool)),this,SLOT(cb_otp_key_toggled()));
    //connect(cb_otp_mamd_key,SIGNAL(toggled(bool)),this,SLOT(cb_otp_key_toggled()));
    connect(LeOtpKey,SIGNAL(textEdited(QString)),this,SLOT(LeOtpKeyEdited(QString)));
    //connect(cb_otp_sfox_msg,SIGNAL(toggled(bool)),this,SIGNAL(EmitSFoxRxMsg(bool)));
    connect(cb_otp_msg,SIGNAL(toggled(bool)),this,SLOT(cb_otp_msg_toggled(bool)));
    connect(socet_tcp_otp,SIGNAL(readyRead()),this,SLOT(readOtp()));
    otp_check_timer = new QTimer();
    connect(otp_check_timer, SIGNAL(timeout()),this,SLOT(SendOtpCheckDataGram()));
    otp_check_timer->setSingleShot(true);
    otp_pos_ch_writ = 0;
    otp_pos_ch_read = 0;
    otp_is_act_send_check = 0;
    /*qrz_error_str = "Unknown Error";
    qrz_once_data_err_msg = false;
    qrz_err_flag = false;
    qrz_pos_ul_writ = 0;
    qrz_pos_ul_read = 0;
    qrz_is_act_send_check = 0;*/
    //end Otp///////////////////////////////////////////

    THvBcnListW = new HvBcnListW(path,indsty,x,y);

    TRadListW = new QWidget();
    TRadListW->setWindowTitle(tr("Radio And Frequencies Configuration"));
    QVBoxLayout *V_rlw = new QVBoxLayout(TRadListW);
    V_rlw->setContentsMargins(5,5,5,5);
    V_rlw->setSpacing(2);
    THvRadList = new HvRadList(indsty);
    //SetDefaultFreqs(true);//true all default
    V_rlw->addWidget(THvRadList);
    //V_l->addWidget(THvRadList);
    QPushButton *b_reset_default_freqs= new QPushButton(tr("Default Standard FREQs"));// TABLE   "DEFAULT STANDARD FREQs"  "Default Standard FREQs"
    b_reset_default_freqs->setFixedHeight(20);
    connect(b_reset_default_freqs, SIGNAL(clicked(bool)), this, SLOT(ResetDefaultFreqsBut()));
    b_reset_default_freqs_cont= new QPushButton(tr("N/A For This Activity Type"));
    b_reset_default_freqs_cont->setFixedHeight(20);
    connect(b_reset_default_freqs_cont, SIGNAL(clicked(bool)), this, SLOT(SetDefaultFreqsActTypeBut()));
    b_reset_default_freqs_cont->setEnabled(false);
    QHBoxLayout *H_bb = new QHBoxLayout();
    H_bb->setContentsMargins(0,0,0,0);
    H_bb->setSpacing(2);
    H_bb->addWidget(b_reset_default_freqs);
    H_bb->addWidget(b_reset_default_freqs_cont);
    V_rlw->addLayout(H_bb);
    TRadListW->setLayout(V_rlw);
    SetDefaultFreqs(true);//true all default

    //V_l->addLayout(H_bb);
	//V_l->setAlignment(Qt::AlignTop);
	//setLayout(V_l);

    TEditRadInfo = new EditRadInfo(this);
    connect(THvRadList, SIGNAL(EmitDoubleClick(QStringList,int)), TEditRadInfo, SLOT(SetEditStInfo(QStringList,int)));
    connect(TEditRadInfo, SIGNAL(EmitSetInfo(QStringList,int)), this, SLOT(StInfoChanged(QStringList,int)));

    psk_Reporter = new PSKReporter(false, QString
                                   {"MSHV v" + QString(VER_MS)
                                   }
                                   .simplified (),"",4739);
    connect(cb_start_stop_psk_rpt, SIGNAL(toggled(bool)), this, SLOT(StartStopReport(bool)));
    connect(UDPServer, SIGNAL(textChanged(QString)), this, SLOT(ServTextChanged(QString)));
    connect(pb_re_conect, SIGNAL(clicked(bool)), this, SLOT(Reconnect()));
    connect(UDPPort, SIGNAL(textChanged(QString)), this, SLOT(PortTextChanged(QString)));
    connect(cb_udp_tcp_psk_rpt, SIGNAL(toggled(bool)), this, SLOT(UdpTcpChangedPsk(bool)));
    connect(psk_Reporter, SIGNAL(ConectionInfo(QString)), this, SLOT(SetConectionInfo(QString)));

    //ReadBcnList(path+"/settings/database/msbcn_db.dbbn");
    //m_messageClient->set_server_port(4739);//<-problem
    //m_messageClient->set_server("report.pskreporter.info");//     report.pskreporter.info
    //QString App_Path = (QCoreApplication::applicationDirPath());

    TSpotDialog = new SpotDialog(indsty,parent);//this
    connect(TSpotDialog, SIGNAL(EmitReconnect()), this, SIGNAL(EmitOpenRadNetWToRecon()));
    connect(TSpotDialog, SIGNAL(FindLocFromDB(QString)), this, SIGNAL(FindLocFromDB(QString)));

    TelnetClient = new HvTcpClient();
    connect(TelnetClient, SIGNAL(ConectionInfo(QString)), this, SLOT(SetConectionTcpInfo(QString)));
    //qDebug()<<"Time===========================";

    if (!inst.isEmpty()) inst = " -"+inst;
    m_messageClientBroad = new MessageClient {"MSHV"+inst,VER_MS,"",2237,true,this};
    //m_messageClientBroad = new MessageClient {"MSHV ID:14MHz",VER_MS,"",2237,true,this};
    connect(m_messageClientBroad, SIGNAL(error(QString)), this, SLOT(networkErrorUDPBrodcast(QString)));
    connect(m_messageClientBroad, SIGNAL(replay()), this, SLOT(replayDecodes()));
    connect(m_messageClientBroad, SIGNAL(reply_clr(QStringList)), this, SLOT(set_reply_clr(QStringList)));
    connect(m_messageClientBroad, SIGNAL(halt_tx(bool)), this, SLOT(set_halt_tx(bool)));
    connect(m_messageClientBroad, SIGNAL(ConectionInfo(QString)), this, SLOT(ConectionInfoBroad(QString)));
    connect(cb_udp_broad_log_qso, SIGNAL(toggled(bool)), this, SLOT(StartStopUdpBroad(bool)));
    connect(cb_udp_broad_log_adif, SIGNAL(toggled(bool)), this, SLOT(StartStopUdpBroad(bool)));
    connect(cb_udp_broad_decod, SIGNAL(toggled(bool)), this, SLOT(StartStopUdpBroad(bool)));
    connect(pb_re_conect_udp_broad, SIGNAL(clicked(bool)), this, SLOT(ReconnectUdpBroad()));
    connect(UDPServerBroad, SIGNAL(textChanged(QString)), this, SLOT(UDPSrvPortBroadChanged(QString)));
    connect(UDPPortBroad, SIGNAL(textChanged(QString)), this, SLOT(UDPSrvPortBroadChanged(QString)));

    sr_path = path+"/settings/ms_stinfonet";
    ReadSettings();

    //this->setSize(450,240);//setMinimumWidth(450);
    //this->resize(452,520);
    //block_emit_tcp = false;
    //ReconnectTcp();

    decod_upd_timer = new QTimer();
    connect(decod_upd_timer, SIGNAL(timeout()), this, SLOT(DecodUpdTimer()));
    decod_upd_timer->setSingleShot(true);

    write_status_timer = new QTimer();
    connect(write_status_timer, SIGNAL(timeout()), this, SLOT(WriteStatusTimer()));
    write_status_timer->setSingleShot(true);

    const quint8 digg[20] =
        {
            86,194,0,7,19,154,167,200,124,171,153,168,77,76,108,157,53,192,92,128 // x2
            //22210,7,5018,42952,31915,39336,19788,27805,13760,23680 // x4
        };
    cl_send_file = false;
    cl_err_flag = false;
    cl_once_data_err_msg = false;
    cl_pos_ul_writ = 0;
    cl_pos_ul_read = 0;
    cl_is_act_send_check = 0; //0=no_active 1=send data 2=send file 3=check data 4=check file
    cl_upd_timer = new QTimer();
    connect(cl_upd_timer, SIGNAL(timeout()), this, SLOT(UplClubLogAdif()));
    cl_upd_timer->setSingleShot(true);
    _kkwe2_ = "";
    for (int i = 0; i < 20; ++i) _kkwe2_ += QString("%1").arg(digg[i],2,16,QLatin1Char('0'));
    
    QTabWidget *TW = new QTabWidget();
    QWidget *T0 = new QWidget();
    V_l->setAlignment(Qt::AlignTop);
    T0->setLayout(V_l);
    QWidget *T1 = new QWidget();
    V_2->setAlignment(Qt::AlignTop);
    T1->setLayout(V_2);
    TW->addTab(T0," "+tr("Page")+"  1");
    TW->addTab(T1," "+tr("Page")+"  2");    
    
    QVBoxLayout *V_0 = new QVBoxLayout(this);
    V_0->setContentsMargins(2,2,2,2);
    V_0->setSpacing(0);
    V_0->addWidget(TW);
    setLayout(V_0);
    
}
RadioAndNetW::~RadioAndNetW()
{
    if (cb_start_stop_psk_rpt->isChecked()) psk_Reporter->stop(); //psk_Reporter->sendReport(true);
    TelnetClient->DisconectOnly();
    StartStopClubLog(false);
    if (socet_udp2_broad->state() == QAbstractSocket::ConnectedState) socet_udp2_broad->disconnectFromHost();
    StartStopQRZLog(false);
    StartStopEQSL(false);
    StartStopOtp(false);
    //SaveSettings(); //2.68 for TAB "Radio And Frequencies Configuration" moved to hvtxw.cpp qDebug()<<"DELETE RadioAndNetW";
}
void RadioAndNetW::GetFtFr(long long int *a)
{
	const long long int wsprf[10] = {1836600,5364700,3568600,7038600,10138700,14095600,18104600,21094600,24924600,28124600};
    QString s;
    int j = 0;
    for (int i = 3; i < 14; ++i)
    {
        if (i==5 || i==12) continue;
        s = all_bands_mods_frq[i][3];
        s.remove(".");
        a[j] = s.toLongLong();
        j++;
        /*s = all_bands_mods_frq[i][6];//+FT2
        s.remove(".");
        a[j] = s.toLongLong();
        j++;*/
        if (i==3) continue;
        s = all_bands_mods_frq[i][2];
        s.remove(".");
        a[j] = s.toLongLong();
        j++;
    }
    for (int i = 0; i < 10; ++i)
    {
    	a[j] = wsprf[i];
    	j++;
   	} //for (int i = 0; i < j; ++i) printf(" %d Band =%lld\n",i,a[i]);
}
void RadioAndNetW::RefreshUdpOrTcpBroadLoggedAll()
{
    emit EmitUdpBroadLoggedAll(cb_udp_broad_log_qso->isChecked(),GetLogAdifAll());
}
bool RadioAndNetW::isUnsafe(QChar compareChar)
{
    const int c_c = 25; //const QString csUnsafeString= "\"<>%\\^[]`+$,@:;/!#?=&";
    const QChar csUnsafe[c_c]=
        {
            //old '\"','<','>','%','\\','^','[',']','`','+','$',',','@',':',';','/','!','#','?','=','&'
            //HV since 2005 the current RFC in use for URIs standard is RFC 3986.
            '\"','<','>','%','\\','^','[',']','`','+','$',',','@',':',';','/','!','#','?','=','&','(',')','\'','*'
            //,'%'         ,'[',']'    ,'+','$',',','@',':',';','/','!','#','?','=','&','(',')','\'','*'
        };
    bool bcharfound = false;
    QChar tmpsafeChar;
    for (int ichar_pos = 0; ichar_pos < c_c; ++ichar_pos) //m_strLen = csUnsafeString.count();
    {
        tmpsafeChar = csUnsafe[ichar_pos]; //qDebug()<<csUnsafeString[ichar_pos];
        if (tmpsafeChar == compareChar)
        {
            bcharfound = true;
            break;
        }
    }
    int char_ascii_value = 0;
    char_ascii_value = (int)compareChar.toLatin1();
    if (bcharfound == false &&  char_ascii_value > 32 && char_ascii_value < 123) return false; // found no unsafe chars, return false
    else return true;
    return true;
}
QString RadioAndNetW::decToHex(QChar num, int radix)
{
    const QChar hexVals[32]=
        {
            '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'
        };
    int temp=0;
    QString csTmp;
    int num_char;
    num_char = (int)num.toLatin1();
    if (num_char < 0) num_char = 256 + num_char;
    while (num_char >= radix)
    {
        temp = num_char % radix;
        num_char = (int)floor((num_char / radix) * 1.0);
        csTmp = hexVals[temp];
    }
    csTmp += hexVals[num_char];
    if (csTmp.count() < 2) csTmp += '0';
    QString strdecToHex; // Reverse the String
    for (int i = csTmp.count()-1; i >= 0; --i) strdecToHex.append(csTmp.at(i));
    return strdecToHex;
}
QString RadioAndNetW::convert(QChar val)
{
    QString csRet;
    csRet += "%";
    csRet += decToHex(val,16);
    return  csRet;
}
QString RadioAndNetW::URLEncode(QString strEncode)
{
    QString  strSrc;
    QString  strDest;
    strSrc = strEncode;
    for (int i = 0; i < strSrc.count(); ++i)
    {
        QChar ch = strSrc[i];
        if (ch < ' ') ch = ch;  //??? if (ch < ' ') qDebug()<<(int)ch.toLatin1()<<(int)' ';
        if (!isUnsafe(ch)) strDest += ch;// Safe Character
        else strDest += convert(ch);     // get Hex Value of the Character
    }
    return strDest;
}
#include <QToolTip>
#define MAXUPLOAUD 10 //2.71 old=6
/*void RadioAndNetW::connected_sfox()
{
    qDebug()<<"CONECTED  <--";
}
void RadioAndNetW::disconnected_sfox()
{
    qDebug()<<"DISCONECT <--";
}*/
/*void RadioAndNetW::sslError(QList<QSslError> errors)
{
	qDebug()<<"SSL ERROR <-----------------------------";
    socet_tcp_sfox->ignoreSslErrors();
}*/
///// Otp //////
void RadioAndNetW::SendOtpTxKey()
{
    QString s = QString("%1").arg(cb_otp_key->isChecked())+"#";
    s.append(LeOtpKey->text());
    s.append("#0#0");//protect
    emit EmitOtpTxKey(s);
}
void RadioAndNetW::cb_otp_key_toggled()
{
    SendOtpTxKey();
    //if (!cb_otp_key->isChecked()) LeOtpKey->setEnabled(false);//2.76.2
    //else LeOtpKey->setEnabled(true);
}
void RadioAndNetW::LeOtpKeyEdited(QString s0)
{
    LeOtpKey->setText(s0.toUpper());
    SendOtpTxKey();
}
void RadioAndNetW::RefreshOtpKeyMsg()
{
    cb_otp_key_toggled();
    emit EmitOtpRxMsg(cb_otp_msg->isChecked());
}
void RadioAndNetW::CbOTPServersChanged(QString s)
{
	if (CbOTPServers->currentIndex()==0 || s.isEmpty()) return;//if (s=="List Servers" || s.isEmpty()) return;
	LeOtpServer->setText(s);
	CbOTPServers->setCurrentIndex(0);//reset to begin
}
/*bool RadioAndNetW::StartStopOtp(bool f)
{
	static QString serv_0 = "#";
    if (f)
    {
    	if (serv_0 != LeOtpServer->text())//2.761
    	{
    		serv_0 = LeOtpServer->text();//www.9dx.cc hamdx.org
    		if (socet_tcp_otp->state() == QAbstractSocket::ConnectedState)
    		{
    			socet_tcp_otp->disconnectFromHost();
    			socet_tcp_otp->waitForDisconnected(350);
   			}
   		}
        if (socet_tcp_otp->state() != QAbstractSocket::ConnectedState)
        {
            int p1 = LeOtpPort->text().toInt();
            socet_tcp_otp->connectToHostEncrypted(LeOtpServer->text(),p1);//socet_tcp_sfox->connectToHost(LeSFoxServer->text(),80);
            socet_tcp_otp->waitForConnected(500);
        }
    }
    else if (socet_tcp_otp->state() == QAbstractSocket::ConnectedState) socet_tcp_otp->disconnectFromHost();
    
    if (socet_tcp_otp->state() == QAbstractSocket::ConnectedState) return true;
    else return false;
}*/
void RadioAndNetW::StartStopOtp(bool f)
{
    if (f)
    {
        if (socet_tcp_otp->state() != QAbstractSocket::ConnectedState)
        {
            int p1 = LeOtpPort->text().toInt();
            socet_tcp_otp->connectToHostEncrypted(LeOtpServer->text(),p1);//socet_tcp_sfox->connectToHost(LeSFoxServer->text(),80);
            socet_tcp_otp->waitForConnected(500);
        }
    }
    else if (socet_tcp_otp->state() == QAbstractSocket::ConnectedState) socet_tcp_otp->disconnectFromHost();
}
void RadioAndNetW::cb_otp_msg_toggled(bool f)
{
    //StartStopSFox(f);
    emit EmitOtpRxMsg(f);
    //if (f) SendSFoxCheck("-");
}
#define MAXOTPCHECK 8
typedef struct
{
    QString time;
    QString call;
    QString freq;
    //QString msg;
}
SotpAD;
static SotpAD otp_all_data[MAXOTPCHECK+10];
static QString otp_formated_msg[MAXOTPCHECK+10];
//#define _TEST_OTP_
void RadioAndNetW::ParseOtpMsg(QString sba,uint8_t id)
{
    int pos = -1;
    QStringList l0 = sba.split(" ");
    if (otp_pos_ch_writ>MAXOTPCHECK) otp_pos_ch_writ=MAXOTPCHECK;//2.76.5
    for (int i = 0; i < otp_pos_ch_writ; ++i)
    {
        QString call0 = otp_all_data[i].call;
        for (int j = 0; j < l0.count(); ++j)
        {
            if (l0.at(j)==call0)
            {
                pos = i;
                break;
            }
        }
        if (pos>-1) break;
    }
#if defined _TEST_OTP_    
    if (id==3) pos=0;
#endif    
    if (pos>-1 && pos<MAXOTPCHECK)//2.76.5
    {
        QString s = otp_all_data[pos].time+"#";
        s.append("0#");
        s.append("0.0#");
        s.append("0#");
        if		(id==1) s.append(otp_all_data[pos].call+" verified#");
        else if (id==2) s.append(otp_all_data[pos].call+" invalid#");
#if defined _TEST_OTP_         
        else if (id==3) s.append(otp_all_data[pos].call+" not found#");
#endif        
        s.append("0#");
        s.append("1.0#");
        s.append(otp_all_data[pos].freq);
        otp_all_data[pos].call = ".";//protect two times decode
        emit EmitOtpVerif(s,id);
    }
}
void RadioAndNetW::readOtp()
{
    /*HV variants
    \r\n\r\n28\r\n2024-10-15T05:33:30Z LZ2HV 865414 INVALID\r\n"  next-> "0\r\n\r\n"
    \r\n\r\n2024-10-15T05:29:30Z LZ2HV 633766 VERIFIED\r\n"  next-> "0\r\n\r\n"
    */
    QByteArray ba = socet_tcp_otp->readAll();
    QString sba = QString(ba.data());
    //if (sba.count()<1) return;//2.76.5
    int ibeg = sba.indexOf("\r\n\r\n");
    int csba = (sba.count()-(ibeg+4)); //qDebug()<<sba;
    if (ibeg>-1 && csba>0)//2.77 //if (ibeg>-1 && sba.count()>5)//2.77 
    {        
        sba = sba.mid(ibeg+4,csba); //qDebug()<<"INDEX="<<sba;//sba = sba.mid(ibeg+4,sba.count()-(ibeg+4));
        if		(sba.contains(" VERIFIED")) ParseOtpMsg(sba,1);
        else if (sba.contains(" INVALID"))  ParseOtpMsg(sba,2);
#if defined _TEST_OTP_        
        else if (sba.contains(" not found")/* || sba.contains("N/A")*/) ParseOtpMsg(sba,3); //Callsign not found
#endif         	      
    }
    if (otp_is_act_send_check == 2)
    {
        otp_check_timer->stop();
        otp_check_timer->start(50);
    }
}
void RadioAndNetW::SendOtpCheckDataGram()
{
    /*https://www.9dx.cc/check/LZ2HV/2024-10-13T12:03:00/432242.text*/
    /*DataGram Debug -> curl -v https://www.9dx.cc/check/LZ2HV/2024-10-13T12:03:00/432242.text*/
    static QString serv_otp0 = "#";
    if (otp_is_act_send_check < 2)
    {    	
    	if (serv_otp0 != LeOtpServer->text()/*+LeOtpPort->text()*/)//2.76.1 Connect/Disconnect on the fly
    	{
    		serv_otp0 = LeOtpServer->text()/*+LeOtpPort->text()*/;//www.9dx.cc hamdx.org
    		if (socet_tcp_otp->state() == QAbstractSocket::ConnectedState)
    		{
    			socet_tcp_otp->disconnectFromHost();
    			socet_tcp_otp->waitForDisconnected(350);
   			}
   		}
        if (socet_tcp_otp->state() != QAbstractSocket::ConnectedState) StartStopOtp(true);
        if (socet_tcp_otp->state() == QAbstractSocket::ConnectedState)//if (StartStopOtp(true))        	
        {
            QByteArray res = "GET /"+otp_formated_msg[otp_pos_ch_read].toUtf8()+" HTTP/1.1\r\n";
            res.append("Host: "+LeOtpServer->text().toUtf8()+"\r\n");
            QString tmp = "User-Agent: MSHV/"+QString(VER_MS)+"\r\n";
            res.append(tmp.toUtf8());
            res.append("Accept: */*\r\n\r\n");
            socet_tcp_otp->write(res); //qDebug()<<"MSHV Send-->"; qDebug()<<otp_pos_ch_read;//<<res.data();
            otp_is_act_send_check = 2;
            otp_check_timer->start(2000);
        }
        else
        {
            otp_check_timer->stop();
            //qrz_err_flag = true;
            //CheckQRZLogError();
            otp_pos_ch_writ = 0;
            otp_pos_ch_read = 0;
            otp_is_act_send_check = 0;
            /*QString s = " MSHV "+tr("Unsuccessful connection to ")+LeOtpServer->text();
            QPoint post = w_parent->mapToGlobal(QPoint(0,0));
            post+=QPoint(w_parent->width()/2-160,(-w_parent->pos().y()+50));
            QToolTip::showText(post,s,this,rect(),8000);*/
        }
    }
    else //check
    {
        otp_check_timer->stop();
        //CheckQRZLogError();
        otp_pos_ch_read++;
        if (otp_pos_ch_read >= otp_pos_ch_writ)
        {
            otp_pos_ch_writ = 0;
            otp_pos_ch_read = 0;
            otp_is_act_send_check = 0;
        }
        else
        {
            otp_is_act_send_check = 1;//1=next send data
            otp_check_timer->start(200);
        }
    }
}
//#include <QNetworkInterface>
void RadioAndNetW::SendOtpCheck(QString s)
{
    /*QList<QNetworkInterface> list = QNetworkInterface::allInterfaces(); // now you have interfaces list
    foreach (QNetworkInterface iface, list)  // this should print all interfaces' names
    {
    	if (iface.flags() & QNetworkInterface::IsUp)
    	qDebug() << iface.name() << iface.flags();
    }*/
    //for (int i = 0; i<3; i++)
    //{
    //s = "101200 $VERIFY$ K1JT 958844 751";//20250108 		101200 	958844  HH2AA K1JT
    QStringList l = s.split(" "); //qDebug()<<l;//"173730 $VERIFY$ LZ2HV 477064 751"
    if (l.count()<5) return;
    if (l.at(3)=="000000") return;		//no check 000000 code
    if (l.at(2).contains("...")) return;//no check <...>
    if (otp_pos_ch_writ > MAXOTPCHECK) return; //qDebug()<<l;
    otp_all_data[otp_pos_ch_writ].time = l.at(0); //qDebug()<<"Check--------------------";
    otp_all_data[otp_pos_ch_writ].call = l.at(2);
    otp_all_data[otp_pos_ch_writ].freq = l.at(4);//+QString("%1").arg(i);
    QString Call = URLEncode(l.at(2));  //qDebug()<<otp_pos_ch_writ<<sfox_all_data[otp_pos_ch_writ].freq;
    QDateTime timestamp = QDateTime(QDateTime::currentDateTimeUtc().date(),QTime::fromString(l.at(0),"hhmmss"));
    otp_formated_msg[otp_pos_ch_writ] = "check/" + Call + QString("/%1/%2.text").arg(timestamp.toString(Qt::ISODate)).arg(l.at(3));
    otp_pos_ch_writ++;
    if (otp_is_act_send_check == 0)//0=no_active
    {
        //qrz_once_data_err_msg = true;
        otp_is_act_send_check = 1;//1=send data
        //qrz_error_str = tr("Please check your Internet connection");
        otp_check_timer->start(20); //4900
    }
    //}
}
///// end Otp //////
///// EQSL //////
void RadioAndNetW::StartStopEQSL(bool f)
{
    if (f)
    {
        if (socet_tcp_eqsl->state() != QAbstractSocket::ConnectedState)
        {
            int p1 = LeEQSLPort->text().toInt();
            socet_tcp_eqsl->connectToHostEncrypted(LeEQSLServer->text(),p1); //socet_tcp_eqsl->connectToHost(LeEQSLServer->text(),p1);
            socet_tcp_eqsl->waitForConnected(500);
        }
    }
    else if (socet_tcp_eqsl->state() == QAbstractSocket::ConnectedState) socet_tcp_eqsl->disconnectFromHost();
}
void RadioAndNetW::cb_eqsl_toggled()
{
    RefreshUdpOrTcpBroadLoggedAll();  //StartStopEQSL(cb_qrzlog->isChecked()); //no needed 15 sec time out and close
}
void RadioAndNetW::readEQSL()
{
    QByteArray ba = socet_tcp_eqsl->readAll();
    QString sba = QString(ba.data());
    int ice = 0;
    int icb = sba.indexOf("\r\n\r\n");
    if (icb>-1) sba = sba.mid(icb+4,sba.count()-(icb+4));
    else sba = "";
    if (sba.contains("Error")) //if (sba.contains("Result: 1 out of 1 records added")) eqsl_err_flag = false;
    {
        icb = sba.indexOf("Error");
        ice = sba.indexOf("\r\n",icb);
        if (ice<icb) ice = sba.count();
        eqsl_error_str = sba.mid(icb,ice-icb);  //sba.mid(icb,sba.count()-icb);
    }
    else if (sba.contains("Warning"))
    {
        icb = sba.indexOf("Warning");
        ice = sba.indexOf("\r\n",icb);
        if (ice<icb) ice = sba.count();
        eqsl_error_str = sba.mid(icb,ice-icb);
        eqsl_error_str.remove("<BR>");
    }
    /*else if (sba.contains("Caution"))
    {
    	 icb = sba.indexOf("Caution"); 
    	 ice = sba.indexOf("\r\n",icb);
    	 if (ice<icb) ice = sba.count();
    	 eqsl_error_str = sba.mid(icb,ice-icb);  
    	 eqsl_error_str.remove("<BR>");	
    }*/
    else if (sba.contains("Result") || sba.contains("Caution")) eqsl_err_flag = false;
    else eqsl_error_str = "Unknown Error";
    if (!eqsl_err_flag && eqsl_is_act_send_check == 2)
    {
        eqsl_upd_timer->stop();
        eqsl_upd_timer->start(1100);
    }
    //qDebug()<<"EQSL <-- "<<eqsl_err_flag<<eqsl_is_act_send_check<<sba;
    //qDebug()<<"eqsl_error_str <-- "<<eqsl_error_str;
}
void RadioAndNetW::CheckEQSLError()
{
    if (eqsl_once_data_err_msg && eqsl_err_flag)
    {
        eqsl_once_data_err_msg = false;
        QString s = " MSHV "+tr("Unsuccessful upload to")+" eQSL\n"+eqsl_error_str;
        QPoint post = w_parent->mapToGlobal(QPoint(0,0));
        post+=QPoint(w_parent->width()/2-160,(-w_parent->pos().y()+50));
        QToolTip::showText(post,s,this,rect(),8000);
    }
}
QString eqsl_adif_arr[MAXUPLOAUD+10];
void RadioAndNetW::UplEQSLAdif()
{
	//static QString serv_eqsl0 = "#";
    if (eqsl_is_act_send_check < 2)
    {
    	/*if (serv_eqsl0 != LeEQSLServer->text())//2.761 Connect/Disconnect on the fly
    	{
    		serv_eqsl0 = LeEQSLServer->text();
    		if (socet_tcp_eqsl->state() == QAbstractSocket::ConnectedState)
    		{
    			socet_tcp_eqsl->disconnectFromHost();
    			socet_tcp_eqsl->waitForDisconnected(350);
   			}
   		}*/    	
        if (socet_tcp_eqsl->state() != QAbstractSocket::ConnectedState) StartStopEQSL(true);
        if (socet_tcp_eqsl->state() == QAbstractSocket::ConnectedState)
        {
            eqsl_err_flag = true;
            QString adiftrim = eqsl_adif_arr[eqsl_pos_ul_read];
            adiftrim.remove(adiftrim.size()-1,1);//remove last \n  //adiftrim = adiftrim.trimmed();
            QString s = LeEQSLmsg->text();
            s = s.trimmed();
            if (!s.isEmpty()) adiftrim.insert(adiftrim.count()-5,"<QSLMSG:"+QString("%1").arg(s.count())+">"+s);//<EOR>
            QByteArray res = "POST /"+LeEQSLPost->text().toUtf8()+" HTTP/1.1\r\n";
            res.append("Host: "+LeEQSLServer->text().toUtf8()+"\r\n");
            QString tmp = "User-Agent: MSHV/"+QString(VER_MS)+"\r\n";
            res.append(tmp.toUtf8());
            res.append("Accept: */*\r\n");
            res.append("Content-Type: application/x-www-form-urlencoded\r\n");
            tmp = "<ADIF_VER:5>3.1.0<PROGRAMID:4>MSHV";
            s = LeEQSLUser->text();
            tmp.append("<EQSL_USER:"+QString("%1").arg(s.count())+">"+s);
            s = LeEQSLPass->text();
            tmp.append("<EQSL_PSWD:"+QString("%1").arg(s.count())+">"+s+"<EOH>\n");
            s = LeEQSLQTHNick->text();
            tmp.append("<APP_EQSL_QTH_NICKNAME:"+QString("%1").arg(s.count())+">"+s); //qDebug()<<tmp+adiftrim;
            QString cnt = "ADIFData="+URLEncode(tmp+adiftrim);
            //cnt = "ADIFData="+URLEncode("<ADIF_VER:5>3.1.0<PROGRAMID:4>MSHV<EOH>"+adiftrim);
            //cnt.append("&EQSL_USER="+LeEQSLUser->text().toUtf8());
            //cnt.append("&EQSL_PSWD="+LeEQSLPass->text().toUtf8());
            tmp = "Content-Length: "+QString("%1").arg(cnt.count());
            res.append(tmp.toUtf8());
            tmp = "\r\n\r\n"+cnt;
            res.append(tmp.toUtf8()); //qDebug()<<res.data();
            socet_tcp_eqsl->write(res);
            eqsl_is_act_send_check = 2; //qDebug()<<"ttt"<<ttt.elapsed(); ttt.start();
            eqsl_upd_timer->start(2500);//2500 1500
            //qDebug()<<"MSHV Send-->";//<<res.data();
        }
        else
        {
            eqsl_upd_timer->stop();
            eqsl_err_flag = true;
            CheckEQSLError();
            eqsl_pos_ul_writ = 0;
            eqsl_pos_ul_read = 0;
            eqsl_is_act_send_check = 0;
        }
    }
    else //check
    {
        eqsl_upd_timer->stop();
        CheckEQSLError();
        eqsl_pos_ul_read++;
        if (eqsl_pos_ul_read >= eqsl_pos_ul_writ)
        {
            eqsl_pos_ul_writ = 0;
            eqsl_pos_ul_read = 0;
            eqsl_is_act_send_check = 0;
        }
        else
        {
            eqsl_is_act_send_check = 1;//1=next send data
            eqsl_upd_timer->start(500);
        }
    }
}
void RadioAndNetW::SendEQSLAdif(QString s)
{
    //for (int i = 0; i<3; i++)
    //{
    if (LeEQSLUser->text().isEmpty() || LeEQSLPass->text().isEmpty()) return;
    if (eqsl_pos_ul_writ > MAXUPLOAUD) return;
    eqsl_adif_arr[eqsl_pos_ul_writ] = s;
    eqsl_pos_ul_writ++;
    if (eqsl_is_act_send_check == 0)//0=no_active
    {
        eqsl_once_data_err_msg = true;
        eqsl_is_act_send_check = 1;//1=send data
        eqsl_error_str = tr("Please check your Internet connection");
        eqsl_upd_timer->start(5200); //5200
    }
    //}
}
//////// EQSL //////
void RadioAndNetW::StartStopQRZLog(bool f)
{
    if (f)
    {
        if (socet_tcp_qrzlog->state() != QAbstractSocket::ConnectedState)
        {
            int p1 = LeQRZLogPort->text().toInt();
            socet_tcp_qrzlog->connectToHostEncrypted(LeQRZLogServer->text(),p1);
            socet_tcp_qrzlog->waitForConnected(500);
        }
    }
    else if (socet_tcp_qrzlog->state() == QAbstractSocket::ConnectedState) socet_tcp_qrzlog->disconnectFromHost();
}
void RadioAndNetW::cb_qrzlog_toggled()
{
    RefreshUdpOrTcpBroadLoggedAll(); //StartStopQRZLog(cb_qrzlog->isChecked()); //no needed 15 sec time out and close
}
//QElapsedTimer ttt;
void RadioAndNetW::readQRZLog()
{
    //RESULT=OK&LOGIDS=130877825&COUNT=1
    //STATUS=AUTH&REASON=user does not have a valid QRZ subscription to use this function\n&EXTENDED=
    //STATUS=AUTH&REASON=invalid api key 0026A7F6F4E9287\n&EXTENDED=
    //REASON=Unable to add QSO to database: duplicate&EXTENDED=
    //REASON=missing &lt;eor&gt; in record&EXTENDED=
    QByteArray ba = socet_tcp_qrzlog->readAll();
    QString sba = QString(ba.data());
    int icb = sba.indexOf("\r\n\r\n");
    if (icb>-1) sba = sba.mid(icb+4,sba.count()-(icb+4));
    else sba = "";
    if (sba.contains("RESULT=OK")) qrz_err_flag = false;
    else if (sba.contains("REASON="))
    {
        if (sba.contains("duplicate")) qrz_err_flag = false;
        icb = sba.indexOf("REASON=");
        qrz_error_str = sba.mid(icb,sba.count()-icb);
        //int ice = sba.indexOf("\n",icb+7);
        //if (ice>icb) qrz_error_str = sba.mid(icb,ice-icb);
        //else qrz_error_str = "Unknown Error";
    }
    else qrz_error_str = "Unknown Error";
    if (!qrz_err_flag && qrz_is_act_send_check == 2)
    {
        //qDebug()<<(2500 - qrz_upd_timer->remainingTime()+1100+500);
        qrz_upd_timer->stop();
        qrz_upd_timer->start(1100);
    }
    //if (qrz_err_flag) qDebug()<<qrz_is_act_send_check<<"ERROR <-- "<<qrz_error_str<<sba;
    //else qDebug()<<qrz_is_act_send_check<<"QRZ <-- "<<sba;
}
void RadioAndNetW::CheckQRZLogError()
{
    if (qrz_once_data_err_msg && qrz_err_flag)
    {
        qrz_once_data_err_msg = false;
        QString s = " MSHV "+tr("Unsuccessful upload to")+" QRZ Logbook\n"+qrz_error_str;
        QPoint post = w_parent->mapToGlobal(QPoint(0,0));
        post+=QPoint(w_parent->width()/2-160,(-w_parent->pos().y()+50));
        QToolTip::showText(post,s,this,rect(),8000);
    }
}
QString qrzlog_adif_arr[MAXUPLOAUD+10];
void RadioAndNetW::UplQRZLogAdif()
{
	//static QString serv_qrzlog0 = "#";	
    if (qrz_is_act_send_check < 2)
    {
    	/*if (serv_qrzlog0 != LeQRZLogServer->text())//2.761 Connect/Disconnect on the fly
    	{
    		serv_qrzlog0 = LeQRZLogServer->text();
    		if (socet_tcp_qrzlog->state() == QAbstractSocket::ConnectedState)
    		{
    			socet_tcp_qrzlog->disconnectFromHost();
    			socet_tcp_qrzlog->waitForDisconnected(350);
   			}
   		}*/    	
        if (socet_tcp_qrzlog->state() != QAbstractSocket::ConnectedState) StartStopQRZLog(true);
        if (socet_tcp_qrzlog->state() == QAbstractSocket::ConnectedState)
        {
            qrz_err_flag = true;
            QString adiftrim = qrzlog_adif_arr[qrz_pos_ul_read];
            adiftrim.remove(adiftrim.size()-1,1);//remove last \n  //adiftrim = adiftrim.trimmed();
            QByteArray res = "POST /"+LeQRZLogPost->text().toUtf8()+" HTTP/1.1\r\n";
            res.append("Host: "+LeQRZLogServer->text().toUtf8()+"\r\n");
            QString tmp = "User-Agent: MSHV/"+QString(VER_MS)+"\r\n";
            res.append(tmp.toUtf8());
            res.append("Accept: */*\r\n");
            res.append("Content-Type: application/x-www-form-urlencoded\r\n");
            QString cnt = "key="+LeQRZLogApi->text();
            cnt.append("&action=insert");
            //cnt.append("&action=status");
            cnt.append("&adif="+URLEncode("<PROGRAMID:4>MSHV<EOH>"+adiftrim));
            tmp = "Content-Length: "+QString("%1").arg(cnt.count());
            res.append(tmp.toUtf8());
            tmp = "\r\n\r\n"+cnt;
            res.append(tmp.toUtf8());
            socet_tcp_qrzlog->write(res);
            qrz_is_act_send_check = 2; //qDebug()<<"ttt"<<ttt.elapsed(); ttt.start();
            qrz_upd_timer->start(2500);//2500 1500
            //qDebug()<<"MSHV Send-->";//<<res.data();
        }
        else
        {
            qrz_upd_timer->stop();
            qrz_err_flag = true;
            CheckQRZLogError();
            qrz_pos_ul_writ = 0;
            qrz_pos_ul_read = 0;
            qrz_is_act_send_check = 0;
        }
    }
    else //check
    {
        qrz_upd_timer->stop();
        CheckQRZLogError();
        qrz_pos_ul_read++;
        if (qrz_pos_ul_read >= qrz_pos_ul_writ)
        {
            qrz_pos_ul_writ = 0;
            qrz_pos_ul_read = 0;
            qrz_is_act_send_check = 0;
        }
        else
        {
            qrz_is_act_send_check = 1;//1=next send data
            qrz_upd_timer->start(500);
        }
    }
}
void RadioAndNetW::SendQRZLogAdif(QString s)
{
    //for (int i = 0; i<3; i++)
    //{
    if (LeQRZLogApi->text().isEmpty()) return;
    if (qrz_pos_ul_writ > MAXUPLOAUD) return;
    qrzlog_adif_arr[qrz_pos_ul_writ] = s;
    qrz_pos_ul_writ++;
    if (qrz_is_act_send_check == 0)//0=no_active
    {
        qrz_once_data_err_msg = true;
        qrz_is_act_send_check = 1;//1=send data
        qrz_error_str = tr("Please check your Internet connection");
        qrz_upd_timer->start(4900); //4900
    }
    //}
}
void RadioAndNetW::StartStopClubLog(bool f)
{
    if (f)
    {
        if (socet_tcp_clublog->state() != QAbstractSocket::ConnectedState)
        {
            int p1 = LeClubLogPort->text().toInt();
            socet_tcp_clublog->connectToHostEncrypted(LeClubLogServer->text(),p1); //connectToHost(LeClubLogServer->text(),p1);//8080 443
            socet_tcp_clublog->waitForConnected(400);
        }
    }
    else if (socet_tcp_clublog->state() == QAbstractSocket::ConnectedState) socet_tcp_clublog->disconnectFromHost();
}
void RadioAndNetW::cb_clublog_toggled()
{
    RefreshUdpOrTcpBroadLoggedAll(); //StartStopClubLog(f); //no needed 1 min time out and close
}
void RadioAndNetW::readClubLog()
{
    QByteArray ba = socet_tcp_clublog->readAll();
    QString sba = QString(ba.data());
    int ice = -1;
    int icb = sba.indexOf("\r\n\r\n");
    if (icb>-1) icb = sba.indexOf("\r\n",icb+4);
    if (icb>-1) ice = sba.indexOf("\r\n",icb+2);
    if (icb>-1 && ice>icb) sba = sba.mid((icb+2),ice-(icb+2));
    else sba = "";
    /*//other read method
    QString sbb = sba.mid(ic+4,sba.count()-(ic+4));
    QStringList l = sbb.split("\r\n"); 
    l << "0" << "ERROR";
    QString s0 = l.at(0);
    QString s1 = l.at(1);
    if (s0=="OK"   || s1=="OK") 
    if (s0=="Dupe" || s1=="Dupe") 
    if (s0=="Updated QSO" || s1=="Updated QSO")*/
    //"mshvlog.adi (242 bytes) => LZ2HV : Upload accepted and queued!"
    if (sba=="OK" || sba=="Dupe" || sba=="Updated QSO")	cl_err_flag = false;
    if (sba.contains("Upload accepted and queued"))	cl_err_flag = false;
    if (!cl_err_flag && cl_is_act_send_check == 4)//0=no_active 1=send data 2=send file 3=check data 4=check file
    {
        cl_upd_timer->stop();
        cl_upd_timer->start(200);
    }
    if (!cl_err_flag && cl_is_act_send_check == 3)
    {
        cl_upd_timer->stop();
        cl_upd_timer->start(1100);
    }
    //qDebug()<<"READ=BAR="<<ba.data();
    //qDebug()<<"READ=SBA="<<sba<<icb+2<<ice-(icb+2);
}
QString clublog_adif_arr[MAXUPLOAUD+10];
void RadioAndNetW::CheckClubLogError(int b,int e)
{
    if (cl_err_flag && (cl_is_act_send_check == 1 || cl_is_act_send_check == 3))
    {   //0=no_active 1=send data 2=send file 3=check data 4=check file
        QFile file(file_cl_error);
        if (!file.open(QIODevice::Text | QIODevice::Append)) return;
        QTextStream out(&file);
        QString verr  = (QString)VER_MS;
        QString headd = (QString)APP_NAME+" ADIF Export\n<ADIF_VER:5>3.1.0\n<PROGRAMID:4>MSHV\n"
                        "<PROGRAMVERSION:"+QString("%1").arg(verr.count())+">"+verr+"\n<EOH>\n";
        //if (file.size()>8192)
        if (file.size()>5333450) //2666725~10000-QSOs 5333450~20000-QSOs 10666900~40000-QSOs, 21333800~80000-QSOs
        {
            file.close();
            QFile filer(file_cl_error);
            if (!filer.open(QIODevice::ReadOnly | QIODevice::Text)) return; //ReadWrite
            QTextStream in(&filer);
            QStringList lst0;
            while (!in.atEnd()) lst0 << in.readLine();
            filer.close();
            QFile filew(file_cl_error);
            if (!filew.open(QIODevice::WriteOnly | QIODevice::Text)) return;
            QTextStream outw(&filew);
            outw << headd;// 5=count head
            for (int z = 5+((lst0.count()-5)/4); z < lst0.count(); ++z) outw << lst0.at(z) << "\n";
            for (int x=b;  x <= e; ++x) outw << clublog_adif_arr[x];
            //qDebug()<<"Save="<<b<<e<<"WriteOnly";
            filew.close();
            return;
        }
        if (file.size()==0) out << headd;
        for (int j=b;  j <=e; ++j) out << clublog_adif_arr[j];
        //qDebug()<<"Save="<<b<<e<<file.size()<<"Append";
        file.close();
    }

    if (cl_once_data_err_msg || cl_is_act_send_check == 2 || cl_is_act_send_check == 4)
    {	//0=no_active 1=send data 2=send file 3=check data 4=check file
        cl_once_data_err_msg = false;
        if (cl_err_flag)
        {
            QString s = " MSHV "+tr("Unsuccessful upload to")+" Club Log\n"+tr("Please check your Internet connection");
            if (cl_is_act_send_check == 1 || cl_is_act_send_check == 3)
            {
                s.append(" "+tr("or stop uploading to")+" Club Log");
                s.append("\n"+tr("Unposted QSOs are in the MSHV Log directori and the filename is")+
                         ":\n    mshvlog_clublog_error.adi");
                QPoint post = w_parent->mapToGlobal(QPoint(0,0));
                post+=QPoint(w_parent->width()/2-160,(-w_parent->pos().y()+50));
                QToolTip::showText(post,s,this,rect(),10000);
            }
            if (cl_is_act_send_check == 2 || cl_is_act_send_check == 4) emit EmitUploadClubLogInfo(s.trimmed());
        }//"mshvlog.adi (242 bytes) => LZ2HV : Upload accepted and queued!"
        else if (cl_is_act_send_check == 4) emit EmitUploadClubLogInfo("MSHV "+tr("Successful upload")+
                    "\nClub Log: "+LClubLogCall->text()+" "+tr("Upload accepted and queued!"));
    }
}
static QByteArray cl_upload_file_;
void RadioAndNetW::UplClubLogAdif()
{
	//static QString serv_clublog0 = "#";	
    if (cl_is_act_send_check < 3)
    {	//0=no_active 1=send data 2=send file 3=check data 4=check file
    	/*if (serv_clublog0 != LeClubLogServer->text())//2.761 Connect/Disconnect on the fly
    	{
    		serv_clublog0 = LeClubLogServer->text();
    		if (socet_tcp_clublog->state() == QAbstractSocket::ConnectedState)
    		{
    			socet_tcp_clublog->disconnectFromHost();
    			socet_tcp_clublog->waitForDisconnected(350);
   			}
   		}*/    	
        if (socet_tcp_clublog->state() != QAbstractSocket::ConnectedState) StartStopClubLog(true);
        if (socet_tcp_clublog->state() == QAbstractSocket::ConnectedState)
        {
            cl_err_flag = true;
            if (!cl_send_file)
            {   //as data to ClubLog 1 by 1 QSO
                QString adiftrim = clublog_adif_arr[cl_pos_ul_read];
                adiftrim.remove(adiftrim.size()-1,1);//remove last \n  //adiftrim = adiftrim.trimmed();
                QByteArray res = "POST /"+LeClubLogPost0->text().toUtf8()+" HTTP/1.1\r\n";
                res.append("Host: "+LeClubLogServer->text().toUtf8()+"\r\n");
                QString tmp = "User-Agent: MSHV/"+QString(VER_MS)+"\r\n";
                res.append(tmp.toUtf8());
                res.append("Accept: */*\r\n");
                res.append("Content-Type: application/x-www-form-urlencoded\r\n");
                //res.append("Connection: keep-alive\r\n");
                /*QUrlQuery params; //problem ":" -> %3A and +-
                params.addQueryItem("adif","<PROGRAMID:4>MSHV<EOH>"+adiftrim);
                QString cnt = params.toString(QUrl::FullyEncoded);  qDebug()<<cnt;*/
                //QString cnt = "adif="+QUrl::toPercentEncoding("<PROGRAMID:4>MSHV<EOH>"+adiftrim,"","\"<>%\\^[]`+$,@:;/!#?=&");
                //QByteArray text = QByteArray::fromPercentEncoding(cnt.toUtf8());
                QString cnt = "adif="+URLEncode("<PROGRAMID:4>MSHV<EOH>"+adiftrim);
                cnt.append("&email="+LeClubLogMail->text());
                cnt.append("&callsign="+LClubLogCall->text());
                cnt.append("&password="+LeClubLogPass->text());
                cnt.append("&api="+_kkwe2_);
                tmp = "Content-Length: "+QString("%1").arg(cnt.count());
                res.append(tmp.toUtf8());
                tmp = "\r\n\r\n"+cnt;
                res.append(tmp.toUtf8());
                socet_tcp_clublog->write(res);
                cl_is_act_send_check = 3; //qDebug()<<"ttt"<<ttt.elapsed(); ttt.start(); //0=no_active 1=send data 2=send file  3=check data 4=check file
                cl_upd_timer->start(2500);//2500 1500
                /*int ic = adiftrim.indexOf("CALL:");
                int cc = adiftrim.mid(ic+5,1).toInt();
                qDebug()<<"----------START DATA-->"<<adiftrim.mid(ic+7,cc);*/
                //qDebug()<<res.data();
            }
            else
            {	//as file to ClubLog many QSOs
                int x = rand() % 100000 + 1;
                QString tmpf = QString("%1").arg(x,5,16,QChar('0'));
                QByteArray bound = "--MSHV-"+tmpf.toUtf8();
                QByteArray resf = "POST /"+LeClubLogPost1->text().toUtf8()+" HTTP/1.1\r\n";
                resf.append("Host: "+LeClubLogServer->text().toUtf8()+"\r\n");
                tmpf = "User-Agent: MSHV/"+QString(VER_MS)+"\r\n";
                resf.append(tmpf.toUtf8());
                resf.append("Accept: */*\r\n");
                tmpf = QDateTime::currentDateTimeUtc().toString("yyyyMMddhhmm");
                QByteArray cntf = "--"+bound+"\r\n";
                cntf.append("Content-Disposition: form-data; name=\"email\"\r\n\r\n");
                cntf.append(LeClubLogMail->text().toUtf8()+"\r\n");
                cntf.append("--"+bound+"\r\n");
                cntf.append("Content-Disposition: form-data; name=\"password\"\r\n\r\n");
                cntf.append(LeClubLogPass->text().toUtf8()+"\r\n");
                cntf.append("--"+bound+"\r\n");
                cntf.append("Content-Disposition: form-data; name=\"callsign\"\r\n\r\n");
                cntf.append(LClubLogCall->text().toUtf8()+"\r\n");
                cntf.append("--"+bound+"\r\n");
                cntf.append("Content-Disposition: form-data; name=\"api\"\r\n\r\n");
                cntf.append(_kkwe2_.toUtf8()+"\r\n");
                cntf.append("--"+bound+"\r\n");
                cntf.append("Content-Disposition: form-data; name=\"file\"; filename=\"mshvlog"+tmpf.toUtf8()+".adi\"\r\n");
                cntf.append("Content-Type: application/octet-stream\r\n\r\n");
                cntf.append("<PROGRAMID:4>MSHV<EOH>\r\n"+cl_upload_file_+"\r\n\r\n");
                cntf.append("--"+bound+"--");
                tmpf = "Content-Length: "+QString("%1").arg(cntf.count());
                resf.append(tmpf.toUtf8()+"\r\n");
                resf.append("Content-Type: multipart/form-data; boundary="+bound);
                resf.append("\r\n\r\n"+cntf);
                socet_tcp_clublog->write(resf);
                cl_is_act_send_check = 4; //0=no_active 1=send data 2=send file  3=check data 4=check file
                cl_upd_timer->start(10000);//15000 max wait 10s
                emit EmitUploadClubLogInfo("-End-");
                /*int ic = cl_upload_file_.indexOf("CALL:");
                int cc = cl_upload_file_.mid(ic+5,1).toInt();
                qDebug()<<"--START FILE-->"<<cl_upload_file_.mid(ic+7,cc);*/
                //qDebug()<<resf.data();
            }
        }
        else
        {
            cl_upd_timer->stop(); //qDebug()<<"ErrorConnection"<<cl_is_act_send_check<<cl_send_file;
            cl_err_flag = true; //shure error
            CheckClubLogError(cl_pos_ul_read,cl_pos_ul_writ);
            if (cl_is_act_send_check == 1 && cl_send_file)
            {
                cl_pos_ul_writ = 0;
                cl_pos_ul_read = 0;
                cl_is_act_send_check = 2;//2=try send file
                cl_upd_timer->start(1000);
            }
            else if (cl_is_act_send_check == 2 && cl_pos_ul_read < cl_pos_ul_writ)
            {
                cl_once_data_err_msg = true;
                cl_upload_file_.clear();
                cl_send_file = false;
                cl_is_act_send_check = 1;//1=try send data
                cl_upd_timer->start(1000);
            }
            else
            {
                cl_pos_ul_writ = 0;
                cl_pos_ul_read = 0;
                cl_is_act_send_check = 0;//0=no_active
                cl_upload_file_.clear();
                cl_send_file = false;
            }
        }
    }
    else//3=check data 4=check file
    {
        cl_upd_timer->stop(); //qDebug()<<"Check 3 4";
        CheckClubLogError(cl_pos_ul_read,cl_pos_ul_read);
        if (cl_is_act_send_check == 3)//3=check data
        {	//0=no_active 1=send data 2=send file 3=check data 4=check file
            cl_pos_ul_read++;
            if (cl_pos_ul_read >= cl_pos_ul_writ)
            {
                cl_pos_ul_writ = 0;
                cl_pos_ul_read = 0;
                cl_is_act_send_check = 0;
            }
            else cl_is_act_send_check = 1;//1=next send data
            if (cl_send_file) cl_is_act_send_check = 2;//2=try send file
        }
        else//4=check file
        {
            cl_upload_file_.clear();
            cl_send_file = false;
            cl_is_act_send_check = 0;
            if (cl_pos_ul_read < cl_pos_ul_writ)
            {
                cl_once_data_err_msg = true;
                cl_is_act_send_check = 1;//1=try send data
            }
        }
        if (cl_is_act_send_check != 0) cl_upd_timer->start(500);
    }
}
void RadioAndNetW::SetUploadSelected(QByteArray ba)
{
    if (LeClubLogMail->text().isEmpty() || LClubLogCall->text().isEmpty() ||
            LClubLogCall->text()=="CALL" || LeClubLogPass->text().isEmpty())
    {
        emit EmitUploadClubLogInfo("Club Log\n"+tr("Missing: E-Mail or Password or Callsign")+"\n"+
                                   tr("Go to Options, Network Configuration and correct it"));
        return;
    }
    if (cl_send_file) return; //one by one
    cl_upload_file_.clear();
    cl_upload_file_ = ba;
    cl_send_file = true;
    if (cl_is_act_send_check==0)//0=no_active
    {
        cl_once_data_err_msg = true;
        cl_is_act_send_check = 2;//2=send file
        cl_upd_timer->start(1000);//1000
    } //qDebug()<<QString::fromStdString(cl_upload_file_.toStdString());
}
void RadioAndNetW::SendClubLogAdif(QString s)
{
    //for (int i = 0; i<5; i++)
    //{
    if (LeClubLogMail->text().isEmpty() || LClubLogCall->text().isEmpty() ||
            LClubLogCall->text()=="CALL" || LeClubLogPass->text().isEmpty()) return;
    if (cl_pos_ul_writ > MAXUPLOAUD) return;
    clublog_adif_arr[cl_pos_ul_writ] = s;
    cl_pos_ul_writ++;
    if (cl_is_act_send_check==0)//0=no_active
    {
        cl_once_data_err_msg = true;
        cl_is_act_send_check = 1;//1=send data
        cl_upd_timer->start(4600);
    }
    //}
}
void RadioAndNetW::StartStopTCPBroad(bool)
{
    RefreshUdpOrTcpBroadLoggedAll();
}
void RadioAndNetW::networkErrorUDPBrodcast(QString const& s)
{
    QString infoc = "Error: "+s+"\nUDP server  "+UDPServerBroad->text()+" port "+UDPPortBroad->text();
    QMessageBox::warning(w_parent, "MSHV UDP Brodcast Error",(infoc),QMessageBox::Close);
}
void RadioAndNetW::SetDefaultFreqsActType(int id)
{
    // Activity type                id	array
    //"Standard"					0	all_bands_mods_frq
    //"EU RSQ And Serial Number"	1	all_bands_mods_frq
    //"NA VHF Contest"				2	all_bands_mods_frq
    //"EU VHF Contest"				3 	all_bands_mods_frq
    //"ARRL Field Day"				4	all_bands_mods_frq
    //old "RTTY Roundup"			5   FT4: (3.580, 7.080, 14.080, 21.080, 28.080)
    //				  				    FT8: (3.590, 7.090, 14.130, 21.130, 28.160)
    //"ARRL Inter. Digital Contest"	5	FT4: (1.836, 3.580, 7.080, 14.080, 21.080, 28.080, 50.330)
    //									FT8: (1.840, 3.590, 7.090, 14.090, 21.090, 28.090, 50.340)
    //"WW Digi DX Contest"			6	FT4: (1.840, 3.580, 7.080, 14.080, 21.080, 28.080)
    //									FT8: (1.844, 3.590, 7.090, 14.090, 21.090, 28.090)
    //"FT4 DX Contest"				7	FT4: (3.580, 7.080, 14.080, 21.080, 28.080)
    //"FT8 DX Contest"				8	FT8: (3.590, 7.090, 14.090, 21.090, 28.090)
    //"FT Roundup Contest"			9	FT4: (3.580, 7.080, 14.080, 21.080, 28.080)
    //									FT8: (3.590, 7.090, 14.090, 21.090, 28.090)
    //"Bucuresti Digital Contest"	10	FT4: (3.575.000, 7.047.500)
    //"FT4 SPRINT Fast Training"    11  FT4: (7.043, 14.076)
    //"PRO DIGI Contest"			12  FT4: (3.576, 7.080, 14.080, 21.080, 28.080)
    //"CQ WW VHF Contest"			13	all_bands_mods_frq
    //"Q65 Pileup" or "Pileup"		14	all_bands_mods_frq
    //"NCCC Sprint"					15	all standard (1.840, 3.575, 7.047, 14.080, 21.140, 28.180, 50.318)
    //"ARRL Inter. EME Contest"		16	all_bands_mods_frq
    //FT Challenge Contest			17  FT4: (3.580, 7.080, 14.080, 21.080, 28.080)
    //									FT8: (3.590, 7.090, 14.090, 21.090, 28.090)

    const QString aidc_mods_frq[2][7] =
        {
            //{"3.580.000","7.080.000","14.080.000","21.080.000","28.080.000"},
            //{"3.590.000","7.090.000","14.130.000","21.130.000","28.160.000"},
            {"1.836.000","3.580.000","7.080.000","14.080.000","21.080.000","28.080.000","50.330.000"},
            {"1.840.000","3.590.000","7.090.000","14.090.000","21.090.000","28.090.000","50.340.000"},
        };
    const QString ww_mods_frq[2][6] =
        {   
        	//no move
            {"1.840.000","3.580.000","7.080.000","14.080.000","21.080.000","28.080.000"},
            {"1.844.000","3.590.000","7.090.000","14.090.000","21.090.000","28.090.000"},
        };
    /*const QString ft4dx_mods_frq[5] =
       {
           "3.580.000","7.080.000","14.080.000","21.080.000","28.080.000"
       };
    const QString ft8dx_mods_frq[5] =
       {
           "3.590.000","7.090.000","14.090.000","21.090.000","28.090.000" //2.64
       };*/
    const QString ftru_mods_frq[2][5] = //same as ft4dx, ft8dx
        {   
        	//no move
            {"3.580.000","7.080.000","14.080.000","21.080.000","28.080.000"},
            {"3.590.000","7.090.000","14.090.000","21.090.000","28.090.000"},
        };
    const QString bu_mods_frq[2] =
        {
            "3.575.000","7.047.500"
        };
    const QString ft4sft_mods_frq[2] =
        {
            "7.043.000","14.076.000"
        };
    const QString ft4pdc_mods_frq[5] =
        {
            "3.576.000","7.080.000","14.080.000","21.080.000","28.080.000"
        };

    QStringList list;
    QString sfrq;
    QStringList lf;
    QString ft4_s = "";
    QString ft8_s = "";

    int col = 4;//start from 3.5
    if (id == 6 || id == 5) col = 3;//start from 1.8
    if (id == 11) col = 6;//start from 7.0

	//"MSK","FSK","FT2","FT4","FT8", "J65","Q65" <-- real pos in RadList
    int modp = pos_mod_sav_frq[2];//3 start from ft4
    if (id == 8) modp = pos_mod_sav_frq[3];//4 //start from ft8
    
    int coar = 5;//all others
    if (id == 5)  coar = 7;//"ARRL Inter. Digital Contest"
    if (id == 6)  coar = 6;//"WW Digi DX Contest"
    if (id == 10) coar = 2;//"Bucuresti Digital Contest"
    if (id == 11) coar = 2;//"FT4 SPRINT Fast Training"

    //qDebug()<<s_cont_name[id]<<"band="<<col<<"modp="<<modp<<"coar="<<coar;
    for (int i  = 0; i < coar; ++i)
    {
        sfrq = THvRadList->model.item(col, 3)->text();
        lf = sfrq.split("\n");

        if (id == 5) 	  //"ARRL Inter. Digital Contest"  //"RTTY Roundup"
        {
            lf.replace(modp,  aidc_mods_frq[0][i]);
            lf.replace(modp+1,aidc_mods_frq[1][i]);
            ft4_s.append(aidc_mods_frq[0][i]+(QString)", ");
            ft8_s.append(aidc_mods_frq[1][i]+(QString)", ");
        }
        else if (id == 6) //"WW Digi DX Contest"
        {
            lf.replace(modp,  ww_mods_frq[0][i]);
            lf.replace(modp+1,ww_mods_frq[1][i]);
            ft4_s.append(ww_mods_frq[0][i]+", ");
            ft8_s.append(ww_mods_frq[1][i]+", ");
        }
        else if (id == 7) //"FT4 DX Contest" same as "FT Roundup Contest"
        {
            /*lf.replace(modp,  ft4dx_mods_frq[i]);
            ft4_s.append(ft4dx_mods_frq[i]+", ");*/
            lf.replace(modp,  ftru_mods_frq[0][i]);
            ft4_s.append(ftru_mods_frq[0][i]+", ");
        }
        else if (id == 8) //"FT8 DX Contest" same as "FT Roundup Contest"
        {
            /*lf.replace(modp,  ft8dx_mods_frq[i]);
            ft8_s.append(ft8dx_mods_frq[i]+", ");*/
            lf.replace(modp,  ftru_mods_frq[1][i]);
            ft8_s.append(ftru_mods_frq[1][i]+", ");
        }
        else if (id == 9 || id == 17) //"FT Roundup Contest" and "FT Challenge"
        {
            lf.replace(modp,  ftru_mods_frq[0][i]);
            lf.replace(modp+1,ftru_mods_frq[1][i]);
            ft4_s.append(ftru_mods_frq[0][i]+", ");
            ft8_s.append(ftru_mods_frq[1][i]+", ");
        }
        else if (id == 10)//"Bucuresti Digital Contest"
        {
            lf.replace(modp,  bu_mods_frq[i]);
            ft4_s.append(bu_mods_frq[i]+", ");
        }
        else if (id == 11)//"FT4 SPRINT Fast Training"
        {
            lf.replace(modp,  ft4sft_mods_frq[i]);
            ft4_s.append(ft4sft_mods_frq[i]+", ");
        }
        else if (id == 12) //"PRO DIGI Contest"
        {
            lf.replace(modp,  ft4pdc_mods_frq[i]);
            ft4_s.append(ft4pdc_mods_frq[i]+", ");
        }

        sfrq.clear();
        for (int j  = 0; j < COUNT_FREQ_MODES; j++)
        {
            sfrq.append(lf.at(j));
            if (j<COUNT_FREQ_MODES-1) sfrq.append("\n");
        }
        list << lst_bands[col]<<THvRadList->model.item(col, 1)->text()<<sfrq;
        THvRadList->SetItem_hv(list,col);  //qDebug()<<list;
        list.clear();

        if (id == 5) //"ARRL Inter. Digital Contest"
        {
            if   	(i==0) col=4;
            else if (i==1) col=6;
            else if (i==2) col=8;
            else if (i==3) col=10;
            else if (i==4) col=13;//2.76.5
            else if (i==5) col=15;//2.76.5
        }
        else if (id == 6) //"WW Digi DX Contest"
        {
            if   	(i==0) col=4;
            else if (i==1) col=6;
            else if (i==2) col=8;
            else if (i==3) col=10;
            else if (i==4) col=13;//2.76.5
        }
        else if (id == 11)//"FT4 SPRINT Fast Training"
        {
            if   	(i==0) col=8;
            //else if (i==1) col=9;
        }
        else //All others
        {
            if   	(i==0) col=6;
            else if (i==1) col=8;
            else if (i==2) col=10;
            else if (i==3) col=13;//2.76.5
        }
    }

    QString tsts = "<p align='center'>"+tr("Default frequencies have been changed for the")+
                   "<strong> - "+s_cont_name[id]+" -</strong></p>";
    if (!ft4_s.isEmpty())
    {
        ft4_s = ft4_s.mid(0,ft4_s.count()-2);//remove last ", "
        tsts.append("<div>FT4: ( "+ft4_s+" )</div>");
    }
    if (!ft8_s.isEmpty())
    {
        ft8_s = ft8_s.mid(0,ft8_s.count()-2);//remove last ", "
        tsts.append("<div>FT8: ( "+ft8_s+" )</div>");
    }
    QMessageBox::information(this,"MSHV",tsts,QMessageBox::Close);
}
void RadioAndNetW::SetDefaultFreqsActTypeBut()
{
    //if (s_cont_id > 4)
    SetDefaultFreqsActType(s_cont_id);
}
void RadioAndNetW::ResetDefaultFreqsBut()
{
    SetDefaultFreqs(false);
}
//static const QString all_bands_mods_frq5[5][COUNT_BANDS][COUNT_FREQ_MODES];
void RadioAndNetW::SetDefaultFreqs(bool f)
{
    for (int i  = 0; i < COUNT_BANDS; i++)
    {
        QStringList list;
        QString sfrq;
        for (int j  = 0; j < COUNT_FREQ_MODES; j++)
        {
            sfrq.append(all_bands_mods_frq[i][pos_mod_rea_frq[j]]);
            if (j<COUNT_FREQ_MODES-1) sfrq.append("\n");
        }
        if (f)//true all is default
        {
            list << lst_bands[i]<<"Dipole"<<sfrq;
            THvRadList->InsertItem_hv(list);
        }
        else //false reset
        {
            list << lst_bands[i]<<THvRadList->model.item(i, 1)->text()<<sfrq;
            THvRadList->SetItem_hv(list,i);
        }
    }
    if (!f)
    {
        QMessageBox::information(this,"MSHV","<p align='center'>"+tr("Default frequencies have been changed to")+
                                 "<strong> - Standard -</strong></p>",QMessageBox::Close);
    }
}
void RadioAndNetW::SetFont(QFont f)
{
    QFont f_t = f;
    f_t.setPointSize(f.pointSize()+1);//10
    l_con_info->setFont(f_t);
    l_tcpcon_info->setFont(f_t);
    TSpotDialog->SetFont(f);
    LClubLogCall->setFont(f);
}
void RadioAndNetW::SetLocFromDB(QString loc)
{
    TSpotDialog->SetLocFromDB(loc);
}
/*void RadioAndNetW::OpenRadNetWToRecon()
{
    //TSpotDialog->close();
    //TSpotDialog->setFocus(Qt::MouseFocusReason);
    this->exec();
    //this->show();
}*/
void RadioAndNetW::ReadTelnetList(QString path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    QStringList list_t;
    QTextStream in(&file);
    while (!in.atEnd())
    {
        QString line = in.readLine();
        if (!line.isEmpty())
            list_t<<line;
    }
    Cb_telnet->addItems(list_t);
    file.close();
}
void RadioAndNetW::CbTelnetChanged(QString s)
{
    QStringList list_t = s.split(",");
    if (list_t.count()==2)
    {
        QStringList list_a = list_t[1].split(":");
        if (list_a.count()==2)
        {
            TCPServer->setText(list_a[0]);
            TCPPort->setText(list_a[1]);
        }
    }//TCPServer->text(),TCPPort->text()
}
void RadioAndNetW::SetGlobalTcpStat(QString val)
{
    if (val.toInt()==1)
        global_tcp_stat = true;
    else
        global_tcp_stat = false;
}
void RadioAndNetW::ConDiscon()
{
    //TCPServPortChanged("d");
    if (tcp_conect_stat)
    {
        global_tcp_stat = false;
        tcp_conect_stat = false;
    }
    else
    {
        global_tcp_stat = true;
        tcp_conect_stat = true;
    }

    if (tcp_conect_stat)
        TCPServPortChanged("d");
    else
        TelnetClient->DisconectOnly();
}
void RadioAndNetW::TCPServPortChanged(QString)
{
    if (!s_myCall.isEmpty() && global_tcp_stat)
    {
        TelnetClient->Connect(TCPServer->text(),TCPPort->text(),s_myCall,TCPPass->text());
        //qDebug()<<"RadioAndNetW::TCPServPortChanged";
    }
}
void RadioAndNetW::ShowBcnList()
{
    THvBcnListW->show();
}
void RadioAndNetW::CloseBcnList()
{
    if (THvBcnListW->isVisible())
        THvBcnListW->close();
}
void RadioAndNetW::SetConectionTcpInfo(QString s)
{
    l_tcpcon_info->setText(tr("Status")+": "+s);
    TSpotDialog->SetTcpconInfo(tr("Status")+": "+s);

    if (s.contains(tr("Disconnected")))
    {
        pb_tcp_conect->setText(tr("Press To Connect"));
        tcp_conect_stat = false;
    }
    else
    {
        pb_tcp_conect->setText(tr("Press To Disconnect"));
        tcp_conect_stat = true;
    }
}
void RadioAndNetW::SetConectionInfo(QString s)
{
    if (cb_start_stop_psk_rpt->isChecked())
        l_con_info->setText(tr("Status")+": "+s);
    else
        l_con_info->setText(tr("Status")+": <font color='red'>"+tr("PSK Reporter Is Disabled And Disconnected")+"</font>");
    //l_con_info->setText("Status: PSK Reporter Is Disabled");// "+s
}
void RadioAndNetW::Reconnect()
{
    psk_Reporter->stop();
    l_con_info->setText(tr("Status: Reconnecting..."));
    psk_Reporter->reconnect();
}
void RadioAndNetW::ServTextChanged(QString s)
{
    psk_Reporter->set_server(s);
}
void RadioAndNetW::StartStopReport(bool f)
{
    if (f)
    {
        l_con_info->setText(tr("Status: Connecting..."));
        pb_re_conect->setEnabled(true);
        cb_udp_tcp_psk_rpt->setEnabled(true);
        ServTextChanged(UDPServer->text());
        Reconnect();
    }
    else
    {
        l_con_info->setText(tr("Status")+": <font color='red'>"+tr("PSK Reporter Is Disabled And Disconnected")+"</font>");
        pb_re_conect->setEnabled(false);
        cb_udp_tcp_psk_rpt->setEnabled(false);
        ServTextChanged("_none_");
        psk_Reporter->stop();
    }
}
void RadioAndNetW::PortTextChanged(QString s)
{
    psk_Reporter->set_server_port(s.toInt());
}
void RadioAndNetW::StInfoChanged(QStringList l, int i)
{
    THvRadList->SetItem_hv(l,i);
    SetLocalStation(s_myCall,s_myLoc,s_band,s_cont_id);
    //emit EmitRefreshFreqGlobal(); //1.61=
}
void RadioAndNetW::UdpTcpChangedPsk(bool f)
{
    //l_con_info->setText("Status: Connecting...");
    psk_Reporter->set_udp_tcp(f);
    if (cb_start_stop_psk_rpt->isChecked())
    {
        //l_con_info->setText("Status: Connecting...");
        Reconnect();// for read settings
    }
}
void RadioAndNetW::SetFullRigInfo(QString rigInfo)//2.76.1 for pskreporter
{
	static QString prev_rigInfo = "#";
	s_rigInfo = rigInfo; //qDebug()<<"IN="<<rigInfo;
    if (rigInfo == "None" || rigInfo.isEmpty()) s_rigInfo = "No CAT control"; 
    if (prev_rigInfo == s_rigInfo) return;
    prev_rigInfo = s_rigInfo;
    SetLocalStation(s_myCall,s_myLoc,s_band,s_cont_id); //qDebug()<<"PSKrep OUT="<<s_rigInfo;
}
void RadioAndNetW::SetLocalStation(QString myCall,QString myLoc,QString band,int con_id)
{
    if (s_myCall!=myCall)//1.76
    {
        s_myCall = myCall;
        TCPServPortChanged("d");// for my call if change
    } //qDebug()<<s_myCall<<myCall<<band;
    s_cont_id = con_id;
    s_band = band;
    s_myCall = myCall;
    s_myLoc = myLoc;
    LClubLogCall->setText(myCall);
    QString myAnt;
    for (int z=0; z<COUNT_BANDS; z++)
    {
        if (s_band == lst_bands[z])
        {
            myAnt=THvRadList->model.item(z, 1)->text();
            break;
        }
    }
    //psk_Reporter->setLocalStation(myCall,myLoc,myAnt,QString{"MSHV v" + QString(VER_MS)}.simplified ());   
    psk_Reporter->setLocalStation(myCall,myLoc,myAnt,s_rigInfo);
    if ((s_cont_id > 4 && s_cont_id < 13) || s_cont_id == 17)//=COUNT_CONTEST protection  13 = CQ WW VHF Contest no default freq
    {
        b_reset_default_freqs_cont->setEnabled(true);
        b_reset_default_freqs_cont->setText(tr("Default FREQs For")+" "+s_cont_name[s_cont_id]);
    }
    else
    {
        b_reset_default_freqs_cont->setEnabled(false);
        b_reset_default_freqs_cont->setText(tr("N/A For This Activity Type"));
    }
}
void RadioAndNetW::SetDistUnit(bool f_)
{
    TSpotDialog->SetDistUnit(f_);//f_km_mi = f_;
    //CalcDistance();
}
void RadioAndNetW::FindFreqLocFromBcnList(QString hisCall,int id,QString &hisLoc,QString &sfrq)
{
    int band_n=0;
    for (band_n=0; band_n<COUNT_BANDS; band_n++)
    {
        if (s_band == lst_bands[band_n])
            break;
    }

    if (band_n>=COUNT_BANDS) return;

    for (int z=0; z<THvBcnListW->THvBcnList->model.rowCount(); z++)
    {
        if (THvBcnListW->THvBcnList->model.item(z, 1)->text()==lst_bcnband[band_n])
        {
            if (THvBcnListW->THvBcnList->model.item(z, 2)->text()==hisCall)
            {
                if (id==0)//id 0->to psk reporter 1->to dx clusters spot 10->empty to dx clusters spot
                    sfrq = THvBcnListW->THvBcnList->model.item(z, 3)->text()+"000";
                if (id==1)
                    sfrq = THvBcnListW->THvBcnList->model.item(z, 3)->text();//+"000";

                sfrq.remove(".");
                //long long int frq0 = sfrq.toLongLong();//2.31 remove first zero 0.455.000
                //sfrq = QString("%1").arg(frq0);
                hisLoc = THvBcnListW->THvBcnList->model.item(z, 4)->text();
                break;
            }
        }
    } //qDebug()<<"hisLoc="<<hisLoc<<sfrq;
}
void RadioAndNetW::SetModeForFreqFromMode(int i)
{
    QString s = ModeStr(i);
    s_smode = "";//QString {};//"0";
    if (s=="FT4") // submode = m_nSubMode + 65;
    {
        s_mode = s;//"MFSK";  //??????
        //s_smode = s;        //??????
    }
    else if (s=="FT2") // submode = m_nSubMode + 65;
    {
        s_mode = s;//"MFSK";  //??????
        //s_smode = s;        //??????
    }
    else if (s=="ISCAT-A" || s=="ISCAT-B")
    {
        s_mode = "ISCAT";
        if (s=="ISCAT-A") s_smode = "A";//QString("%1").arg(65);      
        else s_smode = "B";//QString("%1").arg(66);            
    }
    else if (s=="JT65A" || s=="JT65B" || s=="JT65C")
    {
        s_mode = "JT65";
        if (s=="JT65A") s_smode = "A";//QString("%1").arg(65);            
        else if (s=="JT65B") s_smode = "B";//QString("%1").arg(66);   
        else s_smode = "C";//QString("%1").arg(67);            
    }
    else if (s=="Q65A" || s=="Q65B" || s=="Q65C" || s=="Q65D")
    {
        s_mode = "Q65";
        if (s=="Q65A") s_smode = "A";  
        else if (s=="Q65B") s_smode = "B";  
        else if (s=="Q65C") s_smode = "C";   
        else s_smode = "D";       
    }
    else s_mode = s;
    //convertion from mod to no sub mode
    // i = 0 msk144
    if (i==1 || i==2 || i==3 || i==4 || i==5 || i==6 || i==10) i=1;//"JTMS","FSK441","FSK315","ISCAT-A","ISCAT-B","JT6M" for any case "PI4" 
    if (i==13) i=2;//"FT4"    
    if (i==11) i=3;//"FT8"        
    if (i==7 || i==8 || i==9) i=4;//"JT65A","JT65B","JT65C"  
    if (i==0 || i==12) i=0;//msk144 msk144ms    
    if (i==14 || i==15 || i==16 || i==17) i=5;//Q65        
    if (i==18) i=6;//"FT2"
    QString mode = "UNKNOWN";
    if (i>=0 && i<=COUNT_FREQ_MODES-1) mode = ModeStrForFerq[i];//HV no pos_mod_rea_frq[i]
    s_mode_str_for_ferq = mode;
    if (s_mode=="FT8" || s_mode=="FT4" || s_mode=="FT2" || s_mode=="MSK144" || s_mode=="MSKMS") f_mods_accept_cmd = true;
    else f_mods_accept_cmd = false;
    SendStatus(8);//qDebug()<<"Mode=8"<<s<<s_smode<<mode<<f_mods_accept_cmd;
    if (cb_wr_status->isChecked()) write_status_timer->start(400);
}
void RadioAndNetW::SetFreqGlobal(QString s)
{
    FREQ_GLOBAL = s;//qDebug()<<"Freq=7"<<FREQ_GLOBAL;
    SendStatus(7);//2.55
    if (cb_wr_status->isChecked()) write_status_timer->start(400);
}
void RadioAndNetW::FindFreqRadList(int id,QString &sfrq)
{
    for (int z=0; z<COUNT_BANDS; z++)
    {
        if (s_band == lst_bands[z])
        {
            sfrq=THvRadList->model.item(z, 3)->text();
            QStringList lsfrq = sfrq.split("\n");
            QString smode=THvRadList->model.item(z, 2)->text();
            QStringList lsmode = smode.split("\n");
            for (int x=0; x<COUNT_FREQ_MODES; x++)
            {
                if (s_mode_str_for_ferq==lsmode.at(x))//HV no by find name   pos_mod_rea_frq[x]
                {
                    sfrq=lsfrq.at(x);
                    break;
                }
            }
            sfrq.remove(".");
            sfrq.remove(",");
            //long long int frq0 = sfrq.toLongLong();//2.31 remove first zero 0.455.000
            //sfrq = QString("%1").arg(frq0);
            //if (id==0)
			//qDebug()<<"FindFreqRadList="<<sfrq;//sfrq=sfrq.mid(0,sfrq.count()-3);//+"000" to KHz
            if (id==1 || id==10) sfrq=sfrq.mid(0,sfrq.count()-3);//+"000" to KHz//id 0->to psk reporter 1->to dx clusters spot 10->empty to dx clusters spot                
            break;
        }
    }
}
void RadioAndNetW::AddRemoteStation(QString hisCall,QString hisLoc,int frq_offset,QString mode,int snr,int id,QString pwidth,QString hhmmsspt)//id 0->to psk reporter 1->to dx clusters spot 10=empty
{
    //from file open no emit HV
    //global block emitting if somting mising here HV
    //need hiscall hisloc HV
    //qDebug()<<"PskReporter"<<s_myCall<<hisCall<<hisLoc;
    QString sfrq;
    if (cb_start_stop_psk_rpt->isChecked() && id==0)//id 0->to psk reporter 1->to dx clusters spot 10->empty to dx clusters spot
    {
        if (!THvQthLoc.isValidCallsign(s_myCall))
        {
            static bool msg_started_once = false;
            if (msg_started_once) return;
            msg_started_once = true;
            QString infoc = tr("PSK Reporter Problem")+"\n"+s_myCall+" "+tr("Is Not A Valid Call\nGo To Menu Options Macros And Set MY CALL")+":";
            if (s_myCall.isEmpty()) infoc = tr("PSK Reporter Problem\nMY CALL: Is Empty\nGo To Menu Options Macros And Set MY CALL")+":";
            QMessageBox::critical(w_parent,"MSHV",(infoc),QMessageBox::Close);
            msg_started_once = false;
            return;
        }

        if (mode=="PI4" && hisCall.isEmpty()) return;
        if (mode!="PI4" && (hisCall.isEmpty() || hisLoc.isEmpty())) return;

        sfrq = "0";
        if (mode=="PI4") FindFreqLocFromBcnList(hisCall,id,hisLoc,sfrq);//for pi4
        if (sfrq=="0" && mode!="PI4")// no emit if no find from BcnList to psk_Reporter
        {
            unsigned long long freq = FREQ_GLOBAL.toLongLong()+frq_offset;
            sfrq = QString("%1").arg(freq);//1.61=
            //FindFreqRadList(id,sfrq);
        }

        //qDebug()<<"PskReporter1"<<sfrq;
        if (s_myCall.isEmpty() || hisCall.isEmpty() || sfrq.toLongLong()<100000 ) return;//here hz 100000hz 100khz min            
        //qDebug()<<"PskReporter2"<<sfrq;

        //psk_Reporter->addRemoteStation(hisCall,hisLoc,sfrq,mode,QString("%1").arg(snr),
        //QString::number(QDateTime::currentDateTimeUtc().t_oTime_t()));
        psk_Reporter->addRemoteStation(hisCall,hisLoc,sfrq,mode,snr,hhmmsspt);
        //qDebug()<<"PskReporter"<<hisCall<<hisLoc<<QString::number(QDateTime::currentDateTimeUtc().toTime_t())<<sfrq<<snr<<mode;
    }
    else if (id==1)//id 0->to psk reporter 1->to dx clusters spot 10->empty to dx clusters spot
    {
        sfrq = "0";
        if (mode=="PI4") FindFreqLocFromBcnList(hisCall,id,hisLoc,sfrq);//for pi4            
        QStringList list;
        if (pwidth=="NA")//no aplicatenle -> no ping width
        {
            QString ssnr = QString("%1").arg(snr);
            if (!ssnr.contains("-")) ssnr.prepend("+");
            list<<s_myCall<<s_myLoc<<hisCall<<hisLoc<<sfrq<<mode<<ssnr+" dB";
        }
        else // have ping width
        {
            /*double ipw = pwidth.toInt();
            if(ipw>=1000)
            {
            	ipw = ipw / 1000;
            	pwidth = QString("%1").arg((int)ipw)+"s";
            }*/
            int ipw = pwidth.toInt();
            if (ipw>=9980) pwidth = "9980";

            QString pw = pwidth+"/"+QString("%1").arg(snr);
            list<<s_myCall<<s_myLoc<<hisCall<<hisLoc<<sfrq<<mode<<pw;
        }
        StartSpotDialod(list,frq_offset,id);//id 0->to psk reporter 1->to dx clusters spot 10->empty to dx clusters spot
    }
}
QString RadioAndNetW::RemBegEndWSpaces(QString str)
{
    QString s;
    /*int msg_count = 0;//2.64 stop
    for (msg_count = str.count()-1; msg_count>=0; msg_count--)
    {
        if (str.at(msg_count)!=' ')
            break;
    }
    s = str.mid(0,msg_count+1);
    msg_count = 0;
    for (msg_count = 0; msg_count<s.count(); msg_count++)
    {
        if (s.at(msg_count)!=' ')
            break;
    }
    s = s.mid(msg_count,(s.count()-msg_count));*/
    s = str.trimmed();
    return s;
}
void RadioAndNetW::StartSpotDialod(QStringList list,int f_offset,int id)//id 0->to psk reporter 1->to dx clusters spot 10->empty to dx clusters spot
{
    if (!THvQthLoc.isValidCallsign(list[0]))
    {
        QString infoc = tr("DX-Spot Problem")+"\n"+list[0]+" "+tr("Is Not A Valid Call\nGo To Menu Options Macros And Set MY CALL")+":";
        if (list[0].isEmpty()) infoc = tr("DX-Spot Problem\nMY CALL: Is Empty\nGo To Menu Options Macros And Set MY CALL")+":";
        QMessageBox::critical(w_parent, "MSHV",(infoc),QMessageBox::Close);
        return;
    }

    QString sfrq = list[4];
    if (sfrq=="0")
    {
        unsigned long long freq = FREQ_GLOBAL.toLongLong();//+f_offset;  // qDebug()<<"PskReporter1"<<freq;
        QString tsfrq = QString("%1").arg(freq);
        //sfrq = tsfrq.mid(0,tsfrq.size()-3);//stop 2.29
        sfrq = tsfrq.mid(0,tsfrq.size()-2);
        double roundi = sfrq.toDouble()/10.0;
        roundi = round(roundi);
        sfrq = QString("%1").arg((int)roundi);
        //2^32 =4294967296
    }
    list[4] = sfrq;
    //if (f_offset>0)
    //list[6].append(" "+QString("%1").arg(f_offset)+" Hz");

    //QStringList list;
    //list<<s_myCall<<s_myLoc<<hisCall<<hisLoc<<sfrq<<mode<<ssnr;
    //qDebug()<<list;

    TSpotDialog->setAllEdit(list,id,f_offset);//id 0->to psk reporter 1->to dx clusters spot 10->empty to dx clusters spot
    TSpotDialog->exec();

    if (TSpotDialog->send_spot_result)
        list = TSpotDialog->getAllEdit();
    else
        return;

    if (list[0].isEmpty() || list[2].isEmpty() || list[4].toLongLong()<100 )//here khz 100000hz 100khz min
        return; // for any case if no myCall or HisCall or Freq no emit
    //qDebug()<<list[4];

    // remove empty
    QString remarks = RemBegEndWSpaces(list[5]);

    QString result = "DX ";//"DX OZ2M 144000 JO65FR<TR>KN23SF Telnet test";
    result.append(list[2]);
    result.append(" ");
    result.append(list[4]);
    result.append(" ");
    result.append(remarks);

    //qDebug()<<"TelnetClient"<<result;
    TelnetClient->writeData(result);
}
void RadioAndNetW::StartEmptySpotDialog()
{
    QStringList list;
    list<<s_myCall<<s_myLoc<<""<<""<<"0"<<""<<"";
    StartSpotDialod(list,0,10);//id 0->to psk reporter 1->to dx clusters spot 10->empty to dx clusters spot
}
void RadioAndNetW::StartStopUdpBroad(bool)
{
    bool f = false;
    RefreshUdpOrTcpBroadLoggedAll();
    if (cb_udp_broad_log_qso->isChecked() || cb_udp_broad_log_adif->isChecked() || cb_udp_broad_decod->isChecked()) f = true;
    if (f)
    {
        l_udp_broad_info->setText(tr("Status: Connecting..."));
        pb_re_conect_udp_broad->setEnabled(true);
        UDPSrvPortBroadChanged("a");
        //ServTextChanged(UDPServer->text());
    }
    else
    {
        l_udp_broad_info->setText(tr("Status: Disconnecting..."));
        pb_re_conect_udp_broad->setEnabled(false);
        m_messageClientBroad->set_server("_none_");
        //ServTextChanged("_none_");
    }
}
void RadioAndNetW::ReconnectUdpBroad()
{
    l_udp_broad_info->setText(tr("Status: Reconnecting..."));
    UDPSrvPortBroadChanged("a");
    //ServTextChanged(UDPServer->text());
}
void RadioAndNetW::UDPSrvPortBroadChanged(QString)
{
    QString s = UDPServerBroad->text();
    QString p = UDPPortBroad->text();
    if (!s.isEmpty() && !p.isEmpty() &&
            (cb_udp_broad_log_qso->isChecked() || cb_udp_broad_log_adif->isChecked() || cb_udp_broad_decod->isChecked()))// && cb_udp_broad->isChecked())
    {
        l_udp_broad_info->setText(tr("Status: Connecting..."));
        //set port
        m_messageClientBroad->set_server_port(p.toInt());
        //set server
        m_messageClientBroad->set_server(s);
        //qDebug()<<"UDPSrvPortBroadChanged"<<s<<p;
    }
}
void RadioAndNetW::ConectionInfoBroad(QString s)
{
    if (cb_udp_broad_log_qso->isChecked() || cb_udp_broad_log_adif->isChecked() || cb_udp_broad_decod->isChecked())
        l_udp_broad_info->setText(tr("Status")+": "+s);
    else
        l_udp_broad_info->setText(tr("Status")+": <font color='red'>"+tr("UDP Broadcast Is Disabled And Disconnected")+"</font>");
}
void RadioAndNetW::SendLoggedQSO(QStringList ls)
{
    if (!cb_udp_broad_log_qso->isChecked()) return;
    m_messageClientBroad->logged_QSO(ls); //qDebug() << "SendLoggedQSO=" <<ls;
}
void RadioAndNetW::StartStopUdp2Broad(bool)
{
    RefreshUdpOrTcpBroadLoggedAll();
}
void RadioAndNetW::SendAdifRecord(QString s)
{
    QString srem_lf = s.mid(0,s.count()-1); //remove -> \n
    if (cb_tcp_broad_log_adif->isChecked())
    {
        if (socet_tcp_broad->state() != QAbstractSocket::ConnectedState)
        {
            int p1 = TCPPortBroad->text().toInt();
            socet_tcp_broad->connectToHost(TCPServerBroad->text(),p1);
            socet_tcp_broad->waitForConnected(100);
        }
        if (socet_tcp_broad->state() == QAbstractSocket::ConnectedState)
        {
            QString sendd_ = ("<command:3>log<parameters:"+QString("%1").arg(srem_lf.count())+">"+srem_lf);
            QByteArray data = sendd_.toUtf8(); //qDebug()<<"SendAdifRecord="<<sendd_;
            socet_tcp_broad->write(data);
        }
    }
    if (cb_udp_broad_log_adif->isChecked())
    {
        QByteArray adif;
        adif.append(srem_lf.toUtf8());
        m_messageClientBroad->logged_ADIF(adif);
    }
    if (cb_udp2adif->isChecked())
    {
        static QString s0 = "-s-h-";
        static int p0 = -1;
        int p1 = udp2_Port->text().toInt();
        if (s0 != udp2_Server->text() || p0 != p1 || socet_udp2_broad->state() != QAbstractSocket::ConnectedState)
        {
            if (socet_udp2_broad->state() == QAbstractSocket::ConnectedState)
            {
                socet_udp2_broad->disconnectFromHost(); //socet_udp2_broad->waitForDisconnected(300); //???
            }
            s0 = udp2_Server->text();
            p0 = p1;
            socet_udp2_broad->connectToHost(udp2_Server->text(),p1);
            socet_udp2_broad->waitForConnected(200);
        }
        if (socet_udp2_broad->state() == QAbstractSocket::ConnectedState)
        {
            QString sendd_ = "<PROGRAMID:4>MSHV<EOH>"+srem_lf;
            QByteArray data = sendd_.toUtf8();
            socet_udp2_broad->write(data); //qDebug()<<sendd_; qDebug()<<s;
        }
        //No good idea slow down App spead -> The socket is bound to an address and port.
        //socet_udp2_broad->writeDatagram(data,QHostAddress(udp2_Server->text()),p1);
    }
    if (cb_clublog->isChecked()) SendClubLogAdif(s); //ssl needed -> \n
    if (cb_qrzlog->isChecked() ) SendQRZLogAdif(s);	 //ssl
    if (cb_eqsl->isChecked()   ) SendEQSLAdif(s);
}
void RadioAndNetW::SetUdpDecClr()
{	//qDebug()<<"CCCCC=";
    if (!cb_udp_broad_decod->isChecked()) return;
    m_messageClientBroad->decodes_cleared();
}
#define MAXDECOD 210 //2.71 150
QString atim[MAXDECOD+20];
int asn[MAXDECOD+20];
QString adt[MAXDECOD+20];
int afrq[MAXDECOD+20];
QString amsg[MAXDECOD+20];
void RadioAndNetW::SendDecodTxt(QString tim,int sn,QString dt,int frq,QString msg)
{
    //qDebug()<<"s_mode="<<s_mode<<tim;
    if (!cb_udp_broad_decod->isChecked()) return;
    if (pos_dec > MAXDECOD) return;
    atim[pos_dec]=tim;
    asn [pos_dec]=sn;
    adt [pos_dec]=dt;
    afrq[pos_dec]=frq;
    amsg[pos_dec]=msg;
    pos_dec++;
    if (id_activ_upd > 1) return;
    id_activ_upd = 1;//0=no 1=decoded 2=replay
    decod_upd_timer->start(20); //2.59 old=1000
}
void RadioAndNetW::set_reply_clr(QStringList l)
{
    //qDebug()<<"f_mods_accept_cmd"<<f_mods_accept_cmd<<l;
    if (f_mods_accept_cmd || l.at(0)=="CLR") emit EmitUdpCmdDl(l);
}
void RadioAndNetW::set_halt_tx(bool f)
{
    //if (f_mods_accept_cmd)
    emit EmitUdpCmdStop(f);
}
void RadioAndNetW::SendStatus(int id)
{
    if (!cb_udp_broad_decod->isChecked()) return;
    if (id_activ_upd != 0) return;
    id_activ_upd = id;//0=no 1=decoded 2=replay....freq=7,last=8->mode
    decod_upd_timer->start(150); //2.59=200 wait for ->s_tx_msg  old=100
    //qDebug()<<"Staus====="<<id;
}
void RadioAndNetW::replayDecodes()
{
    SendStatus(2); //qDebug()<<"replayDecodes";
    //at start or disconnect and connect
    //emit status and all old decodes with flag -> is_new = false; and fdec = false;
    //MSHV not suport this function, only status at start or disconnect and connect
}
//QElapsedTimer ttt;
void RadioAndNetW::SetDxParm(QString dc,QString re,QString dxg)
{
    s_dx_call = dc;
    s_report = re;
    s_dx_grid = dxg;
    SendStatus(3); //qDebug()<<"SetDxParm="<<dc<<re<<dxg;  //ttt.start();
    if (cb_wr_status->isChecked()) write_status_timer->start(400);
}
void RadioAndNetW::SetAuto(bool f)
{
    s_auto = f;
    SendStatus(4); //qDebug()<<"SetAuto="<<f;
}
void RadioAndNetW::SetTx(bool f)
{
    s_tx = f;
    SendStatus(5); //qDebug()<<"SetTx="<<f;
}
void RadioAndNetW::SetTxMsg(QString s)//2.52
{
    s_tx_msg = s.replace("#",", ");//MAM TX message #
    //s_tx_msg = s;
    SendStatus(6); //qDebug()<<"SetTxMsg="<<s<<6;
}
/*void RadioAndNetW::SetDecode(bool f)//2.59
{
    f_de_active = f;
    SendStatus(9); //qDebug()<<"SetDecode"<<f;
}*/
void RadioAndNetW::WriteStatusTimer() //70.154;FT8;LZ2HV;-15;FT8;KN23
{
    write_status_timer->stop();
    QFile file(AppPath+"/settings/mshv_status.txt"); //QFile file(AppPath+"/settings/wsjtx_status.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;
    QTextStream out(&file);
    long long int ifrq = FREQ_GLOBAL.toLongLong();
    double frq = (double)(ifrq/1000);//rem Hz
    frq = frq/1000.0;//to kHz
    QString dx_gri = s_dx_grid;
    if (dx_gri.isEmpty()) dx_gri = "n/a";
    QString sout = QString("%1").arg(frq,0,'f',3)+";"+s_mode+";"+s_dx_call+";"+s_report+";"+s_mode+";"+dx_gri+"\n";
    out << sout;
    file.close(); //qDebug()<<sout;
}
void RadioAndNetW::DecodUpdTimer()
{
    if (pos_upd == -1)
    {
        bool fdec = false;
        if (pos_dec > 0) fdec = true; //2.61
        //if (id_activ_upd != 1 && pos_dec > 0)  qDebug()<<"error";
        //qDebug()<<"START Status----------------------->"<<id_activ_upd<<s_dx_call<<s_dx_grid;
        quint64 frq = FREQ_GLOBAL.toLongLong();
        m_messageClientBroad->statusUPD(frq,s_mode,s_dx_call,s_report,s_mode,s_myCall,s_myLoc,s_dx_grid,fdec,s_smode,
                                        s_auto,s_tx,s_tx_msg);
        if (pos_dec == 0)//2.59
        {
            //qDebug()<<"0-END Status----->"<<id_activ_upd;
            decod_upd_timer->stop();//for eny case
            id_activ_upd = 0;//0=no 1=decoded 2=replay...last=8
        }
        else
        {
            pos_upd++;
            decod_upd_timer->start(20); //2.59 old=100
        }
    }
    else
    {
        if (pos_upd < pos_dec)
        {
            //qDebug()<<"2-Status start"<<id_activ_upd;
            //qDebug()<<"N="<<pos_upd<<atim[pos_upd]<<asn[pos_upd]<<adt[pos_upd]<<afrq[pos_upd]<<s_mode<<amsg[pos_upd];
            bool is_new = true; //2.61 all is new
            m_messageClientBroad->decode_TXT(is_new,atim[pos_upd],asn[pos_upd],adt[pos_upd],afrq[pos_upd],s_mode,amsg[pos_upd]);
            pos_upd++;

            if (pos_upd < pos_dec) decod_upd_timer->start(10); //2.59 old=20 -> 50*20ms=1000ms
            else decod_upd_timer->start(350);//2.59 old=100  for network errors   qDebug()<<"Status END"<<pos_upd<<pos_dec;
        }
        else
        {
            //qDebug()<<"Status stop"<<pos_upd<<false;
            decod_upd_timer->stop();//for eny case

            if (pos_upd > 0)
            {
                //qDebug()<<"1-END Status-------->"<<id_activ_upd;
                quint64 frq = FREQ_GLOBAL.toLongLong();
                m_messageClientBroad->statusUPD(frq,s_mode,s_dx_call,s_report,s_mode,s_myCall,s_myLoc,s_dx_grid,false,s_smode,
                                                s_auto,s_tx,s_tx_msg);
            }
            pos_upd = -1;
            pos_dec = 0;
            id_activ_upd = 0;//0=no 1=decoded 2=replay...last=8
        }
    }
}
void RadioAndNetW::SaveSettings()
{
    QFile file(sr_path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;
    QTextStream out(&file);

    out << "udp_server=" << UDPServer->text() << "\n";
    out << "udp_port=" << UDPPort->text() << "\n";
    out << "psk_spot_val=" << QString("%1").arg(cb_start_stop_psk_rpt->isChecked()) << "\n";

    QString sada;
    for (int j=0;  j < THvRadList->model.rowCount(); j++)
    {
        sada.append(THvRadList->model.item(j, 0)->text()+"=");
        sada.append(THvRadList->model.item(j, 1)->text()+"||");
        QString str = THvRadList->model.item(j, 3)->text();
        QStringList lstr = str.split("\n");
        str.clear(); //qDebug()<<lstr;
        for (int x = 0;  x < lstr.count(); x++)
        {
            str.append(lstr.at(pos_mod_sav_frq[x]));
            str.remove(".");
            str.remove(",");
            if (x<lstr.count()-1) str.append(";");
        }
        sada.append(str);
        if (j<THvRadList->model.rowCount()-1) sada.append("#");
    }
    out << "st_info_all=" << sada << "\n";
    out << "dx_spot_telnet_val=" << QString("%1").arg(global_tcp_stat) << "\n";
    out << "tcp_server=" << TCPServer->text() << "\n";
    out << "tcp_port=" << TCPPort->text() << "\n";
    out << "udp_broad_server=" << UDPServerBroad->text() << "\n";
    out << "udp_broad_port=" << UDPPortBroad->text() << "\n";
    out << "udp_broad_log_all=" << QString("%1").arg(cb_udp_broad_log_qso->isChecked()) << "#"
    << QString("%1").arg(cb_udp_broad_log_adif->isChecked()) << "#"
    << QString("%1").arg(cb_udp_broad_decod->isChecked()) << "\n";
    out << "psk_udp_tcp=" << QString("%1").arg(cb_udp_tcp_psk_rpt->isChecked()) << "\n";
    out << "tcp_broad_log_all=" << TCPServerBroad->text() << "#" << TCPPortBroad->text() << "#"
    << QString("%1").arg(cb_tcp_broad_log_adif->isChecked()) << "\n";
    out<<"tcps_club_log_all="<<LeClubLogServer->text()<<"#"<<LeClubLogPort->text()<<"#"<<LeClubLogPost0->text()<<"#"<<LeClubLogPost1->text()<<"#"
    <<LeClubLogMail->text()<<"#"<<LeClubLogPass->text()<<"#"<<QString("%1").arg(cb_clublog->isChecked())<<"\n";
    out <<"udp2_broad_all="<<udp2_Server->text()<<"#"<<udp2_Port->text()<<"#"<<QString("%1").arg(cb_udp2adif->isChecked())<<"\n";
    out <<"tcps_qrz_log_all="<<LeQRZLogServer->text()<<"#"<<LeQRZLogPort->text()<<"#"<<LeQRZLogPost->text()<<"#"
    << LeQRZLogApi->text()<<"#"<<QString("%1").arg(cb_qrzlog->isChecked())<<"\n";
    out << "def_wr_status=" << QString("%1").arg(cb_wr_status->isChecked())<<"\n";
    out <<"tcp_eqsl_log_all="<<LeEQSLServer->text()<<"#"<<LeEQSLPort->text()<<"#"<<LeEQSLPost->text()<<"#"
    <<LeEQSLUser->text()<<"#"<<LeEQSLPass->text()<<"#"<<LeEQSLQTHNick->text()<<"#"<<QString("%1").arg(cb_eqsl->isChecked())<<"#"
    <<LeEQSLmsg->text()<<"\n";
    out <<"tcp_otp_all="<<LeOtpServer->text()<<"#"<<LeOtpPort->text()<<"#"<<QString("%1").arg(cb_otp_msg->isChecked())<<"#"
    <<QString("%1").arg(cb_otp_key->isChecked())<<"#"<<LeOtpKey->text()<<"\n";
    sada.clear();
    for (int j=1;  j<CbOTPServers->count(); ++j)
    {
    	sada.append(CbOTPServers->itemText(j)); 
    	if (j<CbOTPServers->count()-1) sada.append("#");
   	}
    out<<"otp_servers_list="<<sada<<"\n";
    out << "tcp_pass=" << TCPPass->text() << "\n";
    
    file.close();
}
bool RadioAndNetW::isFindId(QString id,QString line,QString &res)
{
    bool fres = false;
    QRegExp rx;
    rx.setPattern(id+"=\"?([^\"]*)\"?");
    if (rx.indexIn(line) != -1)
    {
        res = rx.cap(1);
        fres = true;
    }
    return fres;
}
void RadioAndNetW::ReadSettings()
{
    const int c_st_id = 20; //dopalva se tuk v kraia
    const QString st_id[c_st_id]=
        {
            "udp_server","udp_port","psk_spot_val","st_info_all","dx_spot_telnet_val","tcp_server","tcp_port",
            "udp_broad_server","udp_broad_port","udp_broad_log_all","psk_udp_tcp","tcp_broad_log_all",
            "tcps_club_log_all","udp2_broad_all","tcps_qrz_log_all","def_wr_status","tcp_eqsl_log_all",
            "tcp_otp_all","otp_servers_list","tcp_pass"
        };
    QString st_res[c_st_id];
    for (int i = 0; i < c_st_id; ++i) st_res[i]="";

    QFile file(sr_path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        UDPPort->setText("4739"); //ako ne pro4ete ms_mesages
        UDPServer->setText("report.pskreporter.info");
        cb_start_stop_psk_rpt->setChecked(false);
        cb_udp_tcp_psk_rpt->setChecked(false);
        return;
    }
    //QTime ttt;
    //ttt.start();
    QTextStream in(&file);

    while (!in.atEnd())
    {
        QString line = in.readLine();
        //QRegExp rx;

        for (int i = 0; i < c_st_id; ++i)
        {
            if (isFindId(st_id[i],line,st_res[i]))
            {
                line = in.readLine();
                //qDebug()<<i<<"idfind="<<st_id[i]<<"id="<<st_res[i];
            }
            //else
            //qDebug()<<i<<"FALSE READ --------------------------------  idfind="<<st_id[i];
        }
        //qDebug()<<"ffffffffffffffff===============";
    }
    file.close();
    //qDebug()<<"2Time="<<ttt.elapsed();

    //if (!st_res[0].isEmpty()) UDPPort->setText(st_res[0]);
    //if (!st_res[1].isEmpty()) UDPServer->setText(st_res[1]);
    if (!st_res[10].isEmpty() && st_res[10].toInt()==1) cb_udp_tcp_psk_rpt->setChecked(true);
    if (!st_res[2].isEmpty() && st_res[2].toInt()==1) cb_start_stop_psk_rpt->setChecked(true);
    if (!st_res[3].isEmpty())
    {
        QStringList ls=st_res[3].split("#");

        QStringList lsm;
        for (int z=0; z<COUNT_BANDS; z++)
            lsm << lst_bands[z]+"=";

        for (int i=0; i<COUNT_BANDS; i++)
        {
            QString tstr = lsm[i];
            for (int j=0; j<ls.count(); j++)
            {
                if (ls[j].contains(tstr))
                {
                    ls[j].remove(tstr);
                    QStringList lss = ls[j].split("||");

                    QString frq_mod = lss.at(1);
                    QStringList lfrq_mod = frq_mod.split(";");
                    QString frq;

                    if (lfrq_mod.count()!=COUNT_FREQ_MODES)
                    {
                        lfrq_mod.clear();//2.18 missed frq reset to default
                        for (int z=0; z<COUNT_FREQ_MODES; z++) lfrq_mod << "";
                    }

                    for (int x=0; x<COUNT_FREQ_MODES; x++)
                    {
                        QString frq0;
                        if (lfrq_mod.at(x).isEmpty()) frq.append(all_bands_mods_frq[i][pos_mod_rea_frq[x]]);
                        else
                        {
                            frq0 = lfrq_mod.at(pos_mod_rea_frq[x]);
                            int k = frq0.count();
                            k -= 3;
                            if (k>0)
                                frq0.insert(k,".");//Hz 1.44
                            k -= 3;
                            if (k>0)
                                frq0.insert(k,".");//KHz 1.44
                            frq.append(frq0);
                        }
                        if (x<COUNT_FREQ_MODES-1) frq.append("\n");
                    }
                    lss[1] = frq;
                    lss.prepend(lst_bands[i]);
                    //qDebug()<< frq;

                    //if (!l[j].isEmpty())
                    //{
                    //l<<lst_bamds[i]<<l.at(0)<<l.at(1);
                    THvRadList->SetItem_hv(lss,i); //auto_decode_all[i] = ls[j];
                    //}
                    //qDebug()<< auto_decode_all[i];
                    break;
                }
            }
        }
    }
    if (!st_res[4].isEmpty()) SetGlobalTcpStat(st_res[4]);
    if (!st_res[5].isEmpty()) TCPServer->setText(st_res[5]);
    if (!st_res[6].isEmpty()) TCPPort->setText(st_res[6]);
    if (!st_res[19].isEmpty()) TCPPass->setText(st_res[19]);//2.76.3
    if (!st_res[7].isEmpty()) UDPServerBroad->setText(st_res[7]);
    if (!st_res[8].isEmpty()) UDPPortBroad->setText(st_res[8]);
    if (!st_res[9].isEmpty())
    {
        QStringList lsqa = st_res[9].split("#");
        if (lsqa.count()>2)
        {
            if (lsqa.at(0)=="1")
                cb_udp_broad_log_qso->setChecked(true);
            if (lsqa.at(1)=="1")
                cb_udp_broad_log_adif->setChecked(true);
            if (lsqa.at(2)=="1")
                cb_udp_broad_decod->setChecked(true);
        }
    }
    if (!st_res[11].isEmpty())
    {
        QStringList ls=st_res[11].split("#");
        if (ls.count()>2)
        {
            TCPServerBroad->setText(ls.at(0));
            TCPPortBroad->setText(ls.at(1));
            if (ls.at(2)=="1") cb_tcp_broad_log_adif->setChecked(true);
        }
    }
    if (!st_res[12].isEmpty())
    {
        QStringList ls = st_res[12].split("#");
        if (ls.count()>6)
        {
            //LeClubLogServer->setText(ls.at(0));//2.76.1
            //LeClubLogPort->setText(ls.at(1));
            //LeClubLogPost0->setText(ls.at(2));
            //LeClubLogPost1->setText(ls.at(3));
            LeClubLogMail->setText(ls.at(4));
            LeClubLogPass->setText(ls.at(5));
            if (ls.at(6)=="1") cb_clublog->setChecked(true);
        }
    }
    if (!st_res[13].isEmpty())
    {
        QStringList ls=st_res[13].split("#");
        if (ls.count()>2)
        {
            udp2_Server->setText(ls.at(0));
            udp2_Port->setText(ls.at(1));
            if (ls.at(2)=="1") cb_udp2adif->setChecked(true);
        }
    }
    if (!st_res[14].isEmpty())
    {
        QStringList ls = st_res[14].split("#");
        if (ls.count()>4)
        {
            //LeQRZLogServer->setText(ls.at(0));//2.76.1
            //LeQRZLogPort->setText(ls.at(1));
            //LeQRZLogPost->setText(ls.at(2));
            LeQRZLogApi->setText(ls.at(3));
            if (ls.at(4)=="1") cb_qrzlog->setChecked(true);
        }
    }
    if (!st_res[15].isEmpty())
    {
        if (st_res[15]=="1") cb_wr_status->setChecked(true);
    }
    if (!st_res[16].isEmpty())
    {
        QStringList ls = st_res[16].split("#");
        if (ls.count()>7)
        {
            //LeEQSLServer->setText(ls.at(0));//2.76.1
            //LeEQSLPort->setText(ls.at(1));
            //LeEQSLPost->setText(ls.at(2));
            LeEQSLUser->setText(ls.at(3));
            LeEQSLPass->setText(ls.at(4));
            LeEQSLQTHNick->setText(ls.at(5));
            if (ls.at(6)=="1") cb_eqsl->setChecked(true);
            LeEQSLmsg->setText(ls.at(7));
        }
    }
    if (!st_res[17].isEmpty())
    {
        QStringList ls = st_res[17].split("#");
        if (ls.count()>4)
        {
            LeOtpServer->setText(ls.at(0));
            LeOtpPort->setText(ls.at(1));
            if (ls.at(2)=="1") cb_otp_msg->setChecked(true);//if (ls.at(3)=="1") cb_otp_mamd_key->setChecked(true);
            if (ls.at(3)=="1") cb_otp_key->setChecked(true);
            LeOtpKey->setText(ls.at(4));
        }
    }
    if (!st_res[18].isEmpty())
    {
    	CbOTPServers->clear();
        QStringList ls = st_res[18].split("#");
        ls.prepend(tr("List Servers"));
        CbOTPServers->addItems(ls);
        CbOTPServers->setCurrentIndex(0);
    }
}




