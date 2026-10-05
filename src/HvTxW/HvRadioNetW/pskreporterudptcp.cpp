// Interface for posting spots to PSK Reporter web site
// Implemented by Edson Pereira PY2SDR
// Updated by Bill Somerville, G4WJS
// Reports will be sent in batch mode every 5 minutes.
// Updated For MSHV by Hrisimir Hristov, LZ2HV 2020
#include "pskreporterudptcp.h"

#include <QQueue>
#include <QDateTime>
#include <QTimer>
#include <QDataStream>
#include <QSharedPointer>
#define QT_NO_DEBUG_STREAM // remove qdebug from tcp socet
#include <QTcpSocket>
#include <QUdpSocket>

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)//??? for Linux need check
#include <QRandomGenerator>
#endif

#include "pimpl_impl.h"
#include "moc_pskreporterudptcp.cpp"

//#include <QtGui>
namespace
{
//QLatin1String HOST {"report.pskreporter.info"};
//QLatin1String HOST {"127.0.0.1"};
//quint16 SERVICE_PORT {4739};
//quint16 SERVICE_PORT {14739};
//int MIN_SEND_INTERVAL {15}; // in seconds
//int FLUSH_INTERVAL {4 * 5}; // in send intervals
bool ALIGNMENT_PADDING {true};
//int MIN_PAYLOAD_LENGTH {508};
int MAX_PAYLOAD_LENGTH {10000};//2.74 old=1400    2.70rc1=10000  2.61=1400
//int MAX_PAYLOAD_LENGTH {508};
}
class PSKReporter::impl : public QObject
{
    Q_OBJECT
public:
    impl (PSKReporter * self, bool udptcp, QString const& program_info,QString const& hostt, quint16 portt)
            : self_
    {
        self
    }
    , udptcp_ {udptcp}
    , sequence_number_ {0u}
    , send_descriptors_ {0}
    , send_receiver_data_ {0}
    , flush {false}
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    ,observation_id_ {(quint32)qrand()} //2.74
#else
    ,observation_id_ {(quint32)QRandomGenerator::global()->generate()} //2.74
#endif
    , prog_id_ {program_info}
    , HOST {hostt}
    , SERVICE_PORT {portt}
    , try1_if_discon {false}
    {
        //#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)//for linux (5, 15, 2)  v.qt5-5,15,1 not working ???
        /*
        #if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
                observation_id_ = qrand();
        #else
                observation_id_ = QRandomGenerator::global()->generate ();
        #endif
        */
        // This timer sets the interval to check for spots to send.
		connect (&report_timer_, &QTimer::timeout, [this] (){send_report();});
        // This timer repeats the sending of IPFIX templates and receiver
        // information if we are using UDP, in case server has been
        // restarted ans lost cached information.
        connect (&descriptor_timer_hv, &QTimer::timeout, [this] ()
                 {
                     if (socket_ && QAbstractSocket::UdpSocket == socket_->socketType())
                     {
                         cou_1h++;
                         if (cou_1h>59)//1*60=60min  1min refresh
                         {
                             //qDebug()<<"1h----->"<<QTime::currentTime().toString("hh:mm:ss");
                             cou_1h = 0;
                             // send templates again
                             send_descriptors_ = 3; // three times
                             // send receiver data set again
                             send_receiver_data_ = 3; // three times
                         }
                     }
                     cou_5m++;
                     if (cou_5m>4)//5min 1min refresh
                     {
                         //qDebug()<<"5m-->"<<QTime::currentTime().toString("mm:ss");
                         cou_5m = 0;
                         flush = true;
                         if (!report_timer_.isActive()) report_timer_.start(2000);//hv
                     }
                 });
        connect (&recon_timer_hv, &QTimer::timeout, [this] (){reconnect();});
        recon_timer_hv.setSingleShot(true);
    }
    /*void check_connection()
    {
        if (!socket_ || QAbstractSocket::UnconnectedState == socket_->state () 
        		|| (socket_->socketType () != udptcp_ ? QAbstractSocket::TcpSocket : QAbstractSocket::UdpSocket))
        {
            // we need to create the appropriate socket
            if (socket_ && QAbstractSocket::UnconnectedState != socket_->state () && QAbstractSocket::ClosingState != socket_->state ())
            {
                // handle re-opening asynchronously
                auto connection = QSharedPointer<QMetaObject::Connection>::create ();
                *connection = connect (socket_.data (), &QAbstractSocket::disconnected, [this, connection] ()
                                       {
                                           disconnect (*connection);
                                           check_connection ();
                                       }
                                      );
                // close gracefully
                send_report();//true
                socket_->close();
            }
            else reconnect();
        }
    }*/
    void handle_socket_error(QAbstractSocket::SocketError e)
    {
        //qDebug()<<"EROR"<<e;
        switch (e)
        {
        case QAbstractSocket::RemoteHostClosedError:
            //try1_if_discon = false;//???
            socket_->disconnectFromHost();
            break;
        case QAbstractSocket::TemporaryError:
            break;
        default:
            spots_.clear (); //Q_EMIT self_->errorOccurred(socket_->errorString ());//hv stop
            break;
        }
    }
    void reconnect()
    {
        //qDebug()<<"RRReconnect";
        recon_timer_hv.stop();
        // Using deleteLater for the deleter as we may eventually
        // be called from the disconnected handler above.
        if (udptcp_)
        {
            socket_.reset (new QTcpSocket, &QObject::deleteLater);
            send_descriptors_ = 1;
            send_receiver_data_ = 1;
        }
        else
        {
            socket_.reset (new QUdpSocket, &QObject::deleteLater);
            send_descriptors_ = 3;
            send_receiver_data_ = 3;
        }

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        connect (socket_.get(), &QAbstractSocket::errorOccurred, this, &PSKReporter::impl::handle_socket_error);
#else
        //connect (socket_.data(), QOverload<QAbstractSocket::SocketError>::of (&QAbstractSocket::error), this, &PSKReporter::impl::handle_socket_error);
        connect (socket_.data(), static_cast<void(QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error), this, &PSKReporter::impl::handle_socket_error);
        //connect(socket_.data(), SIGNAL(error(QAbstractSocket::SocketError)),this, SLOT(handle_socket_error(QAbstractSocket::SocketError)));
#endif

        //connect(socket_.data(), SIGNAL(connected()), this, SLOT(connected_s()));
        //connect(socket_.data(), SIGNAL(disconnected()), this, SLOT(disconnected_s()));
        connect(socket_.data(), SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(StateChanged(QAbstractSocket::SocketState)));

        // use this for pseudo connection with UDP, allows us to use
        // QIODevice::write() instead of QUDPSocket::writeDatagram()
        socket_->connectToHost(HOST, SERVICE_PORT, QAbstractSocket::WriteOnly);
        socket_->waitForConnected(350);//importent for TCP 350ms hv

        /*if (!report_timer_.isActive ())//hv stop
        {
            report_timer_.start (MIN_SEND_INTERVAL * 1000);
            //report_timer_.start (500);
        }*/
        if (!descriptor_timer_hv.isActive())
        {
            //descriptor_timer_.start(1 * 60 * 60 * 1000); // hourly
            descriptor_timer_hv.start(60000);//1min ->1*60*1000=60000
            cou_1h = 0;
            cou_5m = 0;
        }
        //if (socket_->state() == QAbstractSocket::ConnectedState) qDebug()<<"reconnect()=TRUE";
        //else  qDebug()<<"reconnect()=FALSE";
    }

