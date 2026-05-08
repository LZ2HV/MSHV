#include "hvalsamixer.h"
#if defined _LINUX_
HvAlsaMixer::HvAlsaMixer(QString dev_name, QWidget * parent )
        : QWidget(parent)
{
    blosk_write = true;
    setFixedSize(50, 50);

    p0 = QPixmap(":pic/big/sld_track_v_si.png");
    p1 = QPixmap(":pic/big/sld_up_v_si.png");
    p2 = QPixmap(":pic/big/sld_down_v_si.png");
    p3 = QPixmap(":pic/big/tumb_v_si_p.png");
    p4 = QPixmap(":pic/big/tumb_over_v_si_p.png");
    p5 = QPixmap(":pic/big/tumb_v_si_c.png");
    p6 = QPixmap(":pic/big/tumb_over_v_si_c.png");
    p7 = QPixmap(":pic/big/tumb_v_si_pc.png");
    p8 = QPixmap(":pic/big/tumb_over_v_si_pc.png");

    QFont font_tfi = font();
    font_tfi.setPointSize(9);
    setFont(font_tfi);

    m_isOpen = false;
    devName = dev_name;
    AddWidgets(dev_name);

/////////////////////// setup na refresha ako niakoi drug butne mixera //////////////////////
    if ((m_count = snd_mixer_poll_descriptors_count(handle_mixer)) < 0)
    {
        fprintf(stderr, "Error snd_mixer_poll_descriptors_count()...\n");
    }
    m_fds = (struct pollfd*)calloc(m_count, sizeof(struct pollfd));
    if (m_fds == NULL)
    {
        fprintf(stderr, "Error m_fds == NULL...\n");
    }
    m_fds->events = POLLIN;
    int err;
    if ((err = snd_mixer_poll_descriptors(handle_mixer, m_fds, m_count)) < 0)
    {
        fprintf(stderr, "Error snd_mixer_poll_descriptors()...\n");
    }
    if (err != m_count)
    {
        fprintf(stderr, "Error err != m_count...\n");
    }
///////////////////////////////////////////////////////////////////////////////////////////////////////

    timer_refresh = new QTimer();
    connect(timer_refresh, SIGNAL(timeout()), this, SLOT(handles_refresh()));
    timer_refresh->start(200);
}

HvAlsaMixer::~HvAlsaMixer()
{}

void HvAlsaMixer::handles_refresh()
{
    if (!isVisible() || !m_isOpen)
        return;
    blosk_write = true;

    // Poll on fds with 10ms timeout
    // Hint: alsamixer has an infinite timeout, but we cannot do this because we would block
    // the X11 event handling (Qt event loop) with this.
    int finished = poll(m_fds, m_count, 10);
    if (finished > 0)
    {
        unsigned short revents;
        if (snd_mixer_poll_descriptors_revents(handle_mixer, m_fds, m_count, &revents) >= 0)
        {
            if (revents & POLLNVAL)
            {
                fprintf(stderr, "Error handles_refresh() -> POLLNVAL\n");
            }
            if (revents & POLLERR)
            {
                fprintf(stderr, "Error handles_refresh() -> POLLERR\n");
            }
            if (revents & POLLIN )
            {
                snd_mixer_handle_events(handle_mixer);
                RefreshAllHW();
                //qDebug()<<"REFRESH"<<devName;
            }
        }
    }
    blosk_write = false;
}

void HvAlsaMixer::RefreshAllHW()
{	//qDebug()<<"gggg";
    for (int i = 0; i < mixer_elem_list.count(); i++)
    {
        snd_mixer_elem_t *elem = mixer_elem_list[i];
        if ( snd_mixer_selem_is_enumerated(elem))
            enum_cb[i]->SetValue(get_enumIdHW(elem));
        else
            readVolumeFromHW(elem, i);
    }
}

