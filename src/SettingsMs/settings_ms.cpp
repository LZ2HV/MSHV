/* MSHV SettingsMs
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */

#include "settings_ms.h"

//#include <QtGui>

#if defined _WIN32_
QString ConvertToStr_t(LPCTSTR name)
{
    QString s;
    for (unsigned int i=0; i<wcslen(name); ++i) s.push_back(name[i]);
    //s.append(" Микрофон 128 ?????????"); //for test coding utf-8
    return s.toUtf8();
}
#define MAXCARDS 202
static QStringList lst_f_win;//LPCTSTR=lpszDesc LPCTSTR=lpszDrvName=*.dll LPVOID=pContext
BOOL CALLBACK DSEnumProc_x(LPGUID,LPCTSTR lpszDesc,LPCTSTR,LPVOID)
{
    if (lst_f_win.count() < MAXCARDS-3) //2.48 =0-199 in out cards
    {
        lst_f_win<<ConvertToStr_t(lpszDesc); //qDebug()<<ConvertToStr_t(lpszDesc)<<ConvertToStr_t(s);
    }
    return true;
}
#endif
 
SettingsMs::SettingsMs( QWidget * parent )
        : QDialog(parent)
{
    setWindowTitle(tr("Settings"));
    setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);// maha help butona

    f_tci_restr = false;

    QVBoxLayout *VL = new QVBoxLayout(this);
    VL->setContentsMargins(5, 5, 5, 5);
    VL->setSpacing(5);

    QGroupBox *GB_out = new QGroupBox(tr("Sound Output Settings")+":");
    //GB_out->setFixedWidth(364);
    QLabel *l_devo;
    l_devo = new QLabel();
    //l_devo->move(10, 10);
    //l_devo->setFixedSize(180, 15);
    /*
    #if defined _WIN32_
       	l_devo->setText(tr("Direct Sound Output Devices")+": ");
    #endif 
    #if defined _LINUX_
       	l_devo->setText(tr("Out Devices")+": ");
    #endif
    */
    l_devo->setText(tr("Output Devices")+": "); //2.65 not only Direct Sound
    DevBoxOut = new QComboBox();
    QVBoxLayout *VL_out = new QVBoxLayout();
    VL_out->setContentsMargins ( 5, 5, 5, 5);
    VL_out->setSpacing(5);
    VL_out->addWidget(l_devo);
    VL_out->addWidget(DevBoxOut);

    QVBoxLayout *VL_dsb = new QVBoxLayout();
    VL_dsb->setContentsMargins(1, 1, 1, 1);
    VL_dsb->setSpacing(1);
    QLabel *l_bout;
    l_bout = new QLabel();
    l_bout->setText(tr("Direct Sound Buffer")+": ");
    OutBuf = new QComboBox();
    //OutBuf->move(10,80);
    QStringList lst_outb;
    //lst_outb << "100" << "300" << "400" << "600" << "800" << "1000" << "1500"; to 1.41
    lst_outb << "400" << "600" << "800" << "1000" << "1500" << "2000" << "2500";// from 1.42
    OutBuf->addItems(lst_outb);
    OutBuf->setCurrentIndex(3);//default 1.42 from 1=600 to 3=1000
    VL_dsb->addWidget(l_bout);
    VL_dsb->addWidget(OutBuf);

    QVBoxLayout *VL_ro = new QVBoxLayout();
    VL_ro->setContentsMargins(1, 1, 1, 1);
    VL_ro->setSpacing(1);
    QLabel *l_BPSo;
    l_BPSo = new QLabel();
    //l_BPSo->setFixedSize(120, 15);
    l_BPSo->setText(tr("Bits Per Sample")+":");
    cb_out_bitpersample = new QComboBox();
    QStringList lst_bps;
    lst_bps << "16 bit" << "24 bit" << "32 bit";
    cb_out_bitpersample->addItems(lst_bps);
    cb_out_bitpersample->setCurrentIndex(0);
    //cb_in_bitpersample->setDisabled(true);
    VL_ro->addWidget(l_BPSo);
    VL_ro->addWidget(cb_out_bitpersample);

    QHBoxLayout *H_lout = new QHBoxLayout();
    H_lout->setContentsMargins(1,1,1,1);
    H_lout->setSpacing(1);
    H_lout->addLayout(VL_dsb);
    H_lout->addLayout(VL_ro);

    VL_out->addLayout(H_lout);
    //VL_out->addWidget(l_bout);
    //VL_out->addWidget(OutBuf);
    GB_out->setLayout(VL_out);
    VL->addWidget(GB_out);

    QGroupBox *GB_in = new QGroupBox(tr("Sound Input Settings")+":");
    QVBoxLayout *VL_in = new QVBoxLayout();
    VL_in->setContentsMargins(5, 5, 5, 5);
    VL_in->setSpacing(5);
    QLabel *l_alsadrv;
    l_alsadrv = new QLabel();
    //l_alsadrv->move(10, 120);
    //l_alsadrv->setFixedSize(185, 15);
    /*
    #if defined _WIN32_
       	l_alsadrv->setText(tr("Direct Sound Input Devices")+": ");
    #endif
    #if defined _LINUX_
       	l_alsadrv->setText(tr("Input Devices")+": ");
    #endif
    */
    l_alsadrv->setText(tr("Input Devices")+":"); //2.65 not only Direct Sound
    DevBoxIn = new QComboBox();
    VL_in->addWidget(l_alsadrv);
    VL_in->addWidget(DevBoxIn);

    /*QVBoxLayout *VL_r = new QVBoxLayout();
    VL_r->setContentsMargins(1, 1, 1, 1);
    VL_r->setSpacing(1);
    QLabel *l_SR;
    l_SR = new QLabel();
    l_SR->setFixedSize(120, 15);
    l_SR->setText("Sample Rate in Hz: ");*/
    /*SampleRate = new QComboBox();
    QStringList lst_SR;
    lst_SR << "11025" << "22050" << "44100" << "48000";
    SampleRate->addItems(lst_SR);
    SampleRate->setCurrentIndex(2);
    SampleRate->setDisabled(true);*/
    //VL_r->addWidget(l_SR);
    //VL_r->addWidget(SampleRate);

    QVBoxLayout *VL_ri = new QVBoxLayout();
    VL_ri->setContentsMargins(1, 1, 1, 1);
    VL_ri->setSpacing(1);
    QLabel *l_BPSi;
    l_BPSi = new QLabel();
    //l_BPSi->setFixedSize(120, 15);
    l_BPSi->setText(tr("Bits Per Sample")+":");
    cb_in_bitpersample = new QComboBox();
    cb_in_bitpersample->addItems(lst_bps);
    cb_in_bitpersample->setCurrentIndex(0);
    //cb_in_bitpersample->setDisabled(true);
    VL_ri->addWidget(l_BPSi);
    VL_ri->addWidget(cb_in_bitpersample);

    QVBoxLayout *VL_l = new QVBoxLayout();
    VL_l->setContentsMargins(1, 1, 1, 1);
    VL_l->setSpacing(1);
    QLabel *l_Latency;
    l_Latency = new QLabel();
    //l_Latency->move(140, 180);
    l_Latency->setFixedSize(110, 15);
    l_Latency->setText(tr("Latency in ms")+": ");
    CardLatency = new QComboBox();
    //CardLatency->move(140,210);
    QStringList lst_Latency;
    lst_Latency << "40" << "50" << "100" << "150" << "200" << "250" << "300";
    CardLatency->addItems(lst_Latency);
    CardLatency->setCurrentIndex(2);
    VL_l->addWidget(l_Latency);
    VL_l->addWidget(CardLatency);

    QVBoxLayout *VL_b = new QVBoxLayout();
    VL_b->setContentsMargins ( 1, 1, 1, 1);
    VL_b->setSpacing(1);
    QLabel *l_CardBufferPolls;
    l_CardBufferPolls = new QLabel();
    //l_CardBufferPolls->move(240, 180);
    l_CardBufferPolls->setFixedSize(130, 15);
    l_CardBufferPolls->setText(tr("Buffer Polls in ms")+": ");
    CardBufferPolls = new QComboBox();
    //CardBufferPolls->move(240,210);
    QStringList lst_CardBufferPolls;
    lst_CardBufferPolls << "4000" << "5000" << "6000" << "7000";
    CardBufferPolls->addItems(lst_CardBufferPolls);
    CardBufferPolls->setCurrentIndex(1);
    VL_b->addWidget(l_CardBufferPolls);
    VL_b->addWidget(CardBufferPolls);

    /*QHBoxLayout *H_lbr = new QHBoxLayout();
    H_lbr->setContentsMargins(1,1,1,1);
    H_lbr->setSpacing(1);
    H_lbr->addLayout(VL_l);
    H_lbr->addLayout(VL_b);
    H_lbr->addLayout(VL_ri);
    VL_in->addLayout(H_lbr);
    GB_in->setLayout(VL_in);
    VL->addWidget(GB_in);*/

    //QGroupBox *GB_Input_ch = new QGroupBox(tr("Input Channel Settings")+":");
    //GB_Input_ch->move(9, 235);
    //GB_Input_ch->setFixedWidth(364);
    rb_left_ch = new QRadioButton(tr("Left Channel"));
    rb_right_ch = new QRadioButton(tr("Right Channel"));
    QHBoxLayout *l_chanels = new QHBoxLayout();
    l_chanels->setContentsMargins(1,1,1,1);
    l_chanels->addWidget(rb_left_ch);
    l_chanels->addWidget(rb_right_ch);
    l_chanels->setAlignment(Qt::AlignHCenter);
    //GB_Input_ch->setLayout(l_chanels);
    rb_left_ch->setChecked(true);
    connect(rb_left_ch, SIGNAL(toggled(bool)), this, SLOT(InChannelChanget(bool)));
    connect(rb_right_ch, SIGNAL(toggled(bool)), this, SLOT(InChannelChanget(bool)));

    //VL->addWidget(GB_Input_ch);

    QHBoxLayout *H_lbr = new QHBoxLayout();
    H_lbr->setContentsMargins(1,1,1,1);
    H_lbr->setSpacing(1);
    H_lbr->addLayout(VL_l);
    H_lbr->addLayout(VL_b);
    H_lbr->addLayout(VL_ri);
    VL_in->addLayout(H_lbr);
    VL_in->addLayout(l_chanels);
    GB_in->setLayout(VL_in);
    VL->addWidget(GB_in);

    QGroupBox *GB_Display = new QGroupBox(tr("Settings")+":");
    //GB_Display->move(9, 295);
    //GB_Display->setFixedWidth(364);
    QLabel *l_refr = new QLabel();
    l_refr->setText(tr("Display Refresh Speed,")+" MSK JTMS FSK ISCAT JT6M : ");
    SB_Refresh = new QSpinBox();
    SB_Refresh->setRange(0,20);
    SB_Refresh->setValue(0);//5
    QHBoxLayout *l_ch = new QHBoxLayout();
    l_ch->setContentsMargins ( 1, 1, 1, 1);
    l_ch->addWidget(l_refr);
    l_ch->addWidget(SB_Refresh);
    l_ch->setSpacing(1);

    QLabel *l_refr_lm = new QLabel();
    l_refr_lm->setText(tr("Level Meter Refresh Speed (fastest=0 slowest=5)")+" : ");
    SB_Refresh_lm = new QSpinBox();
    SB_Refresh_lm->setRange(0,5);
    SB_Refresh_lm->setValue(2);//5
    QHBoxLayout *l_ch_lm = new QHBoxLayout();
    l_ch_lm->setContentsMargins ( 1, 1, 1, 1);
    l_ch_lm->addWidget(l_refr_lm);
    l_ch_lm->addWidget(SB_Refresh_lm);
    l_ch_lm->setSpacing(1);

    QVBoxLayout *Layout_VC = new QVBoxLayout();
    Layout_VC->setContentsMargins ( 10, 3, 10, 10);
    Layout_VC->addLayout(l_ch);
    Layout_VC->addLayout(l_ch_lm);
    //Layout_VC->addLayout(H_ttt);
    GB_Display->setLayout(Layout_VC);
    Layout_VC->setSpacing(5);

    VL->addWidget(GB_Display);
    this->setLayout(VL);

    //g_block_alsa = false;

    //DevBoxIn->setDisabled(true);
    //DevBoxOut->setDisabled(true);
    //connect(SampleRate, SIGNAL(currentIndexChanged(QString)), this, SLOT(DeviceChanged(QString)));
    connect(DevBoxIn, SIGNAL(currentIndexChanged(QString)), this, SLOT(InDeviceChanged(QString)));
    connect(CardLatency, SIGNAL(currentIndexChanged(QString)), this, SLOT(InDeviceChanged(QString)));
    connect(CardBufferPolls, SIGNAL(currentIndexChanged(QString)), this, SLOT(InDeviceChanged(QString)));
    connect(SB_Refresh, SIGNAL(valueChanged(QString)), this, SLOT(InDeviceChanged(QString)));
    connect(SB_Refresh_lm, SIGNAL(valueChanged(QString)), this, SLOT(InDeviceChanged(QString)));
    connect(cb_in_bitpersample, SIGNAL(currentIndexChanged(QString)), this, SLOT(InDeviceChanged(QString)));

    connect(DevBoxOut, SIGNAL(currentIndexChanged(QString)), this, SLOT(OutDeviceChanged(QString)));
    connect(OutBuf, SIGNAL(currentIndexChanged(QString)), this, SLOT(OutDeviceChanged(QString)));
    connect(cb_out_bitpersample, SIGNAL(currentIndexChanged(QString)), this, SLOT(OutDeviceChanged(QString)));

    no_emit = true;
    SearchSoundDev();
}
SettingsMs::~SettingsMs()
{}
/////////////////// pulse dev //////////////////////////////////////////////
#if defined _LINUX_
#include <pulse/pulseaudio.h>
typedef struct pa_devicelist
{
    uint8_t initialized;
    char name[512];
    uint32_t index;
    char description[256];
}
pa_devicelist_t;
// This callback gets called when our context changes state.  We really only
// care about when it's ready or if it has failed
void pa_state_cb(pa_context *c, void *userdata)
{
    pa_context_state_t state;
    int *pa_ready = (int*)userdata;

    state = pa_context_get_state(c);
    switch  (state)
    {
        // There are just here for reference
    case PA_CONTEXT_UNCONNECTED:
    case PA_CONTEXT_CONNECTING:
    case PA_CONTEXT_AUTHORIZING:
    case PA_CONTEXT_SETTING_NAME:
    default:
        break;
    case PA_CONTEXT_FAILED:
    case PA_CONTEXT_TERMINATED:
        *pa_ready = 2;
        break;
    case PA_CONTEXT_READY:
        *pa_ready = 1;
        break;
    }
}
// pa_mainloop will call this function when it's ready to tell us about a sink.
// Since we're not threading, there's no need for mutexes on the devicelist
// structure
void pa_sinklist_cb(pa_context *, const pa_sink_info *l, int eol, void *userdata)
{
    pa_devicelist_t *pa_devicelist = (pa_devicelist_t *)userdata;
    int ctr = 0;

    // If eol is set to a positive number, you're at the end of the list
    if (eol > 0)
    {
        return;
    }

    // We know we've allocated 16 slots to hold devices.  Loop through our
    // structure and find the first one that's "uninitialized."  Copy the
    // contents into it and we're done.  If we receive more than 16 devices,
    // they're going to get dropped.  You could make this dynamically allocate
    // space for the device list, but this is a simple example.
    for (ctr = 0; ctr < 16; ctr++)
    {
        if (! pa_devicelist[ctr].initialized)
        {
            strncpy(pa_devicelist[ctr].name, l->name, 511);
            strncpy(pa_devicelist[ctr].description, l->description, 255);
            pa_devicelist[ctr].index = l->index;
            pa_devicelist[ctr].initialized = 1;
            break;
        }
    }
}
// See above.  This callback is pretty much identical to the previous
void pa_sourcelist_cb(pa_context *, const pa_source_info *l, int eol, void *userdata)
{
    pa_devicelist_t *pa_devicelist = (pa_devicelist_t *)userdata;
    int ctr = 0;

    if (eol > 0)
    {
        return;
    }

    for (ctr = 0; ctr < 16; ctr++)
    {
        if (! pa_devicelist[ctr].initialized)
        {
            strncpy(pa_devicelist[ctr].name, l->name, 511);
            strncpy(pa_devicelist[ctr].description, l->description, 255);
            pa_devicelist[ctr].index = l->index;
            pa_devicelist[ctr].initialized = 1;
            break;
        }
    }
}
int _pa_get_devicelist_(pa_devicelist_t *input, pa_devicelist_t *output)
{
    // Define our pulse audio loop and connection variables
    pa_mainloop *pa_ml;
    pa_mainloop_api *pa_mlapi;
    pa_operation *pa_op;
    pa_context *pa_ctx;
    //pa_context_flags_t *pa_ctx;

    // We'll need these state variables to keep track of our requests
    int state = 0;
    int pa_ready = 0; //2.65 test

    // Initialize our device lists
    memset(input, 0, sizeof(pa_devicelist_t) * 16);
    memset(output, 0, sizeof(pa_devicelist_t) * 16);

    // Create a mainloop API and connection to the default server
    pa_op = NULL;//2.60
    pa_ml = pa_mainloop_new();
    pa_mlapi = pa_mainloop_get_api(pa_ml);
    pa_ctx = pa_context_new(pa_mlapi, "test");

    // This function connects to the pulse server
    pa_context_connect(pa_ctx, NULL, (pa_context_flags_t)0, NULL);// 		pa_context_flags_t  	flags,

    // This function defines a callback so the server will tell us it's state.
    // Our callback will wait for the state to be ready.  The callback will
    // modify the variable to 1 so we know when we have a connection and it's
    // ready.
    // If there's an error, the callback will set pa_ready to 2
    pa_context_set_state_callback(pa_ctx, pa_state_cb, &pa_ready);

    // Now we'll enter into an infinite loop until we get the data we receive
    // or if there's an error

    for (;;)
    {
        // We can't do anything until PA is ready, so just iterate the mainloop
        // and continue
        //fprintf(stderr, "in state %d\n", pa_ready);
        if (pa_ready == 0)
        {
            pa_mainloop_iterate(pa_ml, 1, NULL);
            continue;
        }
        // We couldn't get a connection to the server, so exit out
        if (pa_ready == 2)
        {
            pa_context_disconnect(pa_ctx);
            pa_context_unref(pa_ctx);
            pa_mainloop_free(pa_ml);
            return -1;
        }
        // At this point, we're connected to the server and ready to make
        // requests


        switch (state)
        {
            // State 0: we haven't done anything yet
        case 0:
            // This sends an operation to the server.  pa_sinklist_info is
            // our callback function and a pointer to our devicelist will
            // be passed to the callback The operation ID is stored in the
            // pa_op variable
            pa_op = pa_context_get_sink_info_list(pa_ctx,
                                                  pa_sinklist_cb,
                                                  output
                                                 );

            // Update state for next iteration through the loop
            state++;
            break;
        case 1:
            // Now we wait for our operation to complete.  When it's
            // complete our pa_output_devicelist is filled out, and we move
            // along to the next state
            if (pa_operation_get_state(pa_op) == PA_OPERATION_DONE)
            {
                pa_operation_unref(pa_op);

                // Now we perform another operation to get the source
                // (input device) list just like before.  This time we pass
                // a pointer to our input structure
                pa_op = pa_context_get_source_info_list(pa_ctx,
                                                        pa_sourcelist_cb,
                                                        input
                                                       );
                // Update the state so we know what to do next
                state++;
            }
            break;
        case 2:
            if (pa_operation_get_state(pa_op) == PA_OPERATION_DONE)
            {
                // Now we're done, clean up and disconnect and return
                pa_operation_unref(pa_op);
                pa_context_disconnect(pa_ctx);
                pa_context_unref(pa_ctx);
                pa_mainloop_free(pa_ml);
                return 0;
            }
            break;
        default:
            // We should never see this state
            fprintf(stderr, "in state %d\n", state);
            return -1;
        }
        // Iterate the main loop and go again.  The second argument is whether
        // or not the iteration should block until something is ready to be
        // done.  Set it to zero for non-blocking.
        pa_mainloop_iterate(pa_ml, 1, NULL);
    }
}
#endif
///////////////////end pulse dev //////////////////////////////////////////////
void SettingsMs::SearchSoundDev()
{
#if defined _LINUX_
    ///////////////////pulse dev //////////////////////////////////////////////
    int ctr; //2.63 first is pulse, for PI4 devices is important
    QStringList lst_f_out_pa,lst_f_in_pa;
    pa_devicelist_t pa_input_devicelist[16];
    pa_devicelist_t pa_output_devicelist[16];
    if (_pa_get_devicelist_(pa_input_devicelist, pa_output_devicelist) < 0)
    {
        fprintf(stderr, "PulseAudio failed to get device list\n");
        //return; 2.65 test
    }
    for (ctr = 0; ctr < 16; ctr++)
    {
        if (! pa_output_devicelist[ctr].initialized) break;
        if (ctr == 0) lst_f_out_pa << "pulse: default";
        lst_f_out_pa << "pulse: " + (QString)pa_output_devicelist[ctr].name;
    }
    for (ctr = 0; ctr < 16; ctr++)
    {
        if (! pa_input_devicelist[ctr].initialized) break;
        if (ctr == 0) lst_f_in_pa << "pulse: default";
        lst_f_in_pa << "pulse: " + (QString)pa_input_devicelist[ctr].name;
    }
    if (lst_f_in_pa.count()>0) DevBoxIn->addItems(lst_f_in_pa);
    if (lst_f_out_pa.count()>0) DevBoxOut->addItems(lst_f_out_pa);
    ///////////////////end pulse dev //////////////////////////////////////////////

    QStringList lst_f;
    snd_ctl_t *handle;
    int card, err, dev;// idx;
    snd_ctl_card_info_t *info;
    snd_pcm_info_t *pcminfo;
    snd_ctl_card_info_alloca(&info);
    snd_pcm_info_alloca(&pcminfo);
    /*static snd_pcm_t *hCapture;
    snd_pcm_hw_params_t *hware; */
    card = -1;
    if (snd_card_next(&card) < 0 || card < 0)
    {
        if (DevBoxIn->count()<1 || DevBoxOut->count()<1)
            fprintf(stderr, "No soundcards found...\n");
    }
    // fprintf(stderr, "**** List of %s Hardware Devices ****\n", snd_pcm_stream_name(stream));
    while (card >= 0)
    {
        char name[32];
        sprintf(name, "hw:%d", card);
        if ((err = snd_ctl_open(&handle, name, 0)) < 0)
        {
            fprintf(stderr, "control open (%i): %s", card, snd_strerror(err));
            goto next_card;
        }
        if ((err = snd_ctl_card_info(handle, info)) < 0)
        {
            fprintf(stderr, "control hardware info (%i): %s", card, snd_strerror(err));
            snd_ctl_close(handle);
            goto next_card;
        }
        dev = -1;
        while (1)
        {
            //unsigned int count;
            if (snd_ctl_pcm_next_device(handle, &dev)<0)
                fprintf(stderr, "snd_ctl_pcm_next_device\n");
            if (dev < 0)
                break;
            snd_pcm_info_set_device(pcminfo, dev);
            snd_pcm_info_set_subdevice(pcminfo, 0);
            //snd_pcm_info_set_stream(pcminfo, stream);
            if ((err = snd_ctl_pcm_info(handle, pcminfo)) < 0)
            {
                if (err != -ENOENT)
                    fprintf(stderr, "control digital audio info (%i): %s", card, snd_strerror(err));
                continue;
            }
            //fprintf(stderr, "card %i: %s [%s], device %i: %s [%s]\n",card, snd_ctl_card_info_get_id(info),
            //snd_ctl_card_info_get_name(info),dev,
            //                    snd_pcm_info_get_id(pcminfo),
            //                    snd_pcm_info_get_name(pcminfo));
            lst_f << QString("card %1: %3, device %2: %4").arg(card).arg(dev).arg(snd_ctl_card_info_get_name(info)).arg(snd_pcm_info_get_name(pcminfo));
            //count = snd_pcm_info_get_subdevices_count(pcminfo);
        }
        snd_ctl_close(handle);
        /*
        	if ((err = snd_pcm_open (&hCapture, name, SND_PCM_STREAM_CAPTURE, SND_PCM_NONBLOCK)) < 0) {
        		fprintf(stderr, "Card snd_pcm_open problem\n");
        	}
        	if ((err = snd_pcm_hw_params_malloc (&hware)) < 0) {
        		fprintf(stderr, "Card snd_pcm_hw_params_malloc problem\n");
        	}
        		if ((err = snd_pcm_hw_params_any (hCapture, hware)) < 0) {
        		fprintf(stderr, "Card snd_pcm_hw_params_any problem\n");
        	}
            unsigned int min, max;
        	snd_pcm_hw_params_get_rate_min(hware, &min, 0);
        	qDebug()<<min;
            snd_pcm_hw_params_get_rate_max(hware, &max, 0);
            qDebug()<<max;
            snd_pcm_close(hCapture);
        */
next_card:
        if (snd_card_next(&card) < 0)
        {
            fprintf(stderr, "snd_card_next\n");
            break;
        }
    }
    if (!lst_f.isEmpty())
    {
        DevBoxIn->addItems(lst_f);
        DevBoxOut->addItems(lst_f);
    }

    //if (DevBoxIn->count()<1 || DevBoxOut->count()<1) g_block_alsa = true; //no any devices
    if (DevBoxIn->count()<1 || DevBoxOut->count()<1)
    {
        //DevBoxIn->setDisabled(true); 2.65 test
        //DevBoxOut->setDisabled(true); 2.65 test
        QMessageBox::critical(this, "MSHV",
                              "No sound device detected\n"
                              "Close application and install\nsoundcard driver please.",
                              QMessageBox::Close);
    }
    /*else 2.65 test
    {
        DevBoxIn->setDisabled(false);
        DevBoxOut->setDisabled(false);
    }*/
    //DevBoxIn->setDisabled(false);//2.65 test
    //DevBoxOut->setDisabled(false);//2.65 test
    //g_block_alsa = false;//2.65 test

    //no_emit = false;

    /*  if (g_block_alsa)
          QMessageBox::critical(this, tr("Player LZ2HV"),
                                tr("No Alsa or OSS sound device detected\n"
                                   "Close application and install\nsoundcard driver please."),
                                QMessageBox::Close);*/
/////////////////alsa/////////////////////////////////////////////////////////////////////////////////////
    //qDebug()<<"alsa_block: "<<g_block_alsa;
    //qDebug()<<"oss_block: "<<g_block_oss;
#endif
///////////////////////alsa/////////////////////////////////////////////
#if defined _WIN32_
    DWORD pv;  // Can be any 32-bit type.

    if ( DS_OK != DirectSoundCaptureEnumerate((LPDSENUMCALLBACK)DSEnumProc_x, (VOID*)&pv) )
    {
        QMessageBox::critical(this, "MSHV",
                              "No DirectSoundCaptureEnumerate device detected\n"
                              "Close application and install\nsoundcard driver please.",
                              QMessageBox::Close);
    }

    if (!lst_f_win.isEmpty())
    {
        //q_Sort(lst_f_win.begin()+2, lst_f_win.end()); // no problem tested
        DevBoxIn->addItems(lst_f_win); //q_Sort(lst_f_win);
        //DevBoxIn->setDisabled(false);
    }

    lst_f_win.clear();

    if ( DS_OK != DirectSoundEnumerate((LPDSENUMCALLBACK)DSEnumProc_x, (VOID*)&pv) )
    {
        QMessageBox::critical(this, "MSHV",
                              "No DirectSoundEnumerate device detected\n"
                              "Close application and install\nsoundcard driver please.",
                              QMessageBox::Close);
    }

    if (!lst_f_win.isEmpty())
    {
        //q_Sort(lst_f_win.begin()+2, lst_f_win.end()); // no problem tested
        DevBoxOut->addItems(lst_f_win); //qDebug()<<"out_dev"<<lst_f_win;
        //DevBoxOut->setDisabled(false);
    }
    //else
    //g_block_alsa = true;

    lst_f_win.clear();

    //no_emit = false;
#endif
    //tci
    DevBoxIn->addItem("TCI Client Input");
    DevBoxOut->addItem("TCI Client Output");
    //end tci
    no_emit = false;
}
void SettingsMs::SetDevices_Drv(QString dev_in,QString bpsampl,QString card_latency,//sample_rate not used
                                QString card_buffer_polls,QString in_channel,QString scan_refresh,
                                QString lm_refresh,QString out_dev,QString out_buf)
{
    no_emit = true;
    //qDebug()<<"Readd="<<dev_alsa<<out_dev;
    //dev_alsa = "Usb Audio CODEC";
    int index = DevBoxIn->findText(dev_in, Qt::MatchCaseSensitive);
    if (index >= 0) DevBoxIn->setCurrentIndex(index);

    /*index = SampleRate->findText(sample_rate, Qt::MatchCaseSensitive);
    if (index >= 0) SampleRate->setCurrentIndex(index);
    SampleRate->setCurrentIndex(0);*/
    QStringList lbps = bpsampl.split("#"); //qDebug()<<bpsampl;//<<lbps.count()<<lbps.at(0)<<lbps.at(1);
    if (lbps.count()>1)
    {
        index = cb_out_bitpersample->findText(lbps.at(0), Qt::MatchCaseSensitive);
        if (index >= 0) cb_out_bitpersample->setCurrentIndex(index);
        index = cb_in_bitpersample->findText(lbps.at(1), Qt::MatchCaseSensitive);
        if (index >= 0) cb_in_bitpersample->setCurrentIndex(index);
    }

    index = CardLatency->findText(card_latency, Qt::MatchCaseSensitive);
    if (index >= 0) CardLatency->setCurrentIndex(index);

    index = CardBufferPolls->findText(card_buffer_polls , Qt::MatchCaseSensitive);
    if (index >= 0) CardBufferPolls->setCurrentIndex(index);

    index = DevBoxOut->findText(out_dev , Qt::MatchCaseSensitive);
    if (index >= 0) DevBoxOut->setCurrentIndex(index);

    index = OutBuf->findText(out_buf , Qt::MatchCaseSensitive);
    if (index >= 0) OutBuf->setCurrentIndex(index);

    if (in_channel.toInt()==0) rb_left_ch->setChecked(true);
    else rb_right_ch->setChecked(true);

    SB_Refresh->setValue(scan_refresh.toInt());
    SB_Refresh_lm->setValue(lm_refresh.toInt());
    //Slider_Input_Level->SetValue(in_level_cor.toInt());

    no_emit = false;
    //SendDevDrv();
    InDeviceChanged("a");
    OutDeviceChanged("a");
}
/*void SettingsMs::InDeviceChanged(QString)
{
    SendDevDrv();
}*/
void SettingsMs::TciDevSelectAndRestr()//tci
{
    int out = 0;
    QString dev = "";
    if (DevBoxOut->currentText()=="TCI Client Output" && DevBoxIn->currentText()=="TCI Client Input")
    {
        dev=tr("Output and Input Devices");
        out = 3;
    }
    else if (DevBoxOut->currentText()=="TCI Client Output")
    {
        dev=tr("Output Device");
        out = 2;
    }
    else if (DevBoxIn->currentText()=="TCI Client Input")
    {
        dev=tr("Input Device");
        out = 1;
    }
    emit EmitTciSelect(out);//0=non 1=rx 2=tx 3=rx,tx

    /*if (DevBoxOut->currentText()=="TCI Client Output")
    {
    	OutBuf->setEnabled(false);
    	cb_out_bitpersample->setEnabled(false);
    }
    else
    {
    	OutBuf->setEnabled(true);
    	cb_out_bitpersample->setEnabled(true);  
    }
    if (DevBoxIn->currentText()=="TCI Client Input")
    {
    	cb_in_bitpersample->setEnabled(false);
    	CardLatency->setEnabled(false);
    	CardBufferPolls->setEnabled(false);
    }
    else
    {
    	cb_in_bitpersample->setEnabled(true);
    	CardLatency->setEnabled(true);
    	CardBufferPolls->setEnabled(true);
    }*/

    if (!f_tci_restr) return;
    if (!dev.isEmpty())
    {
        QMessageBox::critical(this, "MSHV",
                              tr("The TCI Client does not support 44100 Hz Sample Rate\n"
                                 "Not possible to use modes JTMS, FSK, ISCAT and JT6M\n"
                                 "Please in Sound Settings choose other")+"\n"+dev,
                              QMessageBox::Close);
    }


}
void SettingsMs::SetMode(int mode)//tci
{
    //if (mode==0 || mode==7 || mode==8 || mode==9 || mode==10 || mode==11 || mode==12 || mode==13 ||
    //mode==14 || mode==15 || mode==16 || mode==17)
    if (mode==1 || mode==2 || mode==3 || mode==4 || mode==5 || mode==6) f_tci_restr = true;
    else f_tci_restr = false;
    TciDevSelectAndRestr();
}
void SettingsMs::OutDeviceChanged(QString)
{
    if (!no_emit)
    {
        if (DevBoxOut->currentText()=="TCI Client Output")//2.70
        {
            OutBuf->setEnabled(false);
            cb_out_bitpersample->setEnabled(false);
        }
        else
        {
            OutBuf->setEnabled(true);
            cb_out_bitpersample->setEnabled(true);
        }
        TciDevSelectAndRestr();
        QString b0 = cb_out_bitpersample->currentText();
        b0.remove(" bit"); //qDebug()<<b0<<b0.toInt();
#if defined _WIN32_
        emit OutDevChanged(DevBoxOut->currentText(),b0.toInt(),OutBuf->currentText().toInt());//2.48
        //dev_out mast bi in numbers
#endif
#if defined _LINUX_
        QString dev_name = DevBoxOut->currentText();

        //if (DevBoxOut->currentText()!="pulse")// "pulse: "
        QString dn0 = dev_name+"xxxxxxx";//2.70
        if (dn0.mid(0,7)!="pulse: " && dev_name!="TCI Client Output")
        {
            QRegExp rx;
            QString card_out, device_out;
            rx.setPattern("card (\\d*)");
            if (rx.indexIn(DevBoxOut->currentText()) != -1)
                card_out = rx.cap(1);

            rx.setPattern("device (\\d*)");
            if (rx.indexIn(DevBoxOut->currentText()) != -1)
                device_out = rx.cap(1);

            dev_name = "hw:"+card_out+","+device_out;
        }

        emit OutDevChanged(dev_name,b0.toInt(),OutBuf->currentText().toInt());
        //qDebug()<<"OutDeviceChanged="<<card_out;
        //dev_out mast bi in numbers
#endif
    }
}
void SettingsMs::InChannelChanget(bool)
{
    InDeviceChanged("a"); //SendDevDrv();
}
void SettingsMs::InDeviceChanged(QString)
{
    if (!no_emit)
    {
        //if (!g_block_alsa)//
        //{
        if (DevBoxIn->currentText()=="TCI Client Input")//2.70
        {
            cb_in_bitpersample->setEnabled(false);
            CardLatency->setEnabled(false);
            CardBufferPolls->setEnabled(false);
        }
        else
        {
            cb_in_bitpersample->setEnabled(true);
            CardLatency->setEnabled(true);
            CardBufferPolls->setEnabled(true);
        }
        TciDevSelectAndRestr();
        QString b0 = cb_in_bitpersample->currentText();
        b0.remove(" bit");
#if defined _LINUX_
        QString dev_name = DevBoxIn->currentText();

        //if (DevBoxIn->currentText()!="pulse")
        QString dn0 = dev_name+"xxxxxxx";//2.70
        if (dn0.mid(0,7)!="pulse: " && dev_name!="TCI Client Input")
        {
            QRegExp rx;
            QString card, device;
            rx.setPattern("card (\\d*)");
            if (rx.indexIn(DevBoxIn->currentText()) != -1)
                card = rx.cap(1);

            rx.setPattern("device (\\d*)");
            if (rx.indexIn(DevBoxIn->currentText()) != -1)
                device = rx.cap(1);

            dev_name = "hw:"+card+","+device;
        }

        emit InDevChanged(dev_name,b0.toInt(),/*SampleRate->currentText(),*/
                          CardLatency->currentText().toInt(),
                          CardBufferPolls->currentText().toInt(),(int)rb_right_ch->isChecked(),SB_Refresh->value(),
                          SB_Refresh_lm->value());
        //qDebug()<<"SendDevDrv="<<card;
        // qDebug()<<CardBufferPolls->currentText().toInt();
#endif
#if defined _WIN32_
        //qDebug()<<"Send SettingsMs="<<DevBoxIn->currentText();
        emit InDevChanged(DevBoxIn->currentText(),b0.toInt(),/*SampleRate->currentText(),*/
                          CardLatency->currentText().toInt(),
                          CardBufferPolls->currentText().toInt(),(int)rb_right_ch->isChecked(),SB_Refresh->value(),
                          SB_Refresh_lm->value());
        // qDebug()<<DevBoxIn->currentIndex()<<"OOO"<<DevHandles[DevBoxIn->currentIndex()];
#endif
        //}
    }
}


