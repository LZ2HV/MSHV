#ifndef HVALSAMIXER_H
#define HVALSAMIXER_H


#include "../config.h"

#if defined _LINUX_

#include <QWidget>
#include <QHBoxLayout>
#include <QTimer>
#include <QLabel>
#include <alsa/asoundlib.h>
#include "../HvSlider_V_Identif/hvslider_v_identif.h"
#include "hvvtext.h"
#include "hvrbutton.h"
#include "hvcbox.h"

//#include <QtGui>

class HvAlsaMixer : public QWidget
{
    Q_OBJECT
public:
    HvAlsaMixer(QString dev_name, QWidget * parent = 0);
    virtual ~HvAlsaMixer();

public slots:
    int CloseHandle();

private slots:
    int writeVolumeToHW(int val, int identif);
    void handles_refresh();
    void setEnumIdHW(int devnum, int idx);

private:
    snd_mixer_t *handle_mixer;
    QString devName;

    QTimer *timer_refresh;
    struct pollfd  *m_fds;
    int	m_count;

    void AddWidgets(QString);
    QPixmap p0;
    QPixmap p1;
    QPixmap p2;
    QPixmap p3;
    QPixmap p4;
    QPixmap p5;
    QPixmap p6;
    QPixmap p7;
    QPixmap p8;
///////////////////////////////////////////////////////////
    typedef QList<snd_mixer_selem_id_t *>AlsaMixerSidList;
    AlsaMixerSidList mixer_sid_list;
    typedef QList<snd_mixer_elem_t *> AlsaMixerElemList;
    AlsaMixerElemList mixer_elem_list;

    typedef QHash<int,HvRbutton *>rbc_f_h;
    rbc_f_h rbtc;
    typedef QHash<int,HvRbutton *>rbp_f_h;
    rbp_f_h rbtp;
    typedef QHash<int,HvSlider_V_Identif *>sc_f_h;
    sc_f_h sldc;
    typedef QHash<int,HvSlider_V_Identif *>sp_f_h;
    sp_f_h sldp;
    typedef QHash<int,HvCBox *>cb_f_h;
    cb_f_h enum_cb;
///////////////////////////////////////////////////////////
    bool blosk_write;
    int OpenMixHandle(QString);

    bool m_isOpen;

    //int id2num(const QString& id);
    snd_mixer_elem_t* getMixerElem(int idx);
    void RefreshAllHW();
    int readVolumeFromHW(snd_mixer_elem_t *, int identif);
    int getMaxVol(snd_mixer_elem_t *, bool capt_or_vol);
    void addEnumerated(snd_mixer_elem_t *elem, QList<QString*>& enumList);
    unsigned int get_enumIdHW(snd_mixer_elem_t * elem);



};
#endif
#endif