void HvAlsaMixer::AddWidgets(QString dev_name)
{
    blosk_write = true;

    if (  OpenMixHandle(dev_name) != 0 )
    {
        fprintf(stderr, "Error open hamdle...\n");
        return;
    }

    QHBoxLayout *HLayout = new QHBoxLayout();
    HLayout->setContentsMargins ( 0, 5, 10, 25);//za opraviane
    HLayout->setSpacing(1);

    snd_mixer_elem_t *elem;
    unsigned int idx = 0;
    snd_mixer_selem_id_t *sid;
    snd_mixer_selem_id_alloca( &sid );

    QVBoxLayout *V_CB = new QVBoxLayout();
    V_CB->setContentsMargins ( 1, 0, 0, 3);
    V_CB->setSpacing(1);

    for ( elem = snd_mixer_first_elem( handle_mixer ); elem; elem = snd_mixer_elem_next( elem ) )
    {
        // If element is not active, just skip
        if ( ! snd_mixer_selem_is_active ( elem ) )
        {
            continue;
        }
        /* --- Create basic control structures: snd_mixer_selem_id_t*, ID, ... --------- */
        // snd_mixer_selem_id_t*
        // I believe we must malloc it ourself (just guessing due to missing ALSA documentation)
        snd_mixer_selem_id_malloc ( &sid ); // !! Return code should be checked. Ressoure must be freed when unplugging card
        snd_mixer_selem_get_id( elem, sid );

        mixer_sid_list.append(sid);
        mixer_elem_list.append(elem);

        QVBoxLayout *V_l = new QVBoxLayout();
        V_l->setContentsMargins ( 0, 0, 2, 0);
        V_l->setSpacing(2);
        V_l->setAlignment(Qt::AlignTop);

        if ( snd_mixer_selem_is_enumerated(elem) )
        {
            QList<QString*>enumList;
            addEnumerated(elem, enumList);
            QList<QString>_enumValues;
            for (int i = 0; i < enumList.count(); i++)
            {
                _enumValues.append( *(enumList.at(i)) );
            }
            HvCBox *temp_c = new HvCBox(idx);
            temp_c->addItems(_enumValues);
            qDeleteAll(enumList);
            _enumValues.clear();
            temp_c->SetValue(get_enumIdHW(elem));
            QString str =  QString("%1").arg(snd_mixer_selem_id_get_name(sid));
            QLabel *CB_l = new QLabel(str);
            V_CB->addWidget(CB_l);
            V_CB->setAlignment(CB_l,Qt::AlignCenter);
            V_CB->addWidget(temp_c);
            V_CB->setAlignment(temp_c,Qt::AlignCenter);
            enum_cb.insert(idx, temp_c);
            connect(temp_c, SIGNAL(SendVals(int,int)), this, SLOT(setEnumIdHW(int,int)));
        }
        else
        {
            if (snd_mixer_selem_has_playback_switch(elem))
            {
                HvRbutton *Rb_t = new HvRbutton(idx);
                //Rb_t->SetRbCaptureColor(false);
                V_l->addWidget(Rb_t);
                V_l->setAlignment(Rb_t, Qt::AlignHCenter);

                rbtp.insert(idx, Rb_t);

                connect(Rb_t, SIGNAL(SendVals(int,int)), this, SLOT(writeVolumeToHW(int,int)));
                if (!(snd_mixer_selem_has_playback_volume(elem)) && !(snd_mixer_selem_has_capture_volume(elem)))
                { // ako ne e nito edno ot drugite da go nadpi6e
                    QString str =  QString("%1").arg(snd_mixer_selem_id_get_name(sid));
                    HvVText *THvVText = new HvVText(str);
                    // HLayout->addWidget(THvVText);
                    V_l->addWidget(THvVText);
                }
            }
            if (snd_mixer_selem_has_playback_volume(elem)|| snd_mixer_selem_has_capture_volume(elem))
            {
                QString str =  QString("%1").arg(snd_mixer_selem_id_get_name(sid));
                HvVText *THvVText = new HvVText(str);
                THvVText->setFixedHeight(THvVText->height()+14+2);//2->space 2 elementa po 1pix 14-> kolkoto sa radio butonite za da se izravniat textovete
                HLayout->addWidget(THvVText);
                HLayout->setAlignment(THvVText,Qt::AlignTop);

                int max_v = 0;
                if (snd_mixer_selem_has_playback_volume(elem))
                    max_v = getMaxVol(elem, false);
                if (snd_mixer_selem_has_capture_volume(elem))
                    max_v = getMaxVol(elem, true);

                QHBoxLayout *H_lpc = new QHBoxLayout();
                H_lpc->setContentsMargins ( 0, 0, 0, 0);
                H_lpc->setSpacing(2);

                if (snd_mixer_selem_has_playback_volume(elem))
                {
                    HvSlider_V_Identif *THvSlider_V_I = new HvSlider_V_Identif(idx,max_v,0,0,p1,p0,p2,p3,p4);
                    if (snd_mixer_selem_has_playback_volume(elem))
                    {
                        sldp.insert(idx, THvSlider_V_I);
                    }
                    if (snd_mixer_selem_has_playback_volume(elem) && snd_mixer_selem_has_capture_volume(elem))
                        THvSlider_V_I->SetThumbs(p7,p8);// oranzevo
                    H_lpc->addWidget(THvSlider_V_I);
                    connect(THvSlider_V_I, SIGNAL(SendValue(int,int)), this, SLOT(writeVolumeToHW(int,int)));
                }

                if (snd_mixer_selem_has_capture_volume(elem) )
                {
                    HvSlider_V_Identif *THvSlider_V_I = new HvSlider_V_Identif(idx,max_v,0,0,p1,p0,p2,p3,p4);
                    if (snd_mixer_selem_has_capture_volume(elem))
                    {
                        THvSlider_V_I->SetThumbs(p5,p6);// 4erveni
                        sldc.insert(idx, THvSlider_V_I);
                    }
                    if (snd_mixer_selem_has_playback_volume(elem) && snd_mixer_selem_has_capture_volume(elem))
                        THvSlider_V_I->SetThumbs(p7,p8);// oranzevo
                    H_lpc->addWidget(THvSlider_V_I);
                    connect(THvSlider_V_I, SIGNAL(SendValue(int,int)), this, SLOT(writeVolumeToHW(int,int)));
                }
                //  if ((snd_mixer_selem_has_playback_volume(elem)) && (snd_mixer_selem_has_capture_volume(elem)))
                //THvSlider_V_I->SetThumbs(p7,p8);// oranzevo
                //THvSlider_V_I->setFixedHeight(148);// ne e dobre za opraviane hv
                if (!snd_mixer_selem_has_playback_switch(elem))
                    V_l->setContentsMargins ( 0, (14+2), 2, 0); //14-> kolkoto sa radio butonite za da se izravniat potencionetrite
                V_l->addLayout(H_lpc);
                //connect(THvSlider_V_I, SIGNAL(SendValue(int,int)), this, SLOT(writeVolumeToHW(int,int)));
            }
            if (snd_mixer_selem_has_capture_switch(elem))
            {
                HvRbutton *Rb_t = new HvRbutton(idx);
                //Rb_t->SetRbCaptureColor(true);
                V_l->addWidget(Rb_t);
                V_l->setAlignment(Rb_t, Qt::AlignCenter);

                rbtc.insert(idx, Rb_t);

                connect(Rb_t, SIGNAL(SendVals(int,int)), this, SLOT(writeVolumeToHW(int,int)));
                if (!(snd_mixer_selem_has_playback_volume(elem)) && !(snd_mixer_selem_has_capture_volume(elem)))
                { // ako ne e nito edno ot drugite da go nadpi6e
                    QString str =  QString("%1").arg(snd_mixer_selem_id_get_name(sid));
                    HvVText *THvVText = new HvVText(str);
                    //HLayout->addWidget(THvVText);
                    V_l->addWidget(THvVText);
                }
            }
            readVolumeFromHW(elem, idx);
            HLayout->addLayout(V_l);
            QFrame *lineV = new QFrame();
            // lineV->setGeometry(QRect(70,100,3,61));
            lineV->setFrameShape(QFrame::VLine);
            lineV->setFrameShadow(QFrame::Sunken);
            //lineV->setContentsMargins(0,0,0,0);
            //lineV->setFrameStyle(QFrame::Shadow_Mask);
            HLayout->addWidget(lineV);
        }
        idx++;
        /* ------------------------------------------------------------------------------- */
    }
    HLayout->addLayout(V_CB);// enumerated combo box
    setLayout(HLayout);
    setFixedSize(QSize(HLayout->sizeHint().width(),HLayout->sizeHint().height()));

    blosk_write = false;
}