    void stop()
    {
        //qDebug()<<"SSStop";
        try1_if_discon = false;
        recon_timer_hv.stop();
        descriptor_timer_hv.stop();//hv move here
        report_timer_.stop();//hv move here
        cou_1h = 0;
        cou_5m = 0;
        if (socket_)
        {
            socket_->disconnectFromHost();
            if (socket_->state() == QAbstractSocket::UnconnectedState || socket_->waitForDisconnected(350))//importent for TCP 350ms hv
            {}//hv
        }
    }

    void send_report();//bool send_residue = false
    void build_preamble(QDataStream&);

    /*bool flushing()
    {
    	//qDebug()<<std::numeric_limits<unsigned>::max(); //4294967296/2  2147483648 
    if (flush_counter_ > 4000000) flush_counter_ = 0u; //+1 0x7ffffd78  2000000 2 million
        return FLUSH_INTERVAL && !(++flush_counter_ % FLUSH_INTERVAL);// evry 20-0
    }*/

    //Q_SLOT void connected_s();
    //Q_SLOT void disconnected_s();
    Q_SLOT void StateChanged(QAbstractSocket::SocketState);
    PSKReporter * self_;
    bool udptcp_;
    QSharedPointer<QAbstractSocket> socket_;
    //int dns_lookup_id_;
    QByteArray payload_;
    quint32 sequence_number_;
    int send_descriptors_;

