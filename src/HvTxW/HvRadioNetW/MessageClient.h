 /* MSHV 
 * WSJT-X MessageClient created by G4WJS
 * and modified for MSHV
 * May be used under the terms of the GNU General Public License (GPL)
 */

#ifndef MESSAGECLIENT_H
#define MESSAGECLIENT_H

#include "../../config.h"

#include <QObject>
//#include <QTime>
#include <QDateTime>
#include <QString>
#include <QStringList>

//#include "Radio.hpp"

#include "pimpl_h.h"


class QByteArray;
class QHostAddress;

//
// MessageClient - Manage messages sent and replies received from a
//                 matching server (MessageServer) at the other end of
//                 the wire
//
//
// Each outgoing message type is a Qt slot
//
class MessageClient
            : public QObject
{
    Q_OBJECT

public:
    using Frequency = quint64;//Radio::Frequency;
    using port_type = quint16;

    // instantiate and initiate a host lookup on the server
    //
    // messages will be silently dropped until a server host lookup is complete
    MessageClient (QString const& id, QString const& version, QString const& server, port_type server_port, bool hbt, QObject * parent = nullptr);

    void logged_QSO(QStringList ls);
    void logged_ADIF(QByteArray const& ADIF_record); 
    void decode_TXT(bool is_new,QString tim,int sn,QString dt,int frq,QString mod,QString msg); 
    void statusUPD(quint64 f,QString mode,QString dx_call,QString report,QString tx_mode,
				QString de_call,QString de_grid,QString dx_grid,bool decoding,QString sub_mode,bool,
				bool,QString);
    void decodes_cleared();

    Q_SIGNAL void ConectionInfo(QString);
    // query server details
    QHostAddress server_address () const;
    port_type server_port () const;

    // initiate a new server host lookup or is the server name is empty
    // the sending of messages is disabled
    Q_SLOT void set_server (QString const& server = QString {});

    // change the server port messages are sent to
    Q_SLOT void set_server_port (port_type server_port = 0u);

    qint64 send_raw_datagramHV (QByteArray const& message);

    // this signal is emitted when network errors occur or if a host
    // lookup fails
    Q_SIGNAL void error (QString const&) const;
    Q_SIGNAL void replay(); 
    Q_SIGNAL void reply_clr(QStringList); //<---is here Q_SIGNAL void clear_decodes(int); 
    Q_SIGNAL void halt_tx(bool);   
  	//Q_SIGNAL void reply (QTime, qint32 snr, float delta_time, quint32 delta_frequency, QString const& mode
                       //, QString const& message_text, bool low_confidence, quint8 modifiers);  	
    Q_SIGNAL void configure(QStringList,bool);
    
    //Q_SIGNAL void annotation_info(QString const& dx_call, bool sort_order_provided, quint32 sort_order);

    //bool f_sta_sto_rep_timers;
    //Q_SLOT void set_emit_heartbeat_timer(bool);

private:
    class impl;
    pimpl<impl> m_;


};

#endif