void HvAlsaMixer::addEnumerated(snd_mixer_elem_t *elem, QList<QString*>& enumList)
{
    // --- get Enum names START ---
    int numEnumitems = snd_mixer_selem_get_enum_items(elem);
    if ( numEnumitems > 0 )
    {
        // OK. no error
        for (int iEnum = 0; iEnum<numEnumitems; iEnum++ )
        {
            char buffer[100];
            int ret = snd_mixer_selem_get_enum_item_name(elem, iEnum, 99, buffer);
            buffer[99] = 0; // protect from overflow
            if ( ret == 0 )
            {
                QString* enumName = new QString(buffer); // these QString* items are deleted above (search fo "clear temporary list")
                enumList.append( enumName);
            } // enumName could be read successfully
        } // for all enum items of this device
    } // no error in reading enum list
    else
    {
        // 0 items or Error code => ignore this entry
    }
}

snd_mixer_elem_t* HvAlsaMixer::getMixerElem(int idx)
{
    snd_mixer_elem_t* elem = 0;
    if ( ! m_isOpen )
    {
        fprintf(stderr, "Error -> getMixerElem -> ! m_isOpen \n");
        return elem; // unplugging guard
    }
    if ( idx == -1 )
    {
        fprintf(stderr, "Error finding mixer element -> getMixerElem -> idx == -1 \n");
        return elem;
    }
    if ( int( mixer_sid_list.count() ) > idx )
    {
        snd_mixer_selem_id_t * sid = mixer_sid_list[ idx ];
        // The next line (hopefully) only finds selem's, not elem's.
        elem = snd_mixer_find_selem(handle_mixer, sid);
        if ( elem == 0 )
            fprintf(stderr, "Error finding mixer element -> getMixerElem ->  elem == 0 \n");
    }
    return elem;
}

