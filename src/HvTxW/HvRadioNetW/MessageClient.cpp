/* The algorithms, source code, look-and-feel of WSJT-X and related
 * programs, and protocol specifications for the modes FSK441, FT8, JT4,
 * JT6M, JT9, JT65, JTMS, QRA64, ISCAT, MSK144, are Copyright © 2001-2017
 * by one or more of the following authors: Joseph Taylor, K1JT; Bill
 * Somerville, G4WJS; Steven Franke, K9AN; Nico Palermo, IV3NWV; Greg Beam,
 * KI7MT; Michael Black, W9MDB; Edson Pereira, PY2SDR; Philip Karn, KA9Q;
 * and other members of the WSJT Development Group.
 *
 * MSHV MessageClient
 * Rewritten into C++ and modified by Hrisimir Hristov, LZ2HV 2015-2019
 * May be used under the terms of the GNU General Public License (GPL)
 */

#include "MessageClient.h"

//#include <stdexcept>
#define QT_NO_DEBUG_STREAM // remove qdebug socet
#include <QUdpSocket>
#include <QHostInfo>
#include <QTimer>
//#include <QQueue>
//#include <QByteArray>
//#include <QHostAddress>

#include "NetworkMessage.h"

#include "pimpl_impl.h"

#include "moc_MessageClient.cpp" // HV triabva vazno neznam za6to

//#include <QtGui>
class MessageClient::impl
            : public QUdpSocket
{
    Q_OBJECT
public:
    impl (QString const& id, QString const& version, port_type server_port, bool hbt, MessageClient * self)
            : self_
    {
        self
    }
    , dns_lookup_id_ {0}//2.46
    , id_ {id}
    , version_ {version}
    , server_port_ {server_port}
    , schema_ {2}  // >= qt5.2.0 = schema 2, qt4.8.6=1 HV use 2 prior to negotiation
    , shbt {hbt}
    //, heartbeat_timer_ {new QTimer {this}}
    {
        //QT5 connect (heartbeat_timer_, &QTimer::timeout, this, &impl::heartbeat);
        //qDebug()<<"MessageClient shbt="<<shbt;
        if (shbt)
        {
            heartbeat_timer_ = new QTimer();
            connect (heartbeat_timer_, SIGNAL(timeout()), this, SLOT(heartbeat()));
        }

        //QT5 connect (this, &QIODevice::readyRead, this, &impl::pending_datagrams);
        connect (this, SIGNAL(readyRead()), this, SLOT(pending_datagrams()));

        //stop 1.77 heartbeat_timer_->start (NetworkMessage::pulse * 1000);
        //heartbeat_timer_->start (1000);
        //qDebug()<<NetworkMessage::pulse * 1000;

        // bind to an ephemeral port
        bind ();
        //bind(QHostAddress::Any, 2237);//udpSocket->bind(QHostAddress::LocalHost, 7755);
    }

    ~impl ()
    {
        //qDebug()<<"NetworkMessage::close";
        closedown ();
    }

    //QHostAddress m_pskRepAdd;
    //Q_SIGNAL void ConectionInfo(QString);

    enum StreamStatus {Fail, Short, OK};

    void parse_message (QByteArray const& msg);

    Q_SLOT void pending_datagrams ();

    Q_SLOT void heartbeat ();

    void closedown ();
    StreamStatus check_status (QDataStream const&) const;

    void send_message (QDataStream const& out,QByteArray const&);

    Q_SLOT void host_info_results (QHostInfo);

    MessageClient * self_;
    int dns_lookup_id_;   //2.46
    QString id_;
    QString version_;
    //QString revision_;// no need for MSHV
    QString server_string_;
    port_type server_port_;
    QHostAddress server_;
    quint32 schema_;
    bool shbt;
    QTimer * heartbeat_timer_;
};

#include "MessageClient.moc"