    // Currently PSK Reporter requires that  a receiver data set is sent
    // in every  data flow. This  memeber variable  can be used  to only
    // send that information at session start (3 times for UDP), when it
    // changes (3  times for UDP), or  once per hour (3  times) if using
    // UDP. Uncomment the relevant code to enable that fuctionality.
    int send_receiver_data_;

    bool flush;
    int cou_1h;
    int cou_5m;

    //unsigned flush_counter_;
    quint32 observation_id_;
    QString rx_call_;
    QString rx_grid_;
    QString rx_ant_;
    QString rigInformation_;//2.76.1
    QString prog_id_;
    QByteArray tx_data_;
    QByteArray tx_residue_;
    struct Spot
    {
        bool operator == (Spot const& rhs)
        {
            return
                call_ == rhs.call_
                && grid_ == rhs.grid_
                && mode_ == rhs.mode_
                && freq_ == rhs.freq_; //&& std::abs (Radio::FrequencyDelta (freq_ - rhs.freq_)) < 50;
        }

        QString call_;
        QString grid_;
        int snr_;
        long long int freq_; //Radio::Frequency freq_;
        QString mode_;
        QDateTime time_;
    };
    QQueue<Spot> spots_;
    QTimer report_timer_;
    QTimer descriptor_timer_hv;
    QTimer recon_timer_hv;
    QString HOST;
    quint16 SERVICE_PORT;
    bool try1_if_discon;
};

#include "pskreporterudptcp.moc"