int HvAlsaMixer::writeVolumeToHW(int, int devnum)
{
    if (blosk_write)
        return 0;

    int left, right;

    snd_mixer_elem_t *elem = getMixerElem( devnum );//qDebug()<<devnum ;
    if ( !elem )
    {
        return 0;
    }
    // --- playback volume
    if (snd_mixer_selem_has_playback_volume( elem ))
    {
        left  = sldp[devnum]->get_value();
        right = left;
        snd_mixer_selem_set_playback_volume ( elem, SND_MIXER_SCHN_FRONT_LEFT, left );
        if ( ! snd_mixer_selem_is_playback_mono ( elem ) )
            snd_mixer_selem_set_playback_volume ( elem, SND_MIXER_SCHN_FRONT_RIGHT, right );
        // fprintf(stderr, "playback_volume \n");
    }
    // --- playback switch
    if ( snd_mixer_selem_has_playback_switch( elem )
            ||  snd_mixer_selem_has_common_switch  ( elem ))//
    {
        int sw = 0;
        if ( !rbtp[devnum]->get_value() == 0 )
            sw = !sw; // invert all bits
        snd_mixer_selem_set_playback_switch_all(elem, sw);
        // qDebug()<<"P="<<(*rbp_from_hand[hand_iden])[devnum]->get_value();
        //fprintf(stderr, "playback_switch \n");
    }
    // --- capture volume
    if ( snd_mixer_selem_has_capture_volume( elem ))
    {
        left  = sldc[devnum]->get_value();
        right = left;
        snd_mixer_selem_set_capture_volume ( elem, SND_MIXER_SCHN_FRONT_LEFT, left );
        if ( ! snd_mixer_selem_is_playback_mono ( elem ) )
            snd_mixer_selem_set_capture_volume ( elem, SND_MIXER_SCHN_FRONT_RIGHT, right );
        //fprintf(stderr, "capture_volume \n");
    }
    // --- capture switch
    if ( snd_mixer_selem_has_capture_switch( elem ) )
    {
        //  Hint: snd_mixer_selem_has_common_switch() is already covered in the playback .
        //     switch. This is probably enough. It would be helpful, if the ALSA project would
        //     write documentation. Until then, I need to continue guessing semantics.
        int sw = 0;
        if ( rbtc[devnum]->get_value() == 1 )
            sw = !sw; // invert all bits
        snd_mixer_selem_set_capture_switch_all( elem, sw);
        // fprintf(stderr, "capture_switch \n");
    }
    //handles_refresh();
    return 0;
}

