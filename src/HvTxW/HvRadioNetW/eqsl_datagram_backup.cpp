			//for the future -> "http://www.eqsl.cc/qslcard/importADIF.cfm
            /*res = "POST /qslcard/importADIF.cfm HTTP/1.1\r\n";
            res.append("Host: eqsl.cc\r\n");
            tmp = "User-Agent: MSHV/"+QString(VER_MS)+"\r\n";
            res.append(tmp.toUtf8());*/
            //res.append("Accept: */*\r\n");
            /*res.append("Content-Type: application/x-www-form-urlencoded\r\n");
            tmp = "<ADIF_VER:5>3.1.0<PROGRAMID:4>MSHV";
            QString s = "LZ2HV";
            tmp.append("<EQSL_USER:"+QString("%1").arg(s.count())+">"+s);
            s = "****";
            tmp.append("<EQSL_PSWD:"+QString("%1").arg(s.count())+">"+s+"<EOH>\n");
            s = "Tarnovo";
            tmp.append("<APP_EQSL_QTH_NICKNAME:"+QString("%1").arg(s.count())+">"+s);        
            cnt = "ADIFData="+URLEncode(tmp+adiftrim);
            cnt = "ADIFData="+URLEncode("<ADIF_VER:5>3.1.0<PROGRAMID:4>MSHV<EOH>"+adiftrim);
            cnt.append("&EQSL_USER=LZ2HV");
            cnt.append("&EQSL_PSWD=****");
            tmp = "Content-Length: "+QString("%1").arg(cnt.count());
            res.append(tmp.toUtf8());
            tmp = "\r\n\r\n"+cnt;
            res.append(tmp.toUtf8());*/
            //qDebug()<<res.data();
            /*qDebug()<<"Upoad=--->"<<(float)ttt.elapsed()/1000.0;
            if (cl_pos_ul_read >= cl_pos_ul_writ) qDebug()<<"Stop Upoad=------------->"<<(float)ttt.elapsed()/1000.0;
            qDebug()<<"---------------------------------------";*/
            //qDebug()<<"Send "<<cl_pos_ul_read<<(float)ttt.elapsed()/1000.0;
			



		
