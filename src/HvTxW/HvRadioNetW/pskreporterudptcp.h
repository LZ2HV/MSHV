// Interface for posting spots to PSK Reporter web site
// Implemented by Edson Pereira PY2SDR
// Updated by Bill Somerville, G4WJS
// Reports will be sent in batch mode every 5 minutes.
// Updated For MSHV by Hrisimir Hristov, LZ2HV 2020

#ifndef PSKREPORTERUDPTCP_H
#define PSKREPORTERUDPTCP_H

#include <QObject>
#include <QString>

#include "pimpl_h.h" 

class PSKReporter : public QObject
{
    Q_OBJECT
public:
    explicit PSKReporter (bool udptcp, QString const& program_info,QString const&,quint16);
    ~PSKReporter ();
    void reconnect();
    void stop();//hv add
    void setLocalStation(QString const& call, QString const& grid, QString const& antenna,QString const& ri);
    //
    // Returns false if PSK Reporter connection is not available
    //
    //bool addRemoteStation (QString const& call, QString const& grid, Radio::Frequency freq, QString const& mode, int snr);
    void addRemoteStation(QString call, QString grid,  QString freq, QString mode, int snr,QString);
    //
    // Flush any pending spots to PSK Reporter
    // 
    //void sendReport(bool last = false);
    //Q_SIGNAL void errorOccurred(QString const& reason);//hv stop
    Q_SIGNAL void ConectionInfo(QString);//hv add
    void set_server(QString);
    void set_server_port(int);
    void set_udp_tcp(bool);

private:
    class impl;
    pimpl<impl> m_;

};
#endif