int HvAlsaMixer::readVolumeFromHW(snd_mixer_elem_t *elem, int identif)
{
    long left, right;
    int elem_sw;

    if ( !elem )
    {
        //qDebug()<<elem<<hand_iden;
        fprintf(stderr, "ERROR finding mixer element -> readVolumeFromHW -> !elem\n");
        return 0;
    }

    // --- playback volume
    if ( snd_mixer_selem_has_playback_volume( elem ) )
    {
        int ret = snd_mixer_selem_get_playback_volume( elem, SND_MIXER_SCHN_FRONT_LEFT, &left );
        if ( ret != 0 ) fprintf(stderr, "Error read voll...\n");
        if ( snd_mixer_selem_is_playback_mono ( elem ))
        {
            //return left;
            sldp[identif]->SetValue(left);
        }
        else
        {
            int ret = snd_mixer_selem_get_playback_volume( elem, SND_MIXER_SCHN_FRONT_RIGHT, &right );
            if ( ret != 0 ) fprintf(stderr, "Error read voll...\n");
            // qDebug()<<(*sp_from_hand[hand_iden])[identif]<<hand_iden<<identif;
            sldp[identif]->SetValue(right);

        }
    }

    // --- playback switch
    if ( snd_mixer_selem_has_playback_switch( elem ))
    {
        snd_mixer_selem_get_playback_switch( elem, SND_MIXER_SCHN_FRONT_LEFT, &elem_sw );
        //return elem_sw;
//        md->setMuted( elem_sw == 0 );
        rbtp[identif]->SetValue(elem_sw);
    }

    // --- capture volume
    if ( snd_mixer_selem_has_capture_volume ( elem ) )
    {
        int ret = snd_mixer_selem_get_capture_volume ( elem, SND_MIXER_SCHN_FRONT_LEFT, &left );
        if ( ret != 0 ) fprintf(stderr, "Error read voll...\n");
        if ( snd_mixer_selem_is_capture_mono  ( elem ))
        {
            sldc[identif]->SetValue(left);
        }
        else
        {
            int ret = snd_mixer_selem_get_capture_volume( elem, SND_MIXER_SCHN_FRONT_RIGHT, &right );
            if ( ret != 0 ) fprintf(stderr, "Error read voll...\n");
            /* if (!(*sc_from_hand[hand_iden])[identif])
             (*sc_from_hand[hand_iden])[identif-1]->SetValue(left);
             else	*/
            //qDebug()<<(*sc_from_hand[hand_iden])[identif]<<hand_iden<<identif;
            sldc[identif]->SetValue(right);

        }
    }

    // --- capture switch
    if ( snd_mixer_selem_has_capture_switch( elem ) )
    {
        snd_mixer_selem_get_capture_switch( elem, SND_MIXER_SCHN_FRONT_LEFT, &elem_sw );
        //return elem_sw;
        // md->setRecSource( elem_sw == 1 );
        /* if(!(*rbc_from_hand[hand_iden])[identif])
         (*rbc_from_hand[hand_iden])[identif-1]->SetValue(elem_sw);
         else	*/
        //if (identif == 3 )
        //qDebug()<<"right"<<elem_sw;
        rbtc[identif]->SetValue(elem_sw);
    }
    return 0;
}

int HvAlsaMixer::getMaxVol(snd_mixer_elem_t *elem, bool capture)
{
//    Volume* vol = 0;
    long maxVolume = 0, minVolume = 0;
    //bool capture = true;
//int vol = 0;
    // --- Regular control (not enumerated) ---
//    Volume::ChannelMask chn = Volume::MNONE;
//    Volume::ChannelMask chnTmp;
    // snd_mixer_elem_t *elem = getMixerElem(identif, hand_identif);  //qDebug()<<elem;
    if ( !elem )
    {
        fprintf(stderr, "ERROR finding mixer element -> getMaxVol -> !elem\n");
        return 0;
    }

    bool hasVolume = capture
                     ? snd_mixer_selem_has_capture_volume(elem)
                     : snd_mixer_selem_has_playback_volume(elem);
    if ( hasVolume )
    {
        //kDebug(67100) << "has_xyz_volume()";
        // @todo looks like this supports only 2 channels here
        /*        bool mono = capture
                    ? snd_mixer_selem_is_capture_mono(elem)
                    : snd_mixer_selem_is_playback_mono(elem);
                chnTmp = mono
                    ? Volume::MLEFT
                    : (Volume::ChannelMask)(Volume::MLEFT | Volume::MRIGHT);
                chn = (Volume::ChannelMask) (chn | chnTmp);*/
        if ( capture)
        {
            snd_mixer_selem_get_capture_volume_range( elem, &minVolume, &maxVolume );
        }
        else
        {
            snd_mixer_selem_get_playback_volume_range( elem, &minVolume, &maxVolume );
        }
    }

    bool hasCommonSwitch = snd_mixer_selem_has_common_switch ( elem );

    bool hasSwitch = hasCommonSwitch |
                     capture
                     ? snd_mixer_selem_has_capture_switch ( elem )
                     : snd_mixer_selem_has_playback_switch ( elem );

    if ( hasVolume || hasSwitch )
    {//qDebug()<<"play";
        //   vol = new Volume( chn, maxVolume, minVolume, hasSwitch, capture);
    }

    //qDebug()<<ii<<minVolume<<maxVolume; ii++;
    return maxVolume;
}