void MessageClient::impl::host_info_results(QHostInfo host_info)
{
    //qDebug() << "host_info_results=" <<host_info.lookupId () << dns_lookup_id_;
    if (host_info.lookupId () != dns_lookup_id_) return;//??? 2.64
    if (QHostInfo::NoError != host_info.error ())
    {
        //Q_EMIT self_->error ("UDP server lookup failed:\n" + host_info.errorString ());
        //pending_messages_.clear (); // discard
        if (shbt)
            heartbeat_timer_->stop();
        server_.clear();
        //qDebug() << "UDP server lookup failed: " + host_info.errorString ();
        Q_EMIT self_->ConectionInfo("<font color='red'>"+tr("UDP server lookup failed")+": " + host_info.errorString()+"</font>");

    }
    else if (host_info.addresses ().size ())
    {
        server_ = host_info.addresses ()[0];

        // send initial heartbeat which allows schema negotiation
        heartbeat();

        if (!host_info.addresses().isEmpty())
        {
            if (shbt)
                heartbeat_timer_->start(NetworkMessage::pulse * 1000);
            //QHostAddress m_pskReporterAddress;
            //m_pskRepAdd = host_info.addresses().at(0);
            //qDebug() << "MessageClient::impl HostInfo=" << m_pskRepAdd <<host_info.hostName();
            Q_EMIT self_->ConectionInfo("<font color='#00b300'>"+tr("Connected to")+" "+host_info.hostName()+"  IP "+server_.toString()+"</font>");
        }
    }
}
void MessageClient::impl::pending_datagrams ()
{
    //qDebug() << "MessageClient::impl HostInfo=";
    while (hasPendingDatagrams ())
    {
        QByteArray datagram;
        datagram.resize (pendingDatagramSize ());
        QHostAddress sender_address;
        port_type sender_port;
        if (0 <= readDatagram (datagram.data (), datagram.size (), &sender_address, &sender_port))
        {
            parse_message (datagram);
        }
    }
}
void MessageClient::impl::parse_message(QByteArray const& msg)
{
    /*try
    {*/
    //qDebug() << "--IN-- Message RX="<<(QString(msg.toHex()));
    /*qint32 x =0;
    x |= msg[0] & 0xFF;
    x <<= 8; 
    x |= msg[1] & 0xFF;
    x <<= 8;
    x |= msg[2] & 0xFF;
    x <<= 8;
    x |= msg[3] & 0xFF;
    if (s != (qint32)NetworkMessage::Builder::magic) return;*/
    // protect from logger32 (0000)	<- msg
    if (msg.count()<4) return; //2.70
    QByteArray bmagic = msg.mid(0,4);
    if (bmagic.toHex() != "adbccbda") return; //from NetworkMessage.h -> "adbccbda" in hex for fast work

    //qDebug() << "Message RX="<<(QString(msg.toHex()));
    // message format is described in NetworkMessage.hpp
    //
    NetworkMessage::Reader in {msg};
    if (OK == check_status (in)) // OK and for us    && id_ == in.id ()
    {
        if (schema_ < in.schema ()) schema_ = in.schema (); // negotiated schema   one time record of server's
        //qDebug() <<"In="<<in.type()<<in.id()<<schema_<<in.schema ();
        switch (in.type())
        {
        case NetworkMessage::Reply:
            {
                QTime time;
                qint32 snr;
                float delta_time;
                quint32 delta_frequency;
                QByteArray mode;
                QByteArray message;
                bool low_confidence {false};
                quint8 modifiers {0};
                in >> time >> snr >> delta_time >> delta_frequency >> mode >> message >> low_confidence >> modifiers;
                /*qDebug() <<"Reply: time:"<<time<<"snr:"<< snr<<"dt:"<<delta_time<<"df:"<<delta_frequency<<
                "mode:"<< mode <<"message:"<<message<<"low confidence:"<<low_confidence<<"modifiers: 0x"<<
                hex<<modifiers;*/
                if (check_status (in) != Fail)
                {
                    //Q_EMIT self_->reply (time, snr, delta_time, delta_frequency
                    //, QString::fromUtf8 (mode), QString::fromUtf8 (message)
                    //, low_confidence, modifiers);
                    QStringList list;
                    list<<time.toString("hhmmss")<<QString("%1").arg(snr)<<QString("%1").arg(delta_time,0,'f',1)<<
                    QString::fromUtf8(mode)<<QString::fromUtf8(message);
                    Q_EMIT self_->reply_clr(list);
                }
            }
            break;
        case NetworkMessage::Clear:
            {
                quint8 window {0};
                in >> window;
                //qDebug() <<"Clear window:"<<window;
                if (check_status (in) != Fail)
                {
                    //Q_EMIT self_->clear_decodes(window);
                    QStringList list;
                    list<<"CLR"<<QString("%1").arg(window);
                    Q_EMIT self_->reply_clr(list);
                }
            }
            break;
        case NetworkMessage::Replay:
            if (check_status (in) != Fail)
            {
                //last_message_.clear ();
                Q_EMIT self_->replay();
                //qDebug() << "APP Replay --->";
            }
            break;
        case NetworkMessage::HaltTx:
            {
                bool auto_only {false};
                in >> auto_only;
                //qDebug() <<"Halt Tx auto_only:" << auto_only;
                if (check_status (in) != Fail)
                {
                    Q_EMIT self_->halt_tx(auto_only);
                }
            }
            break;
        case NetworkMessage::Configure:
            {
                /*QByteArray mode;
                quint32 frequency_tolerance;
                QByteArray submode;
                bool fast_mode {false};
                quint32 tr_period;// {std::numeric_limits<quint32>::max ()};
                quint32 rx_df;// {std::numeric_limits<quint32>::max ()};
                QByteArray dx_call;
                QByteArray dx_grid;
                bool generate_messages {false};
                in >> mode >> frequency_tolerance >> submode >> fast_mode >> tr_period >> rx_df
                >> dx_call >> dx_grid >> generate_messages;

                //qDebug()<<"Configure mode:"<<mode<<"frequency tolerance:"<< frequency_tolerance << "submode:" <<
                // 		submode << "fast mode:" << fast_mode << "T/R period:" << tr_period << "rx df:" << rx_df <<
                // 		"dx call:" << dx_call << "dx grid:" << dx_grid << "generate messages:" << generate_messages;
                 		
                if (check_status (in) != Fail)
                {
                    //Q_EMIT self_->configure (QString::fromUtf8 (mode), frequency_tolerance
                    //                         , QString::fromUtf8 (submode), fast_mode, tr_period, rx_df
                    //                         , QString::fromUtf8 (dx_call), QString::fromUtf8 (dx_grid)
                    //                         , generate_messages);
                    QStringList list;
                    list<<QString::fromUtf8(mode)<<QString::fromUtf8(submode)<<QString::fromUtf8(dx_call)<<QString::fromUtf8(dx_grid);
                    Q_EMIT self_->configure(list,generate_messages);                
                }*/
            }
            break;
        case NetworkMessage::AnnotationInfo:
            {
                /*QByteArray dx_call;
                bool sort_order_provided{false};
                quint32 sort_order{std::numeric_limits<quint32>::max()};
                in >> dx_call >> sort_order_provided >> sort_order;
                TRACE_UDP ("External Callsign Info:" << dx_call << "sort_order_provided:" << sort_order_provided
                                                     << "sort_order:" << sort_order);
                if (sort_order > 50000) sort_order = 50000;
                if (check_status(in) != Fail) {
                  Q_EMIT
                  self_->annotation_info(QString::fromUtf8(dx_call), sort_order_provided, sort_order);
                }*/
            }
            break;
        default:
            if (NetworkMessage::Heartbeat != in.type ())
            {
                //qDebug() << "ignoring message type:" << in.type();
            }
            break;
        }
    }
    /*else
    {
    	//qDebug() << "ignored message for id:" << in.id();
    }*/
    /*}
    catch (std::exception const& e)
    {
        Q_EMIT self_->error (QString {"MessageClient exception: %1"}.arg (e.what ()));
    }
    catch (...)
    {
        Q_EMIT self_->error ("Unexpected exception in MessageClient");
    }*/
}
void MessageClient::impl::heartbeat()
{
    //qDebug()<<"SSSSS="<<server_port_<<server_;
    if (server_port_ && !server_.isNull())  // && f_emit_heartbeat_timer
    {
        QString revision_ = "";// in e.g. HEX vsjt = ce1c4f
        QByteArray message;
        NetworkMessage::Builder hb {&message, NetworkMessage::Heartbeat, id_, schema_};
        hb << NetworkMessage::Builder::schema_number // maximum schema number accepted = 3 >= qt5.4.0
        << version_.toUtf8() << revision_.toUtf8();  // fictive N1MM problem -> revision_.toUtf8()
        if (OK == check_status (hb))
        {
            //qDebug()<<"OK"<<message.data()<<id_<<version_.toUtf8()<<server_port_<<server_;
            writeDatagram (message, server_, server_port_);
        }
    }
}
void MessageClient::impl::closedown ()
{
    //qDebug()<<"MessageClient::impl::closedown ()";
    if (server_port_ && !server_.isNull ())
    {
        QByteArray message;
        NetworkMessage::Builder out {&message, NetworkMessage::Close, id_, schema_};
        if (OK == check_status (out))
        {
            writeDatagram (message, server_, server_port_);
        }
    }
}