namespace
{
void writeUtfString(QDataStream& out, QString const& s)
{
    auto const& utf = s.toUtf8 ().left (254);
    out << quint8 (utf.size ());
    out.writeRawData (utf, utf.size ());
}
int num_pad_bytes(int len)
{
    return ALIGNMENT_PADDING ? (4 - len % 4) % 4 : 0;
}
void set_length(QDataStream& out, QByteArray& b)
{
    // pad with nulls modulo 4
    auto pad_len = num_pad_bytes (b.size ());
    out.writeRawData (QByteArray {pad_len, '\0'}.constData (), pad_len);
    auto pos = out.device ()->pos ();
    out.device ()->seek (sizeof (quint16));
    // insert length
    out << static_cast<quint16> (b.size ());
    out.device ()->seek (pos);
}
}
/*
void PSKReporter::impl::connected_s()
{
    //Q_EMIT self_->ConectionInfo("<font color='green'>Connected to "+host_info.hostName()+"  IP "+server_.toString()+"</font>");
    QString type = "Via UDP";
    if (udptcp_) type = "Via TCP";
    //Q_EMIT self_->ConectionInfo("<font color='green'>Connected to "+HOST+" "+type+"</font>");
    qDebug()<<"conected";
}
void PSKReporter::impl::disconnected_s()
{
    //Q_EMIT self_->ConectionInfo("<font color='red'>UDP server lookup failed: " + host_info.errorString()+"</font>");
    //QString type = "UDP";
    //if (udptcp_) type = "TCP";
    //Q_EMIT self_->ConectionInfo("<font color='red'>Disconnected</font>");
    qDebug()<<"disconnected";
}
*/
void PSKReporter::impl::StateChanged(QAbstractSocket::SocketState e)//hv add
{
    QString type = tr("Via UDP");
    if (udptcp_) type = tr("Via TCP");
    QString por = QString("%1").arg(SERVICE_PORT);
    QString hrt = HOST+":"+por+" "+type;
    if 		(e==QAbstractSocket::UnconnectedState) Q_EMIT self_->ConectionInfo("<font color='red'>Unconnected State "+hrt+"</font>");
    else if (e==QAbstractSocket::HostLookupState)  Q_EMIT self_->ConectionInfo("<font color='#00b300'>Host Lookup to "+hrt+"</font>");
    else if (e==QAbstractSocket::ConnectingState)  Q_EMIT self_->ConectionInfo("<font color='#00b300'>Connecting to "+hrt+"</font>");
    else if (e==QAbstractSocket::ConnectedState)   Q_EMIT self_->ConectionInfo("<font color='#00b300'>"+tr("Connected to")+" "+hrt+"</font>");
    else if (e==QAbstractSocket::BoundState)       Q_EMIT self_->ConectionInfo("<font color='#00b300'>Bound to "+hrt+"</font>");
    else if (e==QAbstractSocket::ClosingState)     Q_EMIT self_->ConectionInfo("<font color='#00b300'>Closing "+hrt+"</font>");
    else if (e==QAbstractSocket::ListeningState)   Q_EMIT self_->ConectionInfo("<font color='#00b300'>Listening "+hrt+"</font>");
    else 										   Q_EMIT self_->ConectionInfo("<font color='red'>Unconnected State "+hrt+"</font>");
    //qDebug()<<"StateChanged======"<<e<<try1_if_discon;
    if (try1_if_discon && e==QAbstractSocket::UnconnectedState)
    {
        try1_if_discon = false; //qDebug()<<"From StateChanged Reconneect Start";
        recon_timer_hv.start(30000);//after 30 sec
    }
    if (e==QAbstractSocket::ConnectedState) try1_if_discon = true;
}
void PSKReporter::impl::build_preamble(QDataStream& message)
{
    //qDebug()<<std::numeric_limits<quint32>::max(); //4294967296/2=2147483648
    if (sequence_number_ > 8000000) sequence_number_ = 2;//2.74 old=0 old>4000000 0x7ffffd78

    // Message Header
    message
    << quint16 (10u)          // Version Number
    << quint16 (0u)           // Length (place-holder filled in later)
    << quint32 (0u)           // Export Time (place-holder filled in later)
    << ++sequence_number_     // Sequence Number
    << observation_id_;       // Observation Domain ID

    if (send_descriptors_)
    {
        --send_descriptors_;
        if (send_descriptors_ < 0) send_descriptors_ = 0;//2.74 protection
        {
            // Sender Information descriptor
            QByteArray descriptor;
            QDataStream out {&descriptor, QIODevice::WriteOnly};
            out
            << quint16 (2u)           // Template Set ID
            << quint16 (0u)           // Length (place-holder)
            << quint16 (0x50e3)       // Link ID
            << quint16 (7u)           // Field Count
            << quint16 (0x8000 + 1u)  // Option 1 Information Element ID (senderCallsign)
            << quint16 (0xffff)       // Option 1 Field Length (variable)
            << quint32 (30351u)       // Option 1 Enterprise Number
            << quint16 (0x8000 + 5u)  // Option 2 Information Element ID (frequency)
            << quint16 (5u)           // Option 2 Field Length //2.68 new=5u  //<< quint16 (4u)// Option 2 Field Length //2.68 old=4u
            << quint32 (30351u)       // Option 2 Enterprise Number
            << quint16 (0x8000 + 6u)  // Option 3 Information Element ID (sNR)
            << quint16 (1u)           // Option 3 Field Length
            << quint32 (30351u)       // Option 3 Enterprise Number
            << quint16 (0x8000 + 10u) // Option 4 Information Element ID (mode)
            << quint16 (0xffff)       // Option 4 Field Length (variable)
            << quint32 (30351u)       // Option 4 Enterprise Number
            << quint16 (0x8000 + 3u)  // Option 5 Information Element ID (senderLocator)
            << quint16 (0xffff)       // Option 5 Field Length (variable)
            << quint32 (30351u)       // Option 5 Enterprise Number
            << quint16 (0x8000 + 11u) // Option 6 Information Element ID (informationSource)
            << quint16 (1u)           // Option 6 Field Length
            << quint32 (30351u)       // Option 6 Enterprise Number
            << quint16 (150u)         // Option 7 Information Element ID (dateTimeSeconds)
            << quint16 (4u);          // Option 7 Field Length
            // insert Length and move to payload
            set_length(out, descriptor);
            message.writeRawData(descriptor.constData(), descriptor.size());
        }
        {
            // Receiver Information descriptor
            QByteArray descriptor;
            QDataStream out {&descriptor, QIODevice::WriteOnly};
            out
            << quint16 (3u)          // Options Template Set ID
            << quint16 (0u)          // Length (place-holder)
            << quint16 (0x50e2)      // Link ID
            << quint16 (5u)          //Field Count 2.76.1 //<< quint16 (4u)//old Field Count
            << quint16 (0u)          // Scope Field Count
            << quint16 (0x8000 + 2u) // Option 1 Information Element ID (receiverCallsign)
            << quint16 (0xffff)      // Option 1 Field Length (variable)
            << quint32 (30351u)      // Option 1 Enterprise Number
            << quint16 (0x8000 + 4u) // Option 2 Information Element ID (receiverLocator)
            << quint16 (0xffff)      // Option 2 Field Length (variable)
            << quint32 (30351u)      // Option 2 Enterprise Number
            << quint16 (0x8000 + 8u) // Option 3 Information Element ID (decodingSoftware)
            << quint16 (0xffff)      // Option 3 Field Length (variable)
            << quint32 (30351u)      // Option 3 Enterprise Number
            << quint16 (0x8000 + 9u) // Option 4 Information Element ID (antennaInformation)
            << quint16 (0xffff)      // Option 4 Field Length (variable)
            << quint32 (30351u)      // Option 5 Enterprise Number added->2.76.1
            << quint16 (0x8000 + 13u)// Option 5 Information Element ID (rigInformation) added->2.76.1
            << quint16 (0xffff)      // Option 5 Field Length (variable) added->2.76.1
            << quint32 (30351u);     // Option 4 Enterprise Number
            // insert Length
            set_length(out, descriptor);
            message.writeRawData(descriptor.constData(), descriptor.size());
        }
    }

    // if (send_receiver_data_)
    {
        // --send_receiver_data_;
        // Receiver information
        QByteArray data;
        QDataStream out {&data, QIODevice::WriteOnly};

        // Set Header
        out
        << quint16 (0x50e2)     // Template ID
        << quint16 (0u);        // Length (place-holder)

        // Set data
        writeUtfString(out, rx_call_);
        writeUtfString(out, rx_grid_);
        writeUtfString(out, prog_id_);
        writeUtfString(out, rx_ant_);
        writeUtfString(out, rigInformation_);

        // insert Length and move to payload
        set_length(out, data);
        message.writeRawData(data.constData (),data.size ());
    }
}
void PSKReporter::impl::send_report()//bool send_residue
{
    if (QAbstractSocket::ConnectedState != socket_->state() || (spots_.isEmpty() && tx_residue_.isEmpty() && !flush))
    {
        report_timer_.stop(); //qDebug()<<"Timer Stop Empty ---";
        return;
    }

    QDataStream message {&payload_, QIODevice::WriteOnly | QIODevice::Append};
    QDataStream tx_out  {&tx_data_, QIODevice::WriteOnly | QIODevice::Append};

    // Build header, optional descriptors, and receiver information
    if (!payload_.size()) build_preamble(message);

    /*auto flush = flushing() || send_residue;
    bool flush = send_residue;
    while (spots_.size() || flush)
    {
    if (!payload_.size())// have up, no needed hv
    {
        // Build header, optional descriptors, and receiver information
        build_preamble(message);
    }*/

    //qDebug()<<"payload_"<<payload_.size();

    if (!tx_data_.size() && (spots_.size() || tx_residue_.size()))
    {
        tx_out					// Set Header
        << quint16 (0x50e3)     // Template ID
        << quint16 (0u);        // Length (place-holder)
    }

    // insert any residue
    if (tx_residue_.size())
    {
        tx_out.writeRawData(tx_residue_.constData (), tx_residue_.size ());
        tx_residue_.clear(); //qDebug()<<"tx_residue_.clear()...";
    }

    //qDebug()<<"spots_.size()="<<spots_.size();
    while (spots_.size() || flush)// || flush
    {
        auto tx_data_size = tx_data_.size();
        if (spots_.size())
        {
            auto const& spot = spots_.dequeue();
            //Sender information
            //qDebug()<<spot.call_<<spot.mode_;
            //cc++;
            writeUtfString(tx_out,spot.call_);

            uint8_t data[5];
            long long int i64 = spot.freq_;
            data[0] = ( i64 & 0xff);
            data[1] = ((i64 >>  8) & 0xff);
            data[2] = ((i64 >> 16) & 0xff);
            data[3] = ((i64 >> 24) & 0xff);
            data[4] = ((i64 >> 32) & 0xff);
            tx_out // BigEndian
            << static_cast<uint8_t> (data[4])
            << static_cast<uint8_t> (data[3])
            << static_cast<uint8_t> (data[2])
            << static_cast<uint8_t> (data[1])
            << static_cast<uint8_t> (data[0])
            //tx_out
            //<< static_cast<quint32> (spot.freq_) //2.68 stoped
            << static_cast<qint8> (spot.snr_);
            writeUtfString(tx_out, spot.mode_);
            writeUtfString(tx_out, spot.grid_);
            tx_out
            << quint8 (1u)          // REPORTER_SOURCE_AUTOMATIC
            << static_cast<quint32> (
#if QT_VERSION >= QT_VERSION_CHECK(5, 8, 0)
                spot.time_.toSecsSinceEpoch()
#else
                spot.time_.toMSecsSinceEpoch() / 1000
#endif
            );
        }

        auto len = payload_.size() + tx_data_.size();
        len += num_pad_bytes(tx_data_.size());
        len += num_pad_bytes(len);
        /*if (len > MAX_PAYLOAD_LENGTH // our upper datagram size limit
                || (!spots_.size () && len > MIN_PAYLOAD_LENGTH) // spots drained and above lower datagram size limit
                || (flush && !spots_.size())) // send what we have, possibly no spots*/
        if (len > MAX_PAYLOAD_LENGTH || !spots_.size ())
        {   //qDebug()<<len;
            if (tx_data_.size())
            {
                if (len <= MAX_PAYLOAD_LENGTH)
                {
                    tx_data_size = tx_data_.size();
                }
                QByteArray tx {tx_data_.left (tx_data_size)};
                QDataStream out {&tx, QIODevice::WriteOnly | QIODevice::Append};
                // insert Length
                set_length(out, tx);
                message.writeRawData (tx.constData(), tx.size());
            }

            // insert Length and Export Time
            set_length(message, payload_);
            message.device()->seek(2 * sizeof(quint16));
            message << static_cast<quint32> (
#if QT_VERSION >= QT_VERSION_CHECK(5, 8, 0)
                QDateTime::currentDateTime().toSecsSinceEpoch()
#else
                QDateTime::currentDateTime().toMSecsSinceEpoch() / 1000
#endif
            );

            // Send data to PSK Reporter site
            socket_->write(payload_); // TODO: handle errors
            //qDebug()<<"Send Spot-------->>>"<<sequence_number_<<len<<payload_.size()<<spots_.size()<<QTime::currentTime().toString("hh:mm:ss");
            flush = false; //hv
            cou_5m = 0;
            message.device()->seek(0u);
            payload_.clear();  // Fresh message
            // Save unsent spots
            tx_residue_ = tx_data_.right(tx_data_.size() - tx_data_size);
            tx_out.device()->seek(0u);
            tx_data_.clear();
            break;
        }
    } //qDebug()<<"Send Spot End"<<spots_.size()<<payload_.size()<<tx_data_.size()<<tx_residue_.size();
    /*}*/
    report_timer_.start(2000);//if needed next timing faster 2s,
}
//// impl end ///////
PSKReporter::PSKReporter(bool fut,QString const& program_info,QString const& host0,quint16 port0)
        : m_
{
    this,fut,program_info,host0,port0
}
{}
PSKReporter::~PSKReporter()
{}
void PSKReporter::reconnect()
{
    m_->try1_if_discon = false;
    m_->reconnect();
}
void PSKReporter::stop()
{
    m_->stop();
}
void PSKReporter::setLocalStation(QString const& call,QString const& gridSquare,QString const& antenna,QString const& rigInformation)
{
    //m_->check_connection ();//hv stop
    if (call != m_->rx_call_ || gridSquare != m_->rx_grid_ || antenna != m_->rx_ant_ || rigInformation != m_->rigInformation_)
    {
        m_->send_receiver_data_ = m_->socket_ && QAbstractSocket::UdpSocket == m_->socket_->socketType () ? 3 : 1;
        m_->rx_call_ = call;
        m_->rx_grid_ = gridSquare;
        m_->rx_ant_ = antenna;
        m_->rigInformation_ = rigInformation; //qDebug()<<call<<gridSquare<<antenna<<rigInformation;//2.76.1
    }
}
void PSKReporter::addRemoteStation(QString call,QString grid,QString freq,QString mode,int snr,QString hhmmsspt)
{
    //m_->check_connection ();//hv stop
    if (m_->rx_call_.isEmpty() || m_->rx_grid_.isEmpty()) return;
    if (m_->socket_ && m_->socket_->isValid ())
    {
        if (QAbstractSocket::UnconnectedState == m_->socket_->state ())/* && !m_->recon_timer_hv.isActive()*/
        {
            m_->try1_if_discon = false; //qDebug()<<"From addRemoteStation Reconneect Start";
            m_->recon_timer_hv.start(5000);
        }
        if (QAbstractSocket::ConnectedState == m_->socket_->state())
        {
            //QString s;
            const int spot_max = 250;//2.74 protection Overload m_->spots_
            if (m_->spots_.size() > spot_max)
            {
                //for (int i = 0; i < m_->spots_.size(); ++i) s.append(m_->spots_.at(i).call_+",");
                //qDebug()<<"ALL"<<m_->spots_.size()<<s; s.clear();
                for (int i = 0; i < spot_max / 3; ++i) m_->spots_.dequeue();
                //for (int i = 0; i < m_->spots_.size(); ++i) s.append(m_->spots_.at(i).call_+",");
                //qDebug()<<"REM"<<m_->spots_.size()<<s;
            }
            QStringList hhmmss_pt = hhmmsspt.split("#");//2.76.4 //hhmmss_pt.clear(); qDebug()<<hhmmss_pt<<hhmmss_pt.count();
            /*if (hhmmss_pt.count()<2)//protection
            {
            	hhmmss_pt.clear();
            	hhmmss_pt<<"NA"<<"15";
           	}*/
            QTime t = QTime::fromString(hhmmss_pt.at(0),"hhmmss"); //qDebug()<<hhmmss_pt<<t.isValid();
            if (hhmmss_pt.at(0)=="NA" || !t.isValid()) m_->spots_.enqueue({call,grid,snr,freq.toLongLong(),mode,QDateTime::currentDateTimeUtc()});
            else
            {                
                QDateTime SpotTime1;//QDateTime SpotTime1 = QDateTime(QDateTime::currentDateTimeUtc().date(),t,Qt::UTC);
                float times = hhmmss_pt.at(0).toFloat();
                float ptime = hhmmss_pt.at(1).toFloat();
                if (times + ptime < 236000.0) SpotTime1 = QDateTime(QDateTime::currentDateTimeUtc().date(),t,Qt::UTC);
                else SpotTime1 = QDateTime((QDateTime::currentDateTimeUtc().addDays(-1)).date(),t,Qt::UTC);
                m_->spots_.enqueue({call,grid,snr,freq.toLongLong(),mode,SpotTime1});
                //QDateTime SpotTime0 = QDateTime::currentDateTimeUtc();
                //qDebug() << call << "Format 0 =" << SpotTime0.toString("yyyy-MM-dd hh:mm:ss") << QString("%1").arg((times + ptime),0,'f',2) << times << ptime;
                //qDebug() << call << "Format 1 =" << SpotTime1.toString("yyyy-MM-dd hh:mm:ss") << QString("%1").arg((times + ptime),0,'f',2) << times << ptime;
                //qDebug() << "------------------------------------";
            }
            //s.clear();
            //for (int i = 0; i < m_->spots_.size(); ++i) s.append(m_->spots_.at(i).call_+",");
            //qDebug()<<"ADD"<<m_->spots_.size()<<s;
            //qDebug()<<m_->spots_.size()<<"-----------------------------------------------";
            if (mode=="FT4") m_->report_timer_.start(3000); //for 7.5s period
            else if (mode=="FT2") m_->report_timer_.start(2000); //for 3.75s period
            else m_->report_timer_.start(4400);//2.70 old=4500
        }
    }
}
void PSKReporter::set_server(QString h)
{
    m_->HOST = h;
}
void PSKReporter::set_server_port(int p)
{
    m_->SERVICE_PORT = p;
}
void PSKReporter::set_udp_tcp(bool f)
{
    m_->udptcp_ = f;
}
/*
bool PSKReporter::addRemoteStation(QString call, QString grid,  QString freq, QString mode, int snr)
{
    m_->check_connection ();//hv stop
    if ( m_->rx_call_.isEmpty() || m_->rx_grid_.isEmpty() ) return false;
    if (m_->socket_ && m_->socket_->isValid ())
    {
        if (QAbstractSocket::UnconnectedState == m_->socket_->state ()) reconnect();
        m_->spots_.enqueue ({call, grid, snr, freq.toLongLong(), mode, QDateTime::currentDateTimeUtc()});
        if (!m_->report_timer_.isActive()) //qDebug()<<"Timer Start +++";
        m_->report_timer_.start(4000);  //4s
        return true;
    }
    return false;
}
*/
/*void PSKReporter::sendReport(bool last)
{
    m_->check_connection ();
    if (m_->socket_ && QAbstractSocket::ConnectedState == m_->socket_->state ())
    {
        m_->send_report(true);
    }
    if (last)
    {
        m_->stop();
    }
}*/