unsigned int HvAlsaMixer::get_enumIdHW(snd_mixer_elem_t * elem)
{
    // int devnum = id2num(id);
    // snd_mixer_elem_t *elem = getMixerElem( devnum );
    unsigned int idx = 0;

    if ( elem != 0 && snd_mixer_selem_is_enumerated(elem) )
    {
        int ret = snd_mixer_selem_get_enum_item(elem,SND_MIXER_SCHN_FRONT_LEFT,&idx);
        if (ret < 0)
        {
            idx = 0;
            fprintf(stderr, "ERROR get_enumIdHW(snd_mixer_elem_t * elem)\n");
        }
    }
    return idx;
}
void HvAlsaMixer::setEnumIdHW(int identif, int idx)
{
    //kDebug(67100) << "Mixer_ALSA::setEnumIdHW() id=" << id << " , idx=" << idx << ") 1\n";
    //int devnum = id2num(id);
    snd_mixer_elem_t *elem = getMixerElem( identif );
    int ret = snd_mixer_selem_set_enum_item(elem,SND_MIXER_SCHN_FRONT_LEFT,idx);
    if (ret < 0)
    {
        fprintf(stderr, "ERROR setEnumIdHW(int devnum, unsigned int idx)\n");
    }
    snd_mixer_selem_set_enum_item(elem,SND_MIXER_SCHN_FRONT_RIGHT,idx);
    // we don't care about possible error codes on channel 1
    return;
}

int HvAlsaMixer::OpenMixHandle(QString dev_name)
{
    int err;

    if ( ( err = snd_mixer_open ( &handle_mixer, 0 ) ) < 0 )
    {
        handle_mixer = 0;
        return err; // if we cannot open the mixer, we have no devices
    }       
    if ( ( err = snd_mixer_attach ( handle_mixer, dev_name.toLatin1().data() ) ) < 0 ) //qt5
    {
        return err;
    }

    if ( ( err = snd_mixer_selem_register ( handle_mixer, NULL, NULL ) ) < 0 )
    {

        return err;
    }

    if ( ( err = snd_mixer_load ( handle_mixer ) ) < 0 )
    {
        CloseHandle();
        return err;
    }
    m_isOpen = true;
    return 0;
}

int HvAlsaMixer::CloseHandle()
{
    int ret=0;
    m_isOpen = false;
    if ( handle_mixer != 0 )
    {
        snd_mixer_free ( handle_mixer );
            
        if ( ( ret = snd_mixer_detach ( handle_mixer, devName.toLatin1().data() ) ) < 0 ) //qt5 
        {
            return ret;
        }
        int ret2 = 0;
        if ( ( ret2 = snd_mixer_close ( handle_mixer ) ) < 0 )
        {
            if ( ret == 0 ) ret = ret2; // no error before => use current error code
        }
        handle_mixer = 0;
    }
    mixer_elem_list.clear();
    mixer_sid_list.clear();
    rbtc.clear();
    rbtp.clear();
    sldc.clear();
    sldp.clear();
    enum_cb.clear();
    //qDebug()<<"CLOSE_HANDLE";
    timer_refresh->stop();
    delete timer_refresh;
    return ret;
}
#endif