auto MessageClient::impl::check_status (QDataStream const& stream) const -> StreamStatus
{
    auto stat = stream.status ();
    StreamStatus result {Fail};
    switch (stat)
    {
    case QDataStream::ReadPastEnd:
    result = Short;
    break;

    case QDataStream::ReadCorruptData:
    Q_EMIT self_->error ("Message serialization error: read corrupt data");
        break;

    case QDataStream::WriteFailed:
        Q_EMIT self_->error ("Message serialization error: write error");
        break;

    default:
        result = OK;
        break;
    }
    return result;
}
void MessageClient::impl::send_message(QDataStream const& out,QByteArray const& message)
{
    if (OK == check_status(out))
    {
        //qDebug() << "Message TX="<<(QString(message.toHex()));
        writeDatagram(message, server_, server_port_);
    }
    //else
    //Q_EMIT self_->error("Creating UDP message");
}

MessageClient::MessageClient (QString const& id, QString const& version, QString const& server,
                              port_type server_port, bool hbt, QObject * self)
        : QObject
{
    self
}
, m_ {id,version,server_port,hbt,this}
{
    //connect (&*m_, SIGNAL(ConectionInfo(QString)), this, SIGNAL(ConectionInfo(QString)));
    /*
        connect (&*m_, static_cast<void (impl::*) (impl::SocketError)> (&impl::error)
                 , [this] (impl::SocketError e)
                 {
    #if defined (Q_OS_WIN) && QT_VERSION >= 0x050500
                     if (e != impl::NetworkError 	// take this out when Qt 5.5
                             // stops doing this
                             // spuriously
                             && e != impl::ConnectionRefusedError) 	// not
                         // interested
                         // in this with
                         // UDP socket
    #else
                     Q_UNUSED (e);
    #endif
                     {
                         Q_EMIT error (m_->errorString ());
                     }
                 }
                );
    */
    connect (&*m_
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
             , static_cast<void (impl::*) (impl::SocketError)> (&impl::error), [this] (impl::SocketError e)
#else
             , &impl::errorOccurred, [this] (impl::SocketError e)
#endif
             {
#if defined (Q_OS_WIN)
                 if (e != impl::NetworkError // take this out when Qt 5.5 stops doing this spuriously
                         && e != impl::ConnectionRefusedError) // not interested in this with UDP socket
                 {
#else
                 {
                     Q_UNUSED (e);
#endif
                     Q_EMIT error (m_->errorString ());
                 }
             });

    set_server (server);
    //qDebug()<<"NetworkMessage::pulse * 1000";
}
QHostAddress MessageClient::server_address () const
{
    return m_->server_;
}
auto MessageClient::server_port () const -> port_type
{
    return m_->server_port_;
}
void MessageClient::set_server (QString const& server)
{
    m_->server_.clear ();
    m_->server_string_ = server;

    //if (!server.isEmpty())//old
    if (server.size())
    {
        //old QHostInfo::lookupHost (server, &*m_, SLOT (host_info_results(QHostInfo)));
        //m_->dns_lookup_id_ = QHostInfo::lookupHost (server, &*m_, SLOT (host_info_results(QHostInfo)));//2.46
        //qDebug() << "MessageClient lookupHost=" << m_->dns_lookup_id_;
#if QT_VERSION >= QT_VERSION_CHECK(5, 9, 0)
        m_->dns_lookup_id_ = QHostInfo::lookupHost(server, &*m_, &MessageClient::impl::host_info_results);
#else
        m_->dns_lookup_id_ = QHostInfo::lookupHost(server, &*m_, SLOT (host_info_results (QHostInfo)));
#endif
    }
}

void MessageClient::set_server_port (port_type server_port)
{
    m_->server_port_ = server_port;
}

//?? 2 rc5 qint64 MessageClient::send_raw_datagramHV(QByteArray const& message)
qint64 MessageClient::send_raw_datagramHV(QByteArray const& message) //,QHostAddress const& dest_address, port_type dest_port)
{
    //QHostAddress dest_address = m_->m_pskRepAdd;
    //qDebug() << "ffffffffff= " << m_->server_string_ <<  dest_address;

    if (m_->server_port_ && !m_->server_.isNull ())
    {
        return m_->writeDatagram (message, m_->server_, m_->server_port_);//

        //m_->writeDatagram (message, m_->server_string_, m_->server_port_);
        //qDebug() << "MessageClient Sent to=" << m_->server_ <<  m_->server_port_<<message.toHex();
    }
    return 0;
}
void MessageClient::logged_QSO(QStringList ls)
{
    if (m_->server_port_ && !m_->server_.isNull())
    {
        // QDateTime dateTime2 = QDateTime::fromString("M1d1y9800:01:02","'M'M'd'd'y'yyhh:mm:ss");
        // dateTime is 1 January 1998 00:01:02
        QDateTime t_off = QDateTime::fromString(ls.at(0),"yyyyMMdd hh:mm:ss");
        t_off.setTimeSpec(Qt::UTC);//2.68
        QDateTime t_on = QDateTime::fromString(ls.at(10),"yyyyMMdd hh:mm:ss");
        t_on.setTimeSpec(Qt::UTC);//2.68
        quint64 f = ls.at(3).toLongLong();

        QByteArray message;
        NetworkMessage::Builder out {&message, NetworkMessage::QSOLogged, m_->id_, m_->schema_};

        out << t_off << ls.at(1).toUtf8() << ls.at(2).toUtf8() << f << ls.at(4).toUtf8()
        << ls.at(5).toUtf8() << ls.at(6).toUtf8() << ls.at(7).toUtf8() << ls.at(8).toUtf8()
        << ls.at(9).toUtf8() << t_on << ls.at(11).toUtf8() << ls.at(12).toUtf8() << ls.at(13).toUtf8()
        << ls.at(14).toUtf8() << ls.at(15).toUtf8() << ls.at(16).toUtf8()
        << ls.at(17).toUtf8() << ls.at(18).toUtf8() << ls.at(19).toUtf8();//2.73

        m_->send_message(out, message);
        //m_->writeDatagram (message, m_->server_, m_->server_port_);
        //qDebug() << "MessageClient Sent to=" <<message<<message.size()<<ls.at(4).toUtf8()<<f;
        //qDebug() << ls;
    }
}
void MessageClient::logged_ADIF(QByteArray const& ADIF_record)
{
    //if (m_->server_port_ && !m_->server_string_.isEmpty ())
    if (m_->server_port_ && !m_->server_.isNull())
    {
        QByteArray message;
        NetworkMessage::Builder out {&message, NetworkMessage::LoggedADIF, m_->id_, m_->schema_};
        QByteArray ADIF {"\n<ADIF_VER:5>3.1.0\n<PROGRAMID:4>MSHV\n<EOH>\n" + ADIF_record}; //qDebug()<<ADIF;
        out << ADIF;
        m_->send_message(out, message);
        //m_->writeDatagram (message2, m_->server_, m_->server_port_);
        //qDebug() << "MessageClient Sent to=" << m_->server_<<m_->server_port_;
    }
}
void MessageClient::decodes_cleared()
{
    if (m_->server_port_ && !m_->server_.isNull())
    {
        QByteArray message;
        NetworkMessage::Builder out {&message, NetworkMessage::Clear, m_->id_, m_->schema_};
        m_->send_message (out, message);
    }
}
//#include <limits>
void MessageClient::statusUPD(quint64 f,QString mode,QString dx_call,QString report,QString tx_mode,
                              QString de_call,QString de_grid,QString dx_grid,bool decoding,QString sub_mode,
                              bool tx_enabled,bool transmitting,QString tx_message)
{
    if (m_->server_port_ && !m_->server_.isNull())
    {
        //qDebug()<<"statusUPD"<<dx_call<<report<<dx_grid;
        quint32 quint32_max = std::numeric_limits<quint32>::max(); //qDebug()<<quint32_max;
        //quint64 f;
        //QString mode;
        //QString dx_call;
        //QString report;
        //QString tx_mode;
        //bool tx_enabled = true;  //auto on
        //bool transmitting = false;//tx
        //bool decoding = true;
        quint32 rx_df = 0;//1200;
        quint32 tx_df = 0;//1200;
        //QString de_call;
        //QString de_grid;
        //QString dx_grid;
        bool watchdog_timeout = false;
        //QString sub_mode = "";
        bool fast_mode = false;

        quint8 special_op_mode = 0;
        /*case 1: special = "[NA VHF]"; break;
        case 2: special = "[EU VHF]"; break;
        case 3: special = "[FD]"; break;
        case 4: special = "[RTTY RU]"; break;
        case 5: special = "[Fox]"; break;
        case 6: special = "[Hound]"; break;*/

        quint32 frequency_tolerance = quint32_max;//<1000
        quint32 tr_period = quint32_max;
        QString configuration_name = "Default";
        //QString tx_message = "N/A";//2.52

        QByteArray message;
        NetworkMessage::Builder out {&message, NetworkMessage::Status, m_->id_, m_->schema_};
        out << f << mode.toUtf8 () << dx_call.toUtf8 () << report.toUtf8 () << tx_mode.toUtf8 ()
        << tx_enabled << transmitting << decoding << rx_df << tx_df << de_call.toUtf8 ()
        << de_grid.toUtf8 () << dx_grid.toUtf8 () << watchdog_timeout << sub_mode.toUtf8 ()
        << fast_mode << special_op_mode << frequency_tolerance << tr_period << configuration_name.toUtf8()
        << tx_message.toUtf8();

        m_->send_message (out, message);
    }
}
void MessageClient::decode_TXT(bool is_new,QString tim,int sn,QString dt,int frq,QString mode,QString message_text)
{
    if (m_->server_port_ && !m_->server_.isNull())
    {
        //bool is_new = true;//2.22 need to be true
        QTime time = QTime::fromString(tim,"hhmmss");
        quint32 snr = (quint32)sn;
        float delta_time = (float)dt.toDouble();
        quint32 delta_frequency = (quint32)frq;
        //QString mode = mod; // + ~ .....
        //QString message_text = msg;
        bool low_confidence = false;
        bool off_air = false;//fopen

        QByteArray message;
        NetworkMessage::Builder out {&message, NetworkMessage::Decode, m_->id_, m_->schema_};
        out << is_new << time << snr << delta_time << delta_frequency << mode.toUtf8 ()
        << message_text.toUtf8 () << low_confidence << off_air;

        m_->send_message(out, message);
        //qDebug() << "MessageClient Sent to=" <<message_text;
    }
}
