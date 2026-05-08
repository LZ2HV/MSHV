#ifndef SETTINGS_MS_H
#define SETTINGS_MS_H


#include "../config.h"

#include <QDialog>
//#include <QTabWidget>
#include <QComboBox>
#include <QMessageBox>
#include <QLabel>
#include <QVBoxLayout>
//#include <QTimer>
#include <QGroupBox>
#include <QSpinBox>
#include <QRadioButton>


#if defined _WIN32_
//#include "../../../DirectX90c/include/dsound.h"
#include "../Hv_Lib_DirectX90c/dsound.h"
#endif
#if defined _LINUX_
#include <alsa/asoundlib.h>//hv for alsa scan for devices

//#include <pulse/error.h>
//#include <pulse/simple.h>
/*
#include <pulse/pulseaudio.h>
typedef struct pa_devicelist {
	uint8_t initialized;
	char name[512];
	uint32_t index;
	char description[256];
} pa_devicelist_t;
*/
#endif
//#include "../HvSlider_H/hvslider_h.h"
//#include "../HvAlsaMixer/HvSlider_V_Identif/hvslider_v_identif.h"
//#include <QtGui>

class SettingsMs : public QDialog
{
    Q_OBJECT
public:
    SettingsMs(QWidget * parent = 0);
    virtual ~SettingsMs();

    void SetMode(int);//tci
    QString default_in_dev()
    {
        return DevBoxIn->currentText();
    };
    QString default_out_dev()
    {
        return DevBoxOut->currentText();
    };
    QString default_out_buf()
    {
        return OutBuf->currentText();
    };
    //QString default_sample_rate()
    //{
        //return SampleRate->currentText();
    //};
    QString default_card_latency()
    {
        return CardLatency->currentText();
    };
    QString default_card_buffer_polls()
    {
        return CardBufferPolls->currentText();
    };
    QString default_scan_refresh()
    {
        return QString("%1").arg(SB_Refresh->value());
    };
    QString default_lm_refresh()
    {
        return QString("%1").arg(SB_Refresh_lm->value());
    };
    QString default_in_channel()
    {
        QString s;
        if (rb_left_ch->isChecked()) s = "0";
        else s = "1";
        return s;
    };
    QString default_bitpersample()
    {
        return cb_out_bitpersample->currentText()+"#"+cb_in_bitpersample->currentText();
    };
    void SetDevices_Drv(QString dev_alsa,QString card_latency,QString, 
                        QString card_buffer_polls,QString in_channel,QString scan_refresh,
                        QString lm_refresh,QString out_dev,QString out_buf);

signals:
    void InDevChanged(QString,int,int,int,int,int,int);//QString rate, 
    void OutDevChanged(QString dev,int,int buf);
    void EmitTciSelect(int);//tci

private slots:
    void InChannelChanget(bool);
    void InDeviceChanged(QString);    
    void OutDeviceChanged(QString);

private:
    //QTabWidget *TabOpt;
    QComboBox *DevBoxIn;
    QComboBox *DevBoxOut;
    QComboBox *OutBuf;
    //QComboBox *SampleRate;
    QComboBox *cb_out_bitpersample;
    QComboBox *cb_in_bitpersample;
    
    
///// pule dev //////    
/*
#if defined _LINUX_
    void pa_state_cb(pa_context *c, void *userdata);
	void pa_sinklist_cb(pa_context *c, const pa_sink_info *l, int eol, void *userdata); 
    void pa_sourcelist_cb(pa_context *c, const pa_source_info *l, int eol, void *userdata);
	int pa_get_devicelist(pa_devicelist_t *input, pa_devicelist_t *output);
#endif
*/
/////end  pule dev //////    

    void SearchSoundDev();
    //bool g_block_alsa;
    //void SendDevDrv();
    bool no_emit;

    QComboBox *CardLatency;
    QComboBox *CardBufferPolls;
    QSpinBox *SB_Refresh;
    QSpinBox *SB_Refresh_lm;

    QRadioButton *rb_left_ch;
    QRadioButton *rb_right_ch;
    int f_tci_restr;//tci
    void TciDevSelectAndRestr();//tci

};
#endif
