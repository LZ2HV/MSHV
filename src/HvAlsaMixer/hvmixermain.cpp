#include "hvmixermain.h"
#if defined _LINUX_
HvMixerMain::HvMixerMain( QWidget * parent )
        : QWidget(parent)
{
    size_w = 50;
    size_h = 50;
    //setWindowFlags(Qt::FramelessWindowHint | Qt::MSWindowsFixedSizeDialogHint);
    setWindowIcon(QPixmap(":pic/ms_ico.png"));
    setWindowTitle("MSHV Mixer");
    //setWindowIcon(QPixmap(":pic/mix_icon.png"));
    //setWindowTitle("GeoRadar HV Mixer");
    TabCards = new QTabWidget(this);
    g_block_alsa = false;
    ScanForAlsaDev();
}
HvMixerMain::~HvMixerMain()
{}
void HvMixerMain::Start()
{
    if (!isVisible())
    {
        AddTabs();
        QDesktopWidget *desktop = QApplication::desktop();
        int screenWidth, width;
        int screenHeight, height;
        int x, y;
        QSize windowSize;
        screenWidth = desktop->screenGeometry(this).width();//2.63 correction for two monitors
    	screenHeight = desktop->screenGeometry(this).height();//2.63 correction for two monitors
        windowSize = size();
        width = windowSize.width();
        height = windowSize.height();
        x = (screenWidth - width) / 2;
        y = (screenHeight - height) / 2;
        y -= 50;
        move ( x, y );
        this->show();
    }
}
void HvMixerMain::ScanForAlsaDev()
{
///////////////////////alsa/////////////////////////////////////////////
    snd_ctl_t *handle;
    int card, err;// idx;
    snd_ctl_card_info_t *info;
    snd_pcm_info_t *pcminfo;
    snd_ctl_card_info_alloca(&info);
    snd_pcm_info_alloca(&pcminfo);

    /*    static snd_pcm_t *hCapture;
    	snd_pcm_hw_params_t *hware; */

    card = -1;
    if (snd_card_next(&card) < 0 || card < 0)
    {
    	card = -1;
        //2.65 stop fprintf(stderr, "No soundcards found...\n");
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
        //dev_names << QString("hw:80");
        //card_names << QString("888rtyrt");
        dev_names << QString("hw:%1").arg(card);
        card_names << QString("%1").arg(snd_ctl_card_info_get_name(info));
        //lst_f << QString("hwhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh:%1|#|%2").arg(100).arg(5005);
        //  lst_f << QString("card %1: %3, device %2: %4").arg(card).arg(dev).arg(snd_ctl_card_info_get_name(info)).arg(snd_pcm_info_get_name(pcminfo));
        snd_ctl_close(handle);

next_card:
        if (snd_card_next(&card) < 0)
        {
            fprintf(stderr, "snd_card_next\n");
            break;
        }
    }
    if (dev_names.isEmpty())
        g_block_alsa = true;
}

void HvMixerMain::AddTabs()
{
    if (!g_block_alsa)
    {
        int i = 0;
        for (QStringList::iterator it =  dev_names.begin(); it != dev_names.end(); it++)
        {
            HvAlsaMixer *TempW = new HvAlsaMixer(*it);
            TabCards->addTab(TempW, card_names.at(i));

            if (size_w < TempW->sizeHint().width())
                size_w = TempW->sizeHint().width();
            if (size_h < TempW->sizeHint().height())
                size_h = TempW->sizeHint().height();

            setFixedSize(QSize(size_w,size_h));
            connect(this, SIGNAL(CloseHandles()), TempW, SLOT(CloseHandle()));
            i++;
            //it++;
        }
    }
    else
    {
        this->setDisabled(true);
        QMessageBox::critical(this, "MSHV",
                              "No Alsa sound device detected\n"
                              "Close application and install\nsoundcard driver please.",
                              QMessageBox::Close);
    }
}
#endif