void RadioAndNetW::StartStopEQSL(bool f)
{
    if (f)
    {
        if (socet_tcp_eqsl->state() != QAbstractSocket::ConnectedState)
        {
            int p1 = LeEQSLPort->text().toInt();
            socet_tcp_eqsl->connectToHostEncrypted(LeEQSLServer->text(),p1);
            socet_tcp_eqsl->waitForConnected(500);
        }
    }
    else if (socet_tcp_eqsl->state() == QAbstractSocket::ConnectedState) socet_tcp_eqsl->disconnectFromHost();
}
void RadioAndNetW::cb_eqsl_toggled()
{
    RefreshUdpOrTcpBroadLoggedAll();
}
void RadioAndNetW::readEQSL()
{
    QByteArray ba = socet_tcp_eqsl->readAll();
    QString sba = QString(ba.data()); 
    /*int icb = sba.indexOf("\r\n\r\n");
    if (icb>-1) sba = sba.mid(icb+4,sba.count()-(icb+4));
    else sba = "";
    if (sba.contains("RESULT=OK")) eqsl_err_flag = false;
    else if (sba.contains("REASON="))
    {
    	 if (sba.contains("duplicate")) eqsl_err_flag = false;
    	 icb = sba.indexOf("REASON=");
    	 eqsl_error_str = sba.mid(icb,sba.count()-icb);   	
   	}
   	else eqsl_error_str = "Unknown Error";  	
   	if (!eqsl_err_flag && eqsl_is_act_send_check == 2)
   	{
   		eqsl_upd_timer->stop();
   		eqsl_upd_timer->start(1100);
  	} */
	qDebug()<<"EQSL <-- "<<sba; 
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
    if (eqsl_is_act_send_check < 2)
    {
        if (socet_tcp_eqsl->state() != QAbstractSocket::ConnectedState) StartStopEQSL(true);
        if (socet_tcp_eqsl->state() == QAbstractSocket::ConnectedState)
        {
        	eqsl_err_flag = true;
            QString adiftrim = eqsl_adif_arr[eqsl_pos_ul_read];
            adiftrim.remove(adiftrim.size()-1,1);//remove last \n  //adiftrim = adiftrim.trimmed();
            QByteArray res = "POST /"+LeEQSLPost->text().toUtf8()+" HTTP/1.1\r\n";
            res.append("Host: "+LeEQSLServer->text().toUtf8()+"\r\n");
            QString tmp = "User-Agent: MSHV/"+QString(VER_MS)+"\r\n";
            res.append(tmp.toUtf8());
            res.append("Accept: */*\r\n");
            res.append("Content-Type: application/x-www-form-urlencoded\r\n");
            //QString cnt = "key="+LeQRZLogApi->text();
            /*cnt.append("&action=insert");
            //cnt.append("&action=status");
            cnt.append("&adif="+URLEncode("<PROGRAMID:4>MSHV<EOH>"+adiftrim));
            tmp = "Content-Length: "+QString("%1").arg(cnt.count());
            res.append(tmp.toUtf8());
            tmp = "\r\n\r\n"+cnt;
            res.append(tmp.toUtf8());
            socet_tcp_eqsl->write(res);*/ 
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
        eqsl_upd_timer->start(4900); //4900
    }
    //}
}			


EQSL <--  true 2 "\r\n\r\n<!-- Reply form eQSL.cc ADIF Real-time Interface -->\r\n\r\n\r\n<HTML>\r\n<HEAD></HEAD>\r\n<BODY>\r\n\r\n\r\n  Result: 1 out of 1 records added<BR>\r\n  \r\n  
Information: From: lz2hv To: LZ0TEST Date: 20240116 Time: 1129 Band: 20M Mode: FT8 RST: +00<BR> \r\n</CENTER>\r\n</BODY>\r\n</HTML>\r\n"
DISCONECT <--
CONECTED  <--
POST /qslcard/importADIF.cfm HTTP/1.1
Host: eqsl.cc
User-Agent: MSHV/2.74

Content-Type: application/x-www-form-urlencoded
Content-Length: 505


EQSL <--  true 2 "\r\n\r\n<!-- Reply form eQSL.cc ADIF Real-time Interface -->\r\n\r\n\r\n<HTML>\r\n<HEAD></HEAD>\r\n<BODY>\r\n\r\nWarning: Y=2024 M=01 D=16 LZ0TEST 20M FT8 Bad record: Duplicate<BR>\r\n 
 Result: 0 out of 1 records added<BR>\r\n  \r\n  \r\n</CENTER>\r\n</BODY>\r\n</HTML>\r\n"
DISCONECT <--
CONECTED  <--
POST /qslcard/importADIF.cfm HTTP/1.1
Host: eqsl.cc
User-Agent: MSHV/2.74

Content-Type: application/x-www-form-urlencoded
Content-Length: 505
TNX For QSO TU 73!.

EQSL <--  true 2 "\r\n\r\n<!-- Reply form eQSL.cc ADIF Real-time Interface -->\r\n\r\n\r\n<HTML>\r\n<HEAD></HEAD>\r\n<BODY>\r\n\r\n\r\n 
 Error: No match on eQSL_User/eQSL_Pswd\r\n  </BODY>\r\n  </HTML>\r\n  "
 
QSLMSG	Truncated after 240 characters (Note: The field called 'COMMENT' is normally used only for private comments and is NOT printed on the eQSLs
<CALL:7>LZ0TEST<QSO_DATE:8:D>20240117<TIME_ON:4>0922<BAND:2>2m<FREQ:7>14.0740<MODE:3>FT8<RST_SENT:3>+00<SAT_NAME:6>QO-100<SAT_MODE:1>A<PROP_MODE:3>SAT<QSL_SENT:1>Y<QSL_SENT_VIA:1>E<APP_EQSL_UPLOAD_DATE:8:D>20240117<EOR>