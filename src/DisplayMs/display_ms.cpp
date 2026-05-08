/* MSHV DisplayMs
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */

#include "display_ms.h"

//#include <QtGui>

static const double DISP_SAMPLE_RATE_11025 = 11025.0;
static const double DISP_SAMPLE_RATE_12000 = 12000.0;

#define TIMER_RTD_FAST 10   //10 6
#define TIMER_RTD_SLOW 60   //80 60

DisplayMs::DisplayMs(int dispident,bool f,QWidget * parent )
        : QWidget(parent)
{
    dsty = f;
    bwQ65 = 216.667;//2.53 Q65A 30s
    dflimit = 10;
    frq00_limit = 20;
    frq01_limit = 4980;
    allq65 = false;
    //ft_df1500 = true;
    f_is_decodet3int = false;

    disp_ident = dispident;//disp_ident 0-primary 1-second display ....
    s_3intFt_d_ = false;					//2.51 default for display 1
    if (disp_ident==0) s_3intFt_d_ = true;  //2.51 default for display 0

    f_flat_vd =false;
    f_adle_vd =true;//2.51 default

    count_disp_data_dec65 = DISP_SAMPLE_RATE_12000 * 53.0;
    one_emit_disp_data_dec65 = false;
    vd_mouse_lines_draw = true;// default
    s_vd_mouse_lines_draw65 = false;// default
    f_lock_txrx = false;
    end_rtd = true;
    f_priority_disp1 = false;// for rtd
    s_fopen = false;
    QFont font_t = font();
    font_t.setPointSize(8);
    fm_X = new QFontMetrics(font_t);
    setFont(font_t);

    f_dec_pings_draw = false;
    dec_labels_posx_count = 0;
    for (int i = 0; i < MAX_DEC_LABELS_COUNT; i++) dec_labels_posx[i] = -1;

    for (int i = 0; i < MAX_DEC_LINES_COUNT; i++) dec_lines_posx[i] = -1;
    dec_lines_posx_count = 0;

    DISP_SAMPLE_RATE = DISP_SAMPLE_RATE_11025;//HV important set to default mode fsk441 sample rate
    s_mode = 2; //default mode fsk441 at start for iscat labels

    this ->setMouseTracking(true);
    setCursor(Qt::CrossCursor);
    f_auto_decode_all = false;
    trans_fac = 1.0;
    //trans_fac_time = 1.0;

    //setFixedSize(GRAPHIC_WIDTH, GRAPHIC_HEIGHT);
    this->setFixedHeight(DATA_HEIGHT);
    setMinimumSize(DATA_WIDTH, DATA_HEIGHT);//1.39 rem DATA_WIDTH/2
    x_pos = 0;
    s_upd_pos = 0;
    img_tmp = QImage(DATA_WIDTH+100, DATA_DSP_HEIGHT+100, QImage::Format_RGB32);//  DATA_HEIGHT   DATA_DSP_HEIGHT+100

    for (int j = 0; j < DATA_DSP_HEIGHT+100; j++)//y    DATA_HEIGHT   DATA_DSP_HEIGHT+100
    {
        for (int i = 0; i < DATA_WIDTH+100; i++)//x
        {
            img_tmp.setPixel(i,j,0);//rgb 0x000000 cialo iztrivane moze ida ne e -offset_wat_sm and i=offset_wat_sm
        }
    }

    f_zero_freq_scale = false;
    s_start = 100;//2.76.1 3000hz bw
    s_stop = 3300;//2.76.1 3000hz bw
    x_pos_freq_line = 25;//25pix
    f_disp_v_h = false;
    var_vdisp_height = DATA_HEIGHT*2 - x_pos_freq_line;
    pos_y_v0=-(var_vdisp_height);
    pos_y_v1=0;
    pos_y_draw_vimgs=var_vdisp_height-1;
    f_img_v0_v1 = true;
    img_tmp_v0 = QImage(DATA_WIDTH+100, var_vdisp_height, QImage::Format_RGB32);//1.39 rem DATA_WIDTH/2
    img_tmp_v1 = QImage(DATA_WIDTH+100, var_vdisp_height, QImage::Format_RGB32);//1.39 rem DATA_WIDTH/2

    for (int j = 0; j < var_vdisp_height; j++)//y
    {
        for (int i = 0; i < DATA_WIDTH+100; i++)//x
        {
            img_tmp_v0.setPixel(i,j,0);//rgb 0x000000
            img_tmp_v1.setPixel(i,j,0);//rgb 0x000000
        }
    }

    //2.24 ft8/4 df
    frq00 = 200.0; //2.76.1 3000hz bw
    frq01 = 3200.0;//2.76.1 3000hz bw
    f_mouse_pres = false;
    f_r00 = false;
    f_r01 = false;
    s_prev_vd_rx_df = 1;

    vd_rx_freq = 1270.46;//1270.46
    s_vd_df = 400;
    //CorrFrqStratStop();
    setVDispFreqScale(s_start,s_stop);
    lines_vd_t_count = 0;
    lines_vd_t_new = true;

    vd_mouse_reset0 = 10;
    vd_mouse_reset1 = 10;
    for (int i = 0; i < 42; i++)//1.55=40 for new count speads max 40 lines old=20
    {
        posy_vd_lines_t[i]=0;
        lines_vd_t_text[i]="";
    }
    posy_vd_mouse0[0]=0;
    posy_vd_mouse0[1]=0;
    posy_vd_mouse1[0]=0;
    posy_vd_mouse1[1]=0;

    vd_tx_freq = 1700.0;// only for ft8 for the moment 1.43
    vd_rx_freq_txt = "RX: "+QString("%1").arg((int)vd_rx_freq)+" Hz";
    vd_rx_df_txt = "DF: "+QString("%1").arg(0)+" Hz";

    s_ft8_vd_rxtx_freq[0] = 1700.0;
    s_ft8_vd_rxtx_freq[1] = 1700.0;
    /*s_ft8_vd_rxtx_freq[2] = 200.0;
    s_ft8_vd_rxtx_freq[3] = 2200.0;
    s_ft8_vd_rxtx_freq[4] = 1000.0;*/

    refresh_time = 0;

    last_offset_dsp = 50;//1.55=def val
    s_offset_dsp = 50;//1.55=def val
    last_contr_dsp = 12;//1.55=def val
    s_contr_dsp = 12;//1.55=def val
    diplay_ofset = 0;//1.55=def val
    diplay_contr = 0.0;//1.55=def val
    tune_display_thred_busy = false;

    f_disp_time = false;//2.43
    s_time = "NONETM";
    s_ymd = "YMD";
    p_s_time = "NONETM";//2.43
    p_s_ymd = "YMD";//2.43

    full_file_name = "FullFileNameNone";
    //f_s_decode = false;
    f_is_decodet = false;
    s_zap = false;

    temp_pal0 = QPixmap(":pic/palette_default_bw.png");
    temp_pal1 = QPixmap(":pic/palette_default_c.png");
    temp_pal2 = QPixmap(":pic/palette_1.png");
    temp_pal3 = QPixmap(":pic/palette_2.png");
    temp_pal4 = QPixmap(":pic/palette_3.png");
    temp_pal5 = QPixmap(":pic/palette_4.png");
    temp_pal6 = QPixmap(":pic/palette_5.png");
    temp_pal7 = QPixmap(":pic/palette_6.png");
    s_pen_wave = QPen(QColor(255,255,255),0);//,0 need qt5 v1.27 QPen(QColor(xxx,xxx,xxx),0,Qt::SolidLine) Qt::SolidLine=1
    s_brush_wave = QColor(0, 0, 250, 255);

    SetPaletteParm(temp_pal1);

    //setPaletteManual();

    saved_color = 1;
    s_pen_wave = QPen(QColor(255,255,255),0);// default
    s_brush_wave = QColor(0, 0, 250);// default

    raw_count = 0;
    count_refr_disp = 0;

    period_time_sec = 30; //default fsk441  (int)STATIC_FFTW_PERIOD_TIME;

    ///rtd///
    f_dec_busy = false;
    pos_rtd = 0;
    s_upd_pos_lines_rtd = 0;
    koef_pos_lines_rtd = DISP_SAMPLE_RATE/(double)DATA_WIDTH*STAT_FFTW_H30_TIME;
    s_raw_count = 0;
    //s_pos_rtd_end = 0;

    //full_raw_count = DISP_SAMPLE_RATE*period_time_sec;
    //f_end_pos_rtd = 0;
    ///rtd///

    dec_array_pixel_begin = 10;//begin from senter
    dec_array_pixel_end = 10;// end from senter
    // offset_wat_sm = 46;//46;//offset waterful from smiter;
    setHDispPeriodTime(period_time_sec,s_mode);
    //ClarDisplay();
    //SetSampleRate(sampe_rate);
    x_pos_timeline = 16;//20

    offset_down_sm = 52;//52 offset down of smiter work > -10bd else =0

    ClarDisplay();

    //setDragDropMode(QAbstractItemView::NoDragDrop);//QAbstractItemView::InternalMove
    //setDragEnabled (false);

    timer_rtd_ = new QTimer();
    connect(timer_rtd_, SIGNAL(timeout()), this, SLOT(RtdDecode()));
    s_start_stop_rtd_timer = false;
    s_rtd_timer_is_active = false;
    f_once_sw_timer_rtd_speed = false;

    //ft8 3 dec
    f_timerft8 = false;
    //isa_timerft8 = false;
    timer_ft8_ = new QTimer();
    connect(timer_ft8_, SIGNAL(timeout()), this, SLOT(Ft8Decode()));
    timer_ft8_->setSingleShot(true);
    
    line_bw = 0;
    mpos_x_bw = 0;
    f_show_bw = false;
    s_show_bw = false;
    s_show_bwrx = true;//2.76.1
    s_show_bwtx = true;//2.76.1
    
    id_mshf = 0;

    update();
    //s_old_graph_width = width();
}

DisplayMs::~DisplayMs()
{}
void DisplayMs::SetFont(QFont f)
{
    QFont font_t = f;
    font_t.setPointSize(f.pointSize()-1);//font_t.setPointSize(8);
    fm_X = new QFontMetrics(font_t);
    setFont(font_t);
    if (f_disp_v_h) setVDispFreqScale(s_start,s_stop);
    else setHDispPeriodTime(period_time_sec,s_mode);
}
void DisplayMs::SetVDMouseDrawLines(bool f)//1.49
{
    s_vd_mouse_lines_draw65 = f;
    if ((s_mode == 7 || s_mode == 8 || s_mode == 9) && s_vd_mouse_lines_draw65) vd_mouse_lines_draw = false;
    else vd_mouse_lines_draw = true;
    update();
}
void DisplayMs::SetVDMouseBWDrawLines(bool f)//2.72
{
    s_show_bw = f;
    update();
}
void DisplayMs::SetVDMouseBWRXDrawLines(bool f)//2.72
{
    s_show_bwrx = f;
    update();
}
void DisplayMs::SetVDMouseBWTXDrawLines(bool f)//2.72
{
    s_show_bwtx = f;
    update();
}
void DisplayMs::SaveFT8RxTxFreq()
{
    if (s_mode==11 || s_mode==13 || s_mode==18 || allq65)//only FT8 FT4 q65
    {
        s_ft8_vd_rxtx_freq[0] = vd_rx_freq;
        s_ft8_vd_rxtx_freq[1] = vd_tx_freq;
        /*s_ft8_vd_rxtx_freq[2] = frq00;
        s_ft8_vd_rxtx_freq[3] = frq01;
        s_ft8_vd_rxtx_freq[4] = s_vd_df;*/
        //qDebug()<<"Save="<<disp_ident<<frq00<<frq01<<s_vd_df;
    }
}
QString DisplayMs::GetFreqRxTxVD()
{
    QString out;
    out = QString("%1").arg((int)s_ft8_vd_rxtx_freq[0]);//(vd_rx_freq,0,'f',1);
    out.append("#");
    out.append(QString("%1").arg((int)s_ft8_vd_rxtx_freq[1]));//(vd_tx_freq,0,'f',1));
    /*out.append("#");
    out.append(QString("%1").arg((int)s_ft8_vd_rxtx_freq[2]));  
    out.append("#");
    out.append(QString("%1").arg((int)s_ft8_vd_rxtx_freq[3]));   
    out.append("#");
    out.append(QString("%1").arg((int)s_ft8_vd_rxtx_freq[4]));*/
    return out;
}
void DisplayMs::SetFreqRxTxVD(QString rxtx)
{
    QStringList ls = rxtx.split("#");
    if (ls.count()>1)
    {
        s_ft8_vd_rxtx_freq[0] = ls.at(0).toDouble();
        s_ft8_vd_rxtx_freq[1] = ls.at(1).toDouble();
        /*s_ft8_vd_rxtx_freq[2] = ls.at(2).toDouble();
        s_ft8_vd_rxtx_freq[3] = ls.at(3).toDouble();
        s_ft8_vd_rxtx_freq[4] = ls.at(4).toDouble();*/
    }
}
void DisplayMs::SetFreqExternal(double frq)
{
    vd_rx_freq = frq;
    vd_rx_freq_txt = "RX: "+QString("%1").arg((int)vd_rx_freq)+" Hz";
    //CorrFrqStratStop();
    //emit EmitVDRxFreq(vd_rx_freq,frq00,frq01);
    SetVDRxFreqF0F1();
    if (f_lock_txrx)
    {
        vd_tx_freq = vd_rx_freq;
        SetVDTxFreq();
    }
    setVDispFreqScale(s_start,s_stop);
    SaveFT8RxTxFreq();
}
void DisplayMs::SetLockTxrx(bool f)
{
    f_lock_txrx = f; //if (f_lock_txrx) SetTxToRx(true);// true  fictive
    if (f_lock_txrx)
    {   	
    	if      (s_mode==11 && id_mshf==2) SetRxToTx(true); 
    	else if (s_mode==11 && id_mshf==1) SetTxToRx(true);
    	else SetTxToRx(true);
   	}
}
void DisplayMs::SetTxToRx(bool)
{
    vd_tx_freq = vd_rx_freq;
    SetVDTxFreq();
    setVDispFreqScale(s_start,s_stop);
    SaveFT8RxTxFreq();  //qDebug()<<"disp_ident="<<disp_ident<<vd_tx_freq;
}
void DisplayMs::SetRxToTx(bool)//2.63
{
    vd_rx_freq = vd_tx_freq;
    vd_rx_freq_txt = "RX: "+QString("%1").arg((int)vd_rx_freq)+" Hz";
    //CorrFrqStratStop();
    //emit EmitVDRxFreq(vd_rx_freq,frq00,frq01);
    SetVDRxFreqF0F1();
    setVDispFreqScale(s_start,s_stop);
    SaveFT8RxTxFreq();
}
void DisplayMs::SetMode(int mode)
{
    //2.41 importent reset here
    setCursor(Qt::CrossCursor); //qDebug()<<"DisplayMs"<<disp_ident<<mode;
    f_mouse_pres = false;
    f_r00 = false;
    f_r01 = false;

    if (mode == 14 || mode == 15 || mode == 16 || mode == 17) allq65 = true;
    else allq65 = false;
    s_mode = mode;
    /*if (mode==0 || mode == 7 || mode == 8 || mode == 9 || mode == 10 || mode == 11 || mode == 12 || mode == 13 || allq65)//msk144 msk144ms jt65abc pi4 ft8 ft4
        DISP_SAMPLE_RATE = DISP_SAMPLE_RATE_12000;
    else
        DISP_SAMPLE_RATE = DISP_SAMPLE_RATE_11025;*/    
    if (mode>0 && mode<7) //2.65
        DISP_SAMPLE_RATE = DISP_SAMPLE_RATE_11025;
    else
    	DISP_SAMPLE_RATE = DISP_SAMPLE_RATE_12000;            

    //start v1.49
    for (int i = 0; i < 4 ; i++) posx_vd_mouse_df[i] = 0;
    for (int i = 0; i < 2 ; i++)
    {
        posy_vd_mouse0[i] = 0;
        posy_vd_mouse1[i] = 0;
    }
    if ((s_mode == 7 || s_mode == 8 || s_mode == 9) && s_vd_mouse_lines_draw65) vd_mouse_lines_draw = false;
    else vd_mouse_lines_draw = true;
    //end
    if (mode == 7 || mode == 8 || mode == 9 || mode == 10 || mode == 11 || mode == 13  || mode == 18 || allq65)//jt65abc pi4 ft8 f44
    {
        f_disp_v_h = true;

        dflimit = 10;//jt65
        if (s_mode==11) dflimit = 60; //50 ft8
        else if (s_mode==13 || s_mode==10) dflimit = 110;//100 ft4 or pi4
        else if (s_mode==18) dflimit = 210;//ft2 
        else if (allq65) dflimit = 50;//q65 limit  10hz

        if (mode == 10) vd_rx_freq = 682.8125; //pi4

        else if (mode == 11 || mode == 13 || mode == 18 || allq65) //ft8 ft4 za sega s 1200
        {
            vd_rx_freq = s_ft8_vd_rxtx_freq[0];
            vd_tx_freq = s_ft8_vd_rxtx_freq[1];
            if (s_mode==11 && id_mshf==2) vd_tx_freq = 750;//2.76sh
			SetVDTxFreq();
            frq00 = s_start+10;//no simetric DF no more then s_start  s_ft8_vd_rxtx_freq[2];
            frq01 = s_stop -10;//no simetric DF no more then s_stop   s_ft8_vd_rxtx_freq[3];
            //frq00 = vd_rx_freq - 1000.0;//simetric DF    s_ft8_vd_rxtx_freq[2];
            //frq01 = vd_rx_freq + 1000.0;//simetric DF    s_ft8_vd_rxtx_freq[3];
            //s_vd_df = s_ft8_vd_rxtx_freq[4];
            //if (disp_ident==0) qDebug()<<frq00<<frq01;
        }
        else vd_rx_freq = 1270.46;

        vd_rx_freq_txt = "RX: "+QString("%1").arg((int)vd_rx_freq)+" Hz";
        //CorrFrqStratStop();
        //emit EmitVDRxFreq(vd_rx_freq,frq00,frq01);
        RefreshLimits(); //if (disp_ident==0) qDebug()<<"MODE"<<mode;
        //qDebug()<<frq00_limit<<frq00<<frq01_limit<<frq01;
        SetVDRxFreqF0F1();
        setVDispFreqScale(s_start,s_stop);//for q65 stop
        CorrDfW(s_vd_df);  //2.53 (frq01-frq00)/2
        //setHDispPeriodTime(period_time_sec);//for q65 start
        ClarHDisplay();
    }
    else
    {
        //if (f_disp_v_h) f_disp_time = false;//no show time
        f_disp_v_h = false;
        setHDispPeriodTime(period_time_sec,s_mode);

        ///////// ClarVDisplay() ///////////////////////////////////////////////////////////////
        lines_vd_t_new = true; // vazno da go ima tuk 1.37 1-min then fsk144 and then jt65
        vd_mouse_reset0 = 10;  // vazno ina4e ima 1 linia pove4e 1.37
        vd_mouse_reset1 = 10;  // vazno ina4e ima 1 linia pove4e 1.37
        lines_vd_t_count=0;    // vazno ina4e ima 1 linia pove4e 1.37
        for (int j = 0; j < DATA_HEIGHT*2; j++)
        {
            for (int i = 0; i < DATA_WIDTH ; i++) data_graph_all_v[j][i]=0;
        }
        /*stop v1.49
        for (int i = 0; i < 4 ; i++)
            posx_vd_mouse_df[i] = 0;
        for (int i = 0; i < 2 ; i++)
        {
            posy_vd_mouse0[i] = 0;
            posy_vd_mouse1[i] = 0;
        }*/
        for (int i = 0; i < 42; i++)//1.55=40 for new count speads max 40 lines old=20
        {
            posy_vd_lines_t[i]=0;
            lines_vd_t_text[i]="";
        }
        ///////// END ClarVDisplay() /////////////////////////////////////////////////////////////
    }


    koef_pos_lines_rtd = DISP_SAMPLE_RATE/(double)DATA_WIDTH*STAT_FFTW_H30_TIME;
    //setPeriodTime(period_time_sec);
    //koef_pos_lines_rtd = (((double)period_time_sec*DISP_SAMPLE_RATE)/(double)width())*trans_fac;

    //2.41 stop importent reset
    /*setCursor(Qt::CrossCursor);
    f_mouse_pres = false;
    f_r00 = false;
    f_r01 = false;*/
}
void DisplayMs::setArrayInPxel(int pxl)
{
    dec_array_pixel_begin = pxl;//begin from senter
    dec_array_pixel_end = pxl;
}
void DisplayMs::SaveFile(QString call_mod)
{
    QString str = call_mod+"_"+s_ymd+"_"+s_time;
    full_file_name = str+".WAV";//zaradi qlabel da se vizda
    emit EmitDataToSaveInFile(raw,raw_count,str);
}
void DisplayMs::setDisplayTime(QString str,QString str1)// from maim_ms file open
{
    p_s_time = s_time;//2.43
    p_s_ymd = s_ymd;//2.43

    s_time = str;
    s_ymd = str1;
    f_disp_time = true;//2.43

    emit EmitFileNameChenged();

    if (f_disp_v_h)//s_time!="NONETM" &&
        lines_vd_t_new = true;

    //2.44 reset here exception same time from file open and no SetSyncPosition
    f_is_decodet3int = false;
    //qDebug()<<"setDisplayTime"<<disp_ident<<s_time<<s_fopen;
}
void DisplayMs::setHDispPeriodTime(int t,int m) //setPeriodTime(int t)
{
    //qDebug()<<width()<<DATA_WIDTH;
    int g_width = width();//GRAPHIC_WIDTH;//width();1259;//

    period_time_sec = t;
    //trans_fac_time = 30.0/(double)t;

    //if (f_disp_v_h)
    //trans_fac = ((double)g_width/((double)DATA_WIDTH/2.0));
    //else
    trans_fac = ((double)g_width/(double)DATA_WIDTH)*(STAT_FFTW_H30_TIME/(double)t);

    //qDebug()<<width()<<DATA_WIDTH<<trans_fac;
    //qDebug()<<trans_fac;
    //trans_lines_rtd = (double)g_width/((double)t*DISP_SAMPLE_RATE);
    //trans_lines_rtd = (((double)t*DISP_SAMPLE_RATE)/(double)g_width)*trans_fac;
    //full_raw_count = DISP_SAMPLE_RATE*period_time_sec;
    //koef_pos_lines_rtd = DISP_SAMPLE_RATE/(double)DATA_WIDTH*STATIC_FFTW_PERIOD_TIME;

    // zavisi ot nai svitia razmer na displea za da ne izliza dupe nadpisa izvan
    // za 30s=1.5 15s=0.75
    if (period_time_sec == STAT_FFTW_H30_TIME)
        ping_pos_end_draw=period_time_sec-1.5; /// v int 760
    else
        ping_pos_end_draw=period_time_sec-0.75;/// v int 380



    //hv vazno da e double scale_x_inc_pix
    double scale_x_inc_pix = ((double)g_width/(double)period_time_sec); //26;// prez kolko piksela da sa

    //double fft_size = (double)((double)sampe_rate/(double)scale_x_inc_pix);
    //qDebug()<<"PerPixel="<<scale_x_inc_pix<<"fft_size="<<fft_size<<"<-Triabva da sa celi 4isla s podbor na DATA_WIDTH";

    count_lines_scale_x = (g_width / scale_x_inc_pix)+3;// vnimanie garmi
    int line_height_pix = 2;

    //qDebug()<<count_lines_scale_x;

    double save_x_inc = 0.0;//hv vazno da e double

    pos_text_hor_Y = x_pos_timeline-line_height_pix-2;//2<-pix otstoianieto na cifrite ot 4ertite
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)//2.56
    int off_from_font_1 = fm_X->horizontalAdvance("8")/2;
    int off_from_font_10 = fm_X->horizontalAdvance("88")/2;
#else
    int off_from_font_1 = fm_X->width("8")/2;
    int off_from_font_10 = fm_X->width("88")/2;
#endif
    for (int i = 0; i<count_lines_scale_x; ++i)
    {
        lines_scale_x[i] = QLine(0+save_x_inc,x_pos_timeline-line_height_pix,0+save_x_inc,x_pos_timeline);

        if (i == 0)
            pos_text_hor_X[i] = 0+save_x_inc+off_from_font_1*2;
        else if (i<10)
            pos_text_hor_X[i] = 0+save_x_inc-off_from_font_1;
        else
            pos_text_hor_X[i] = 0+save_x_inc-off_from_font_10;

        //if (save_x_inc > (GRAPHIC_WIDTH-off_from_font_10))
        if (save_x_inc > (g_width-off_from_font_10))
            pos_text_hor_X[i] = save_x_inc-off_from_font_10*2-3;// poslednata cifra

        save_x_inc += scale_x_inc_pix;
    }
    update();
    //qDebug()<<"setHDispPeriodTime"<<s_mode<<period_time_sec;

    if (allq65 && m==s_mode) //2.53
    {
        RefreshLimits();  //if (disp_ident==0) qDebug()<<"setHDispPeriodTime MODE="<<s_mode<<"Ptime="<<period_time_sec;
        SetVDRxFreqF0F1();
        setVDispFreqScale(s_start,s_stop);//q65 ???
        CorrDfW(s_vd_df); //(frq01-frq00)/2
    }
}
//int jjj=0;
void DisplayMs::DrawAll()
{
    for (;;)
    {
        diplay_ofset = 50 - s_offset_dsp;
        double t_cor = (s_contr_dsp - 12);
        if (t_cor>0.0)
            diplay_contr = t_cor/5.0;//5.0=3.4max <- for BW palette     or 8.0=2.5max or 10.0=2.2max
        else
            diplay_contr = t_cor/24.0;

        if (f_disp_v_h)
            PaintData_VD(true);
        else
            PaintData_HD(true);

        usleep(25000);

        //jjj++; qDebug()<<"DrawAll="<<disp_ident<<jjj;
        if (last_offset_dsp!=s_offset_dsp || last_contr_dsp!=s_contr_dsp)
        {
            s_offset_dsp = last_offset_dsp;
            s_contr_dsp  = last_contr_dsp;
        }
        else
        {
            tune_display_thred_busy = false;  //jjj=0;
            return;
        }
    }
}
void DisplayMs::setTune(int offset, int contr)
{
    last_offset_dsp = offset;
    last_contr_dsp = contr;

    if (tune_display_thred_busy) return;
    tune_display_thred_busy = true;

    s_offset_dsp = offset;
    s_contr_dsp = contr;

    if (disp_ident==0) pthread_create(&th_t0,NULL,DisplayMs::ThreadDrawAll0,(void*)this);
    if (disp_ident==1) pthread_create(&th_t1,NULL,DisplayMs::ThreadDrawAll1,(void*)this);
}
void *DisplayMs::ThreadDrawAll0(void *argom)
{
    DisplayMs* pt = (DisplayMs*)argom;
    pt->DrawAll();
    pthread_detach(pt->th_t0);
    pthread_exit(NULL);//2.74
    return NULL;
}
void *DisplayMs::ThreadDrawAll1(void *argom)
{
    DisplayMs* pt = (DisplayMs*)argom;
    pt->DrawAll();
    pthread_detach(pt->th_t1);
    pthread_exit(NULL);//2.74
    return NULL;
}
void DisplayMs::ReciveClarDisplay()// from player file open
{
    QList<int*>  arr;
    for (int j = 0; j < DATA_WIDTH; j++)
    {
        arr.append(data_graph_all[j]);
        if (j>=s_upd_pos)
            SetWavePoints(j);
    }
    /*
       emit SentData(arr,wave_points,raw_count,raw,t_s_time,t_s_ymd,true,lines_mouse,
                dec_labels_posx,dec_labels_text,fm_dec_labels_txt,dec_labels_posx_count,dec_lines_posx,dec_lines_posx_count,
                s_fopen,lines_rtd,pos_rtd,s_upd_pos_lines_rtd,end_rtd);//no decode //1.27 psk rep   fopen bool true    false no file open
       */

    //qDebug()<<"ReciveClarDisplay"<<disp_ident<<p_s_time<<s_fopen;

    emit SentData(arr,wave_points,raw_count,raw,p_s_time,p_s_ymd,true,lines_mouse,
             dec_labels_posx,dec_labels_text,fm_dec_labels_txt,dec_labels_posx_count,dec_lines_posx,dec_lines_posx_count,
             s_fopen,lines_rtd,pos_rtd,s_upd_pos_lines_rtd,end_rtd);

    ClarDisplay();
}
void DisplayMs::SetAutoDecodeAll(bool f)
{
    f_auto_decode_all = f;
}
void DisplayMs::SeavPrevRaw()
{
    //////  PI4 prew per rav //////////////////////////////
    //if(s_mode==10)
    for (int i = 0; i < RAW_BUF_PREV_PI4; i++)
        raw_prev_pi4[i] = 0;
    int beg = raw_count - RAW_BUF_PREV_PI4;
    int end = raw_count - 1;
    if (beg<0) beg = 0;
    if (end<0) end = 0;
    int raw_p_pi4 = RAW_BUF_PREV_PI4 - 1;
    for (int i = end; i >= beg; i--)
    {
        raw_prev_pi4[raw_p_pi4]=raw[i];
        if (raw_p_pi4>0)
            raw_p_pi4--;
    }
    //qDebug()<<"SAVE disp_ident="<<disp_ident<<"SAVE="<<raw_prev_pi4[10]<<raw_prev_pi4[20]<<raw_prev_pi4[30]<<raw_prev_pi4[40]<<raw_prev_pi4[50];
    ////// end PI4 prew per rav //////////////////////////////
}
void DisplayMs::GetRawAll(int &cou_t)
{
    cou_t=0;
    //// pi4 add prev period end seconds////////////////////////////////////
    if (s_mode==10)
    {
        //double test = raw_prev_pi4[2000]+raw_prev_pi4[4000]+raw_prev_pi4[6000]+raw_prev_pi4[8000]+raw_prev_pi4[10000];
        //if (test!=0.0)// && raw_prev_pi4[500]!=0.0 && raw_prev_pi4[600]!=0.0)
        //{
        for (int j = 0; j<RAW_BUF_PREV_PI4; ++j)
        {
            raw_t[cou_t]=raw_prev_pi4[j];
            cou_t++;
        }
        //qDebug()<<"USE disp_ident="<<disp_ident<<"USE="<<raw_t[10]<<raw_t[20]<<raw_t[30]<<raw_t[40]<<raw_t[50];
        //}
    }
    //// end pi4 add prev period end seconds////////////////////////////////////
    //DISP_SAMPLE_RATE*60
    //DISP_SAMPLE_RATE*30
    for (int j = 0; j<raw_count; ++j)
    {
        raw_t[cou_t] = raw[j];
        cou_t++;
    }
}
void DisplayMs::Ft8Decode()//ft8 3 dec
{
    int count_t;
    GetRawAll(count_t);
    if (f_timerft8)  //mousebutton Left=1, Right=3 fullfile=0 rtd=2 ft8=4,5,6
    {
        f_timerft8=false;
        timer_ft8_->start(1000);//2.70=1000 old=1200  3=172800 min samples
        emit EmitDataToDecode(raw_t,count_t,s_time,0,5,false,true,s_fopen);
    }
    else
    {
        timer_ft8_->stop();
        emit EmitDataToDecode(raw_t,count_t,s_time,0,6,false,true,s_fopen);
        //isa_timerft8 = false;
    }
}
void DisplayMs::SetMsh(uint8_t id)//2.76sh
{
	id_mshf = id;	
	if (s_mode==11 && id_mshf>0) 
	{
		if (id_mshf==2)
		{
			vd_tx_freq = 750;//2.76sh
			SetVDTxFreq();			
		}	
		if (id_mshf==1) 
		{
			vd_rx_freq = 750;//2.76sh
			vd_rx_freq_txt = "RX: "+QString("%1").arg((int)vd_rx_freq)+" Hz";
			SetVDRxFreqF0F1();
		}		
	}	
    setVDispFreqScale(s_start,s_stop);//for display	
    SaveFT8RxTxFreq();
}
void DisplayMs::Decode3intFt(bool f)
{
    if (disp_ident==0) s_3intFt_d_ = f;
}
void DisplayMs::DecodeAllData(bool decode, bool external,bool f_fi_se)
{
    //if (isa_timerft8) return;//protection
    if (f_auto_decode_all)
    {
        int count_t;
        GetRawAll(count_t);
        //2.39 mousebutton Left=1, Right=3 fullfile=0 rtd=2 ft8=4,5,6
        if (external) //2.70 ft8 rx to decode  11800 -1700- 13500 -1000- 14500 //old=11800 -1700- 13500 -1200- 14700
        {			 
            if (decode)
            {
                bool fsdecod = false; //f_fi_se=1 (even first), or f_fi_se=0 (odd second)
                if (s_mode==11 && id_mshf>0 && f_fi_se) fsdecod = true;
                //&& ((id_mshf==1 && ft8_even_odd(s_time)) || id_mshf==0)//2.76
                if (s_3intFt_d_ && s_mode==11 && !fsdecod)//2.76sh tbd...    ft8 3 dec 
                {
                    //2.44 special flag f_is_decodet3int no decode 2 times if fast click in 1s
                    if (!timer_ft8_->isActive() && !f_is_decodet3int)//&& !f_is_decodet3int
                    {
                        f_is_decodet3int = true;
                        f_timerft8 = true;
                        timer_ft8_->start(1740);//2.71 0ld=1700  need //int n=47*3456=162432 samples for 2
                        emit EmitDataToDecode(raw_t,count_t,s_time,0,4,false,true,s_fopen);
                    }
                }
                else emit EmitDataToDecode(raw_t,count_t,s_time,0,0,false,true,s_fopen);
                f_is_decodet = true; //qDebug()<<"DecodeExternal D1"<<s_time;
            }
            else f_is_decodet = false;
        }
        else
        {
            if (!decode)
            {
                //1.27 psk rep fopen //mousebutton Left=1, Right=3 fullfile=0 rtd=2 ft8=4
                emit EmitDataToDecode(raw_t,count_t,s_time,0,0,false,true,s_fopen);
                //qDebug()<<"DecodeInternal D2";
            }
        }
    }
}
void DisplayMs::ResetToBegin()//QString hm,QString ymd
{
    // pazi da ne se iztrie za6toto ResetToBegin() stava niakolko pati s pomo6ta na x_pos
    // to na ClarDisplay(); e stanalo x_pos=0;
    //if (x_pos>GRAPHIC_WIDTH/2)
    //{
    QList<int*>  arr;
    //QPoint wave_points1[DATA_WIDTH+1+2];

    for (int j = 0; j < DATA_WIDTH; j++)
    {
        arr.append(data_graph_all[j]);
        if (j>=s_upd_pos)
            SetWavePoints(j);
    }

    emit SentData(arr,wave_points,raw_count,raw,s_time,s_ymd,f_is_decodet,lines_mouse,
             dec_labels_posx,dec_labels_text,fm_dec_labels_txt,dec_labels_posx_count,dec_lines_posx,dec_lines_posx_count,
             s_fopen,lines_rtd,pos_rtd,s_upd_pos_lines_rtd,end_rtd);//1.27 psk rep   fopen bool true    false no file open

    //qDebug()<<"ResetToBegin"<<raw_count<<disp_ident;;

    emit EmitFileNameChenged();

    ClarDisplay();
    emit EmitResetToBegin();

    if (f_disp_v_h)//s_time!="NONETM" &&
        lines_vd_t_new = true;
    //qDebug()<<"ResetToBegin"<<s_time<<disp_ident;
}
void DisplayMs::SetSyncPosition(int msec,QString hm,QString ymd,bool decodet)
{
    int limit;
    if ( s_time!=hm || s_ymd != ymd )
    {
        //qDebug()<<"Fdec="<<s_time<<hm;
        if (f_disp_v_h) f_is_decodet = decodet;
        ResetToBegin();
        f_is_decodet = false;
        f_is_decodet3int = false;
    }
    //else
    //f_need_decode = false;
    //qDebug()<<"Fdec="<<f_need_decode;

    //int limit = (int)((double)(msec * (DATA_WIDTH-1)) / (period_time_sec*1000.0));//pazi ot prepalvane
    if (f_disp_v_h)
        limit = (int)((double)(msec * (DATA_WIDTH-1)) / (MAX_OPEN_WAW_TIME*1000.0));//2.53 for q65=2min max   STAT_FFTW_V60_TIME
    else
        limit = (int)((double)(msec * (DATA_WIDTH-1)) / (STAT_FFTW_H30_TIME*1000.0));

    //qDebug()<<limit<<(DATA_WIDTH-1)<<msec;
    if (limit <= (DATA_WIDTH-1))
    {
        x_pos = limit;
        raw_count = (int)((double)(DISP_SAMPLE_RATE*(double)msec)/1000.0);
        //qDebug()<<raw_count<<limit;
    }
    s_time = hm;
    s_ymd = ymd;  //qDebug()<<"SetSyncPosition"<<disp_ident<<s_fopen<<s_time;
    f_disp_time = true;//2.43
    emit EmitFileNameChenged();
}
void DisplayMs::SetRriorityDisp(bool f)
{
    f_priority_disp1 = f;
}
void DisplayMs::SetDecBusy(bool f,int)
{
    f_dec_busy = f;

    /*if (f_dec_busy)
        setCursor(Qt::ForbiddenCursor);
    else
        setCursor(Qt::CrossCursor);*/
    /*QPixmap cursor_pixmap = QPixmap(":cursor_default");
    QCursor cursor_default = QCursor(cursor_pixmap, 0, 0);
    setCursor(cursor_default);*/
}
void DisplayMs::SetStartStopRtd(bool f)
{
    //qDebug()<<s_raw_count<<raw_count<<s_fopen;
    if (f && raw_count!=0 && disp_ident==0)//disp_ident 0-primary 1-second display
    {
        //qDebug()<<"s_raw_count<<raw_count-----------------";
        s_rtd_timer_is_active = true; // trqbva pri spriano rtd a ima file za decoding
        timer_rtd_->start(TIMER_RTD_SLOW);//60ms<-ok 30ms<-lighting no acumulated raw count tested all OS Lin Win
        //qDebug()<<"Timer START SetStartStopRtd()"<<disp_ident;
    }
    /*else
    {
        // da ne pokazva lines_rtd v na4aloto pri spriano rtd i postaven now fail za decode
        //if (s_pos_rtd_end == raw_count)
        //timer_rtd_->stop();
    }*/
    s_start_stop_rtd_timer = f;
}

//static const int M_COUN_RAW = 8000; //good pos is 8106 lost=70ms or 7731 lost=45ms max bufer is 375
//int kkk = 0;
#define M_COUN_RAW 8000 //good pos is 8106 lost=70ms or 7731 lost=45ms max bufer is 375
void DisplayMs::RtdDecode()
{
    //qDebug()<<"Tim="<<disp_ident<<"kkk="<<kkk;
    //kkk++; if(kkk>100) kkk=0;

    if (f_dec_busy || f_priority_disp1)
        return;

    //qDebug()<<"Tim="<<disp_ident<<"dec="<<f_dec_busy<<f_priority_disp1;

    bool f_raw_stop = false;
    bool f_decode = false;
    bool f_decode_to_end = false;
    int min_count_raw = raw_count-pos_rtd;

    if (raw_count<pos_rtd)
    {
        pos_rtd = 0;
        s_upd_pos_lines_rtd = 0;
    }

    ///////// Vareable speed rtd timer /////////////////////////////////
    if (min_count_raw > M_COUN_RAW*2 && !f_once_sw_timer_rtd_speed)
    {
        timer_rtd_->setInterval(TIMER_RTD_FAST);//6ms
        f_once_sw_timer_rtd_speed = true;  //qDebug()<<"speed=10ms";
    }
    if (min_count_raw < M_COUN_RAW*2 && f_once_sw_timer_rtd_speed)
    {
        timer_rtd_->setInterval(TIMER_RTD_SLOW);//60ms
        f_once_sw_timer_rtd_speed = false; //qDebug()<<"speed=60ms";
    }
    ///////// End Vareable speed rtd timer /////////////////////////////

    if (min_count_raw > M_COUN_RAW)
        f_decode = true;
    else
        f_decode = false;

    if (s_raw_count != raw_count)
    {
        f_raw_stop = false;
        s_raw_count = raw_count;
    }
    else
        f_raw_stop = true;

    //if ( f_raw_stop && min_count_raw <= M_COUN_RAW && s_pos_rtd_end != raw_count)//
    if ( f_raw_stop && min_count_raw <= M_COUN_RAW && !end_rtd)//
    {
        f_decode = true;
        f_decode_to_end = true;
        //qDebug()<<"f_decode_to_end================="<<disp_ident;
    }

    end_rtd = false; // here for eny case

    if (f_decode)//7168 3600 && !f_dec_busy
    {
        //qDebug()<<"f_raw_stop----------"<<kkk;
        //kkk = 0;

        int pos_rtd_beg = pos_rtd;
        int pos_rtd_end = pos_rtd+7168;

        if (f_decode_to_end)
        {
            pos_rtd_beg = raw_count-7168;
            pos_rtd_end = raw_count;
            //qDebug()<<"pos_rtd_beg"<<pos_rtd_beg<<pos_rtd_end;
            if (pos_rtd_beg < 0)
                pos_rtd_beg = 0;
            f_decode_to_end = false;
            end_rtd = true;
        }

        //s_pos_rtd_end = pos_rtd_end;

        int count_t = 0;

        //qDebug()<<pos_rtd_beg<<pos_rtd_end<<raw_count<<pos_rtd_end-pos_rtd_beg;

        for (int j = pos_rtd_beg; j<pos_rtd_end; j++)
        {
            raw_t[count_t] = raw[j];
            count_t++;
        }

        if (!s_start_stop_rtd_timer || end_rtd)//
        {
            timer_rtd_->stop();
            s_rtd_timer_is_active = false;
            end_rtd = true;
            emit EmitRriorityDisp(false);
            //qDebug()<<"Timer STOP RtdDecode()"<<disp_ident;
        }


        //qDebug()<<"StartE"<<count_t;
        //1.27 psk rep   fopen bool true    false no file open
        emit EmitDataToDecode(raw_t,count_t,s_time,pos_rtd_beg,2,true,end_rtd,s_fopen);//mousebutton Left=1, Right=3 fullfile=0 rtd=2

        //qDebug()<<"EndtartE";

        int b_pos_rtd = (double)pos_rtd_beg/koef_pos_lines_rtd;
        int e_pos_rtd = (double)pos_rtd_end/koef_pos_lines_rtd;
        //qDebug()<<b_pos_rtd<<e_pos_rtd<<e_pos_rtd-b_pos_rtd<<"=="<<pos_rtd_beg<<pos_rtd_end<<pos_rtd_end-pos_rtd_beg;

        if (e_pos_rtd>DATA_WIDTH-1)//1.27 -1
        {
            b_pos_rtd -= abs(e_pos_rtd-(DATA_WIDTH-1));//1.27 -1
            e_pos_rtd = DATA_WIDTH-1; //1.27 -1 d_a_p_end = DATA_WIDTH;
            //qDebug()<<e_pos_rtd<<DATA_WIDTH;
        }

        for (int i = 0; i<COUNT_MOUSE_LINES; i++)
        {
            if (i==0)
                lines_rtd[i] = QLine(b_pos_rtd, x_pos_timeline, b_pos_rtd, height());

            if (i==1)
                lines_rtd[i] = QLine(e_pos_rtd, x_pos_timeline, e_pos_rtd, height());
        }

        int upd_pos_begin = (s_upd_pos_lines_rtd - 2);// 1.27
        if (upd_pos_begin<0)                          // 1.27
            upd_pos_begin = 0;
        int upd_pos_end = (e_pos_rtd-upd_pos_begin + 4);//2 to 4  1.27
        //if(upd_pos_begin+upd_pos_end>DATA_WIDTH)
        //upd_pos_end = (upd_pos_begin+upd_pos_end)-DATA_WIDTH;

        s_upd_pos_lines_rtd = b_pos_rtd;
        //qDebug()<<upd_pos_begin<<upd_pos_end<<DATA_WIDTH;

        update(upd_pos_begin*trans_fac,0,upd_pos_end*trans_fac,height());
        //if(pos_rtd == 0)
        //update();
        //s_pos_rtd_beg = pos_rtd;

        //if(pos_rtd+4000<)
        pos_rtd = pos_rtd_beg + 3583;// ->3583 3600; 3 perioda zastypwa

        //qDebug()<<"OUT";
    }
}
void DisplayMs::SetRaw(int *raw_in,int count,bool ffopen)  //1.27 psk rep   fopen bool true    false no file open
{
    s_fopen = ffopen;
    for (int i = 0; i < count; ++i)	// change volume
    {
        raw[raw_count] = raw_in[i];
        raw_count++;
    }

    if (s_start_stop_rtd_timer && !s_rtd_timer_is_active)//timer_rtd_->isActive()
    {
        s_rtd_timer_is_active = true;
        timer_rtd_->start(TIMER_RTD_SLOW);
        //qDebug()<<"Timer START SetRaw()"<<disp_ident;
    }

    //RtdDecode();
    //c+=count;
    if (!one_emit_disp_data_dec65 && raw_count>count_disp_data_dec65)//1.49 DISP_SAMPLE_RATE*52.0 52s period
    {
        one_emit_disp_data_dec65 = true;
        emit EmitIsDispHaveDataForDec65(disp_ident,true);
        //qDebug()<<"SetRaw count TRUE================="<<raw_count<<disp_ident;
    }
    if (one_emit_disp_data_dec65 && raw_count<count_disp_data_dec65)
    {
        one_emit_disp_data_dec65 = false;
        emit EmitIsDispHaveDataForDec65(disp_ident,false);
        //qDebug()<<"SetRaw count FALSE================="<<raw_count<<disp_ident;
    }
    //qDebug()<<"SetRaw count================="<<raw_count<<disp_ident;
}
/*void DisplayMs::SetSampleRate(int)
{
    sampe_rate = val;
}*/
void DisplayMs::SetDisplayRefr(int val)
{
    refresh_time = val;
}

QColor DisplayMs::getColor(int val)
{
    QColor color = QColor(0,0,0);
    //val = ((double)val*1.5-130.0) - diplay_ofset;
    //val = val - diplay_ofset;  //diplay_contr

    //1.55= inportent reset position  no correction (offset and contrast) if no signal
    //1.55= other case get some color from palette and draw to display
    if (val>0)
        val = ((double)val*(diplay_contr+1.0)-(255.0*diplay_contr)) - diplay_ofset;//255 225

    if (val >= rgb_c.rgb_max)
    {
        color = QColor(255,255,255);
    }
    else if (val <= 0)
    {
        color = QColor(0,0,0);
    }
    else
        color = palette_tmp.pixel(rgb_c.rgb_img_width_center,(rgb_c.rgb_max-val));
    //if()
    //qDebug()<<(rgb_c.rgb_max-val);
    /*if(val>mx)
    	mx=val;
    if(val<mi)
    	mi=val;*/

    return color;
}

void DisplayMs::SetCustomPalette(QPixmap pic1,QColor w_c,QColor wl_c)
{
    temp_pal_custom = pic1;
    s_custom_wave = w_c;
    s_custom_pen_wave = wl_c;

    if (saved_color == 8)
    {
        SetPaletteParm(pic1);
        s_pen_wave = QPen(s_custom_pen_wave,0);// ,0 need qt5 v1.27 QPen(QColor(xxx,xxx,xxx),0,Qt::SolidLine) Qt::SolidLine=1
        s_brush_wave = s_custom_wave; //QColor(0, 45, 122, 255);
        if (f_disp_v_h)
            PaintData_VD(true);
        else
            PaintData_HD(true);
    }
    //qDebug()<<"str"<<pic.height()<<saved_color;
}

void DisplayMs::SetPaletteParm(QPixmap pic)
{
    palette_tmp = QImage(pic.width(), pic.height(), QImage::Format_RGB32);
    palette_tmp = pic.toImage();
    rgb_c.rgb_img_width_center =  palette_tmp.width()/2;
    rgb_c.rgb_max = palette_tmp.height();
}
void DisplayMs::SetPalette(int color)
{
    if (saved_color == color)
        return;
    saved_color = color;

    switch (saved_color)
    {
    case 0:
        SetPaletteParm(temp_pal0);
        s_pen_wave = QPen(QColor(255,255,255),0);// bw
        s_brush_wave = QColor(0, 0, 0, 0);// bw
        break;
    case 1:
        SetPaletteParm(temp_pal1);
        s_pen_wave = QPen(QColor(255,255,255),0);// default
        s_brush_wave = QColor(0, 45, 122, 255);   // default
        break;
    case 2:
        SetPaletteParm(temp_pal2);
        s_pen_wave = QPen(QColor(255,255,255),0);// 1
        s_brush_wave = QColor(0, 0, 250, 255);
        break;
    case 3:
        SetPaletteParm(temp_pal3);
        s_pen_wave = QPen(QColor(255,255,255),0);// 2
        s_brush_wave = QColor(0, 75, 0, 255);
        break;
    case 4:
        SetPaletteParm(temp_pal4);
        s_pen_wave = QPen(QColor(255,255,255),0);// 3
        s_brush_wave = QColor(0, 70, 0, 255);
        break;
    case 5:
        SetPaletteParm(temp_pal5);
        s_pen_wave = QPen(QColor(255,255,255),0);// 4
        s_brush_wave = QColor(0, 45, 122, 255);
        break;
    case 6:
        SetPaletteParm(temp_pal6);
        s_pen_wave = QPen(QColor(0,255,0),0);    // 5
        s_brush_wave = QColor(0, 0, 0, 0);
        break;
    case 7:
        SetPaletteParm(temp_pal7);
        s_pen_wave = QPen(QColor(255,255,255),0);// 6
        s_brush_wave = QColor(0, 65, 100, 255);
        break;
    case 8:
        SetPaletteParm(temp_pal_custom);
        s_pen_wave = QPen(s_custom_pen_wave,0);// custom
        s_brush_wave = s_custom_wave;
        break;
    default:
        SetPaletteParm(temp_pal1);
        break;
    }
    if (f_disp_v_h)
        PaintData_VD(true);
    else
        PaintData_HD(true);
}

void DisplayMs::ReciveData(QList<int*> arr, QPoint *points, int raw_c, int *raw_data,QString t_min,QString ymd,bool f_decode,QLine *mous_lines,
                           int*rtd_dpx,QString*rtd_t,int*rtd_fmt,int rtd_dc,int* rtd_adp,int rtd_adc,bool ffopen,
                           QLine *linesrtd,int posrtd,int updposrtd,bool endrtd)
{
    int j = 0;
    s_time = t_min;
    s_ymd = ymd;
    if (t_min == "NONETM") f_disp_time = false;//2.43
    else f_disp_time = true;
    s_fopen = ffopen;
    //qDebug()<<"ReciveData"<<disp_ident<<s_time<<s_fopen<<f_disp_time;

    for (int i = 0; i<COUNT_MOUSE_LINES; i++)// copy from other display mouse lines
    {
        lines_mouse[i] = mous_lines[i];
        lines_rtd[i] = linesrtd[i];
    }

    ///////////RTD////////////////////////
    dec_labels_posx_count = rtd_dc;
    for (int i = 0; i<dec_labels_posx_count; i++)
    {
        dec_labels_posx[i] = rtd_dpx[i];
        dec_labels_text[i] = rtd_t[i];
        fm_dec_labels_txt[i] = rtd_fmt[i];
    }
    dec_lines_posx_count = rtd_adc;
    for (int i = 0; i<dec_lines_posx_count; i++)
        dec_lines_posx[i] = rtd_adp[i];

    f_dec_pings_draw = true;
    //////////END RTD////////////////////////

    emit EmitFileNameChenged();

    for (j = 0; j < DATA_WIDTH; j++)
    {
        for (int i = 0; i < DATA_DSP_HEIGHT; i++)
        {
            data_graph_all[j][i] = arr.at(j)[i];
            PaintImageWaterfall(j,i);
        }
    }

    x_pos = DATA_WIDTH;

    for (j = 0; j < DATA_WIDTH+2; j++)
        wave_points[j] = points[j];

    SeavPrevRaw();//for second display
    for (j = 0; j < raw_c; j++)
        raw[j] = raw_data[j];

    raw_count = raw_c;
    //qDebug()<<"ReciveData count================="<<raw_count<<disp_ident;

    DecodeAllData(f_decode,false,false);//false=internal data SFox and FT8 one decode in double click

    //qDebug()<<"endddddddddddddddd="<<endrtd;
    if (s_start_stop_rtd_timer && !s_fopen && raw_count!=0 && !endrtd)//raw_count!=0
    {
        emit EmitRriorityDisp(true);
        //s_pos_rtd_end = 0; // inportant
        end_rtd = endrtd;
        s_upd_pos_lines_rtd = updposrtd;
        pos_rtd = posrtd;
        s_rtd_timer_is_active = true;
        timer_rtd_->start(TIMER_RTD_SLOW);
        //qDebug()<<"Timer START ReciveData()"<<disp_ident;
    }

    if (raw_count>count_disp_data_dec65)//1.49 DISP_SAMPLE_RATE*52.0 52s period
        emit EmitIsDispHaveDataForDec65(disp_ident,true);
    else
        emit EmitIsDispHaveDataForDec65(disp_ident,false);


    update();
    //PaintData(true);
}
void DisplayMs::ClarHDisplay()
{
    for (int i = 0; i<COUNT_MOUSE_LINES; i++)// clear mouse lines
    {
        lines_mouse[i] = QLine(0, x_pos_timeline, 0, height());
        lines_rtd[i] = QLine(0, x_pos_timeline, 0, height());
    }

    for (int j = 0; j < DATA_WIDTH; j++)
    {
        for (int i = 0; i < DATA_DSP_HEIGHT; i++)
        {
            img_tmp.setPixel(j,i,0);//0xffffff cialo iztrivane moze ida ne e -offset_wat_sm and i=offset_wat_sm
            data_graph_all[j][i] = 0;
        }

        data_graph_smiter[j] = 0;
        if (j==0)
        {
            wave_points[j] = QPoint(j, height());
            wave_points[j+1] = QPoint(j,DATA_HEIGHT-3);
        }
        else
        {
            wave_points[j+1] = QPoint(j,DATA_HEIGHT-3);
            if (j == DATA_WIDTH-1)
                wave_points[DATA_WIDTH+1] = QPoint(DATA_WIDTH, height());
        }
    }
    dec_labels_posx_count = 0;//1.55=
    dec_lines_posx_count = 0; //1.55=
}
void DisplayMs::ClarDisplay()
{
    //img_tmp = QImage(GRAPHIC_WIDTH, GRAPHIC_HEIGHT, QImage::Format_RGB32)
    /*for (int i = 0; i<COUNT_MOUSE_LINES; i++)// clear mouse lines
    {
        lines_mouse[i] = QLine(0, x_pos_timeline, 0, height());
        lines_rtd[i] = QLine(0, x_pos_timeline, 0, height());
    }

    for (int j = 0; j < DATA_WIDTH; j++)
    {
        for (int i = 0; i < DATA_DSP_HEIGHT ; i++)
        {
            img_tmp.setPixel(j,i,0);// cialo iztrivane moze ida ne e -offset_wat_sm and i=offset_wat_sm
            data_graph_all[j][i] = 0;
        }

        data_graph_smiter[j] = 0;
        if (j==0)
        {
            wave_points[j] = QPoint(j, height());
            wave_points[j+1] = QPoint(j,DATA_HEIGHT-3);
        }
        else
        {
            wave_points[j+1] = QPoint(j,DATA_HEIGHT-3);
            if (j == DATA_WIDTH-1)
                wave_points[DATA_WIDTH+1] = QPoint(DATA_WIDTH, height());
        }

    }*/
    if (!f_disp_v_h)
        ClarHDisplay();

    SeavPrevRaw();//for main first display
    for (int i = 0; i < RAW_BUFFER_SIZE; i++)
        raw[i]=0;

    x_pos = 0;
    s_upd_pos = 0; //printf("ClearDisp=%d Count=%d\n",disp_ident,raw_count);

    raw_count = 0;

    pos_rtd = 0;    
    s_upd_pos_lines_rtd = 0;

    f_dec_pings_draw = false;

    //for (int i = 0; i < rtd_count_dupes; i++)
    //rtd_dupe_posx[i] = -1;
    dec_labels_posx_count = 0;

    //for (int i = 0; i < rtd_all_dec_count; i++)
    //rtd_all_dec_pos[i] = -1;
    dec_lines_posx_count = 0;

    //qDebug()<<"Clear1";
    update();
}
void DisplayMs::SetVDdf(int df, int in_mode)
{
    if (!f_mouse_pres)//2.41 protection
    {
        f_r00 = false;
        f_r01 = false;
    }
    s_prev_vd_rx_df = df;//inportent reset
    if (s_vd_df == df) return; //no refresh if df is a same  qDebug()<<"df"<<df;
    s_vd_df = df;
    if (in_mode == s_mode)
    {//2.41 protection from mode changed (in_mode!=s_mode), refresh ony DF
        SetVDRxFreqF0F1();
        setVDispFreqScale(s_start,s_stop);
        CorrDfW(s_vd_df);//2.53 (frq01-frq00)/2
    }
    //else qDebug()<<"ERROR MODE CHANGED"<<in_mode<<s_mode;
    //SaveFT8RxTxFreq(); //2.41 no needed
}
void DisplayMs::SetZeroDfVScale(bool f)
{
    f_zero_freq_scale = f;
    setVDispFreqScale(s_start,s_stop);
}
void DisplayMs::RefreshLimits()//2.53
{
    if (allq65)
    {
        float h = 1;
        if (s_mode==15) h = 2;
        if (s_mode==16) h = 4;
        if (s_mode==17) h = 8;//int h=int(pow(2.0,SubMode));
        int nsps=1800;
        if (period_time_sec==30) nsps=3600;
        if (period_time_sec==60) nsps=7200;
        if (period_time_sec==120) nsps=16000;
        if (period_time_sec==300) nsps=41472;
        int LL=64*(2+h);
        float df=12000.0/(float)nsps;
        frq00_limit	= (64*df)+1.0;
        frq01_limit	= (double)(5000 - (LL-64)*df)-1.0;
        bwQ65=65.0*h*df;
    }
    else
    {
        frq00_limit	= 20;
        frq01_limit	= 4980;
    }	//if (disp_ident==0) qDebug()<<"DOWN L="<<frq00_limit-1.0<<"DOWN U="<<frq01_limit+1.0<<bwQ65;
    SetVDTxFreq();
}
void DisplayMs::SetVDRxFreqF0F1()//2.24 ft8/4 df
{
    if (vd_rx_freq<frq00_limit+dflimit) vd_rx_freq=frq00_limit+dflimit;//2.53
    if (vd_rx_freq>frq01_limit-dflimit) vd_rx_freq=frq01_limit-dflimit;//2.53

    if (!f_r00 && !f_r01)
    {
        if (s_mode==11 || s_mode==13 || s_mode==18 || allq65)//ft8 ft4
        {
            double prev_df = (frq01 - frq00)/2.0;
            double div_df = s_vd_df - prev_df;
            double t_frq01 = frq01 + div_df;
            double t_frq00 = frq00 - div_df;
            /*if (vd_rx_freq+dflimit>t_frq01)
            {
                frq01=vd_rx_freq+dflimit;
                frq00=frq01-s_vd_df*2.0;
            }
            else if (vd_rx_freq-dflimit<t_frq00)            
            {
                frq00=vd_rx_freq-dflimit;
                frq01=frq00+s_vd_df*2.0;
            }*/
            if (vd_rx_freq-dflimit<t_frq00) //2.53 first down boundary
            {
                frq00=vd_rx_freq-dflimit;
                frq01=frq00+s_vd_df*2.0;
            }
            else if (vd_rx_freq+dflimit>t_frq01)
            {
                frq01=vd_rx_freq+dflimit;
                frq00=frq01-s_vd_df*2.0;
            }
            else
            {
                frq01 = t_frq01;
                frq00 = t_frq00;
            }
        }
        else
        {
            frq00 = vd_rx_freq - s_vd_df;
            frq01 = vd_rx_freq + s_vd_df;
        }
    }
    //if (disp_ident==0) qDebug()<<"DF555============"<<frq00<<frq01;

    if (frq00>vd_rx_freq-dflimit) frq00=vd_rx_freq-dflimit;
    if (frq01<vd_rx_freq+dflimit) frq01=vd_rx_freq+dflimit;
    if (frq00<frq00_limit) frq00=frq00_limit;//2.53
    if (frq01>frq01_limit) frq01=frq01_limit;//2.53

    s_vd_df = (frq01-frq00)/2.0;
    //qDebug()<<"DREAL="<<(int)frq00<<"RX="<<(int)vd_rx_freq<<"STOP="<<(int)frq01<<"DF="<<(int)s_vd_df;

    double f00 = frq00;
    double f01 = frq01;
    if (frq00<40) f00=40;
    if (frq01>4960) f01=4960;
    //if (disp_ident==0) qDebug()<<frq00_limit<<frq01_limit<<"START="<<(int)f00<<"RX="<<(int)vd_rx_freq<<"STOP="<<(int)f01;
    emit EmitVDRxFreqF0F1(vd_rx_freq,f00,f01);
    //if (disp_ident==0) qDebug()<<"DF="<<s_mode<<vd_rx_freq<<s_vd_df<<f00<<f01;
}
void DisplayMs::SetVDTxFreq()//2.53
{
    if (vd_tx_freq<frq00_limit+dflimit) vd_tx_freq=frq00_limit+dflimit;
    if (vd_tx_freq>frq01_limit-dflimit) vd_tx_freq=frq01_limit-dflimit;
	//if (disp_ident==0) qDebug()<<frq00_limit<<frq01_limit<<"TX========="<<(int)vd_tx_freq;
    emit EmitVDTxFreq(vd_tx_freq);
}
void DisplayMs::setVDispFreqScale(int start,int stop)//bool <- change start stop and df
{
    s_start=start;
    s_stop=stop;
    int line_height_pix = 4; //4pix
    int g_width = this->width();

    trans_fac = ((double)g_width/((double)DATA_WIDTH));//1.39 rem DATA_WIDTH/2

    int band = stop-start;

    int k_big=4;
    //int k_start=100;
    if (band<=3000)
    {
        band = band/20;  //prez 20hz
        k_big = 4;
    }
    else
    {
        band = band/25;   //prez 25hz
        k_big = 3;
    }

    bool f_even_odd = false;
    if (stop-start>=2600) f_even_odd = true;

    double scale_x_inc_pix = ((double)g_width/(double)band); //
    count_lines_scale_x = (g_width / scale_x_inc_pix)+3+5;//
    //qDebug()<<"count_lines_scale_x"<<count_lines_scale_x;
    //double save_x_inc = (30.0-(double)s_start)/scale_x_inc_pix;//+scale_x_inc_pix*3.0;//0.0;

    double save_x_inc = 0.0;
    if (f_zero_freq_scale && s_mode!=10 && s_mode!=11 && s_mode!=13 && s_mode!=18 && !allq65)//no f_zero_freq_scale in pi4 and ft8 and ft4
        save_x_inc = (double)((-30.0)*(double)g_width)/(double)(s_stop-s_start)-scale_x_inc_pix*(double)(k_big+1);

    int big_line = 0;

    pos_text_hor_Y = x_pos_freq_line-(line_height_pix*2)-4;//-fm_X->height();//-5  2<-pix otstoianieto na cifrite ot 4ertite
    //qDebug()<<fm_X->height()<<pos_text_hor_Y<<fm_X->xHeight();
    //if(fm_X->height()-3>pos_text_hor_Y)
    //pos_text_hor_Y = fm_X->height()-4;

    for (int i = 0; i<MAX_LINES_SCALE_X; i++) freq_scale_x[i]="";
    for (int i = 0; i<count_lines_scale_x; i++) lines_scale_x[i]=QLine(0,0,0,0);

    bool f_even = true;
    //double t_x_inc;
    for (int i = 0; i<count_lines_scale_x; i++)
    {
        //t_x_inc = save_x_inc;
        //if(save_x_inc < 0.0)
        //t_x_inc = -20.0;
        //if(save_x_inc > g_width)
        //t_x_inc = g_width+20;

        if (big_line==0)
        {

            int start0 = start;

            if (f_zero_freq_scale && s_mode!=10 && s_mode!=11 && s_mode!=13 && s_mode!=18 && !allq65)//no f_zero_freq_scale in pi4 and ft8 and ft4
            {
                //if (s_mode==10)
                //start0 = start-770;
                //else
                start0 = start-1400;
            }

            if (save_x_inc>0.0 && save_x_inc<g_width)
                lines_scale_x[i] = QLine(0+save_x_inc,x_pos_freq_line-(line_height_pix*2)-3,0+save_x_inc,x_pos_freq_line-3);

            if (f_even_odd)
            {
                if (f_even)
                {
                    if (save_x_inc>=0.0 && save_x_inc<g_width-1)
                        //if (i!=0 && (save_x_inc < (g_width-fm_X->width(QString("%1").arg(start0))/2)))//parvata i posledna ne
                    {
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)//2.56
                        pos_text_hor_X[i] = 0+save_x_inc-fm_X->horizontalAdvance(QString("%1").arg(start0))/2;
#else
                        pos_text_hor_X[i] = 0+save_x_inc-fm_X->width(QString("%1").arg(start0))/2;
#endif
                        freq_scale_x[i] = QString("%1").arg(start0);
                    }

                    f_even = false;
                }
                else
                    f_even = true;
            }
            else
            {
                if (save_x_inc>=0.0 && save_x_inc<g_width-1)
                    //if (i!=0 && (save_x_inc < (g_width-fm_X->width(QString("%1").arg(start0))/2)))//parvata i posledna ne
                {
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)//2.56
                    pos_text_hor_X[i] = 0+save_x_inc-fm_X->horizontalAdvance(QString("%1").arg(start0))/2;
#else
                    pos_text_hor_X[i] = 0+save_x_inc-fm_X->width(QString("%1").arg(start0))/2;
#endif
                    freq_scale_x[i] = QString("%1").arg(start0);//-1270
                }
            }
            start+=100;
        }
        else if (save_x_inc>0.0 && save_x_inc<g_width)
            lines_scale_x[i] = QLine(0+save_x_inc,x_pos_freq_line-line_height_pix-3,0+save_x_inc,x_pos_freq_line-3);

        if (big_line==k_big)
            big_line=0;
        else
            big_line++;

        save_x_inc += scale_x_inc_pix;
    }

    double bw=65.0*11025.0/4096.0; 
    double bwt=bw;   
    int xpos_frq1,xpos_frq2,xpos_frq3,xpos_frq4,xpos_frq5,xpos_frq6;

    if (s_mode==8) bw=2.0*bw;
    else if (s_mode==9) bw=4.0*bw;
    
    if (s_mode==10)
    {
        bw=3.0*12000.0/2048.0; // bw=3*11025.0/2520.0;
        bw=40.0*bw;
    }
    if (s_mode==11)
    {
        bw=7.0*12000.0/1920.0;     //FT8 = 43.75
        bwt=bw;
        if (id_mshf==1) bw=127.0*12000.0/1024.0; 
        if (id_mshf==2) bwt=127.0*12000.0/1024.0;
    }
    if (s_mode==13)
    {
        //bw=3.0*12000.0/512.0;     //FT4 =70.3125
        bw=3.0*12000.0/576.0;       //FT4 =62.5
        bwt=bw;
    }
    if (s_mode==18)
    {
        bw=3.0*12000.0/288.0;      //FT2 =125.0
        bwt=bw;
    }
    if (allq65)//2.53
    {
        /*float h = 1;
        if (s_mode==15) h = 2;
        if (s_mode==16) h = 4;
        if (s_mode==17) h = 8;//int h=int(pow(2.0,SubMode));
        int nsps=1800;
        if(period_time_sec==30) nsps=3600;
        if(period_time_sec==60) nsps=7200;
        if(period_time_sec==120) nsps=16000;
        if(period_time_sec==300) nsps=41472;
        float baud=12000.0/nsps;
        bw=65.0*h*baud; // qDebug()<<bw<<period_time_sec<<nsps<<s_mode<<h;*/
        bw=bwQ65;
        bwt=bw;
    }
	line_bw = (double)(bw*(double)g_width)/(double)(s_stop-s_start);//mouse lines

    if (s_mode==10)//pi4
    {
        xpos_frq1 = (double)(((vd_rx_freq-bw*1.0/6.0)-(double)s_start)*(double)g_width)/(double)(s_stop-s_start);
        xpos_frq2 = (double)((vd_rx_freq-(double)s_start)*(double)g_width)/(double)(s_stop-s_start);
        xpos_frq3 = (double)(((vd_rx_freq+bw*1.0/6.0)-(double)s_start)*(double)g_width)/(double)(s_stop-s_start);//(double)vd_rx_freq*scale_x_inc_pix;
        xpos_frq4 = (double)(((vd_rx_freq+bw*1.0/3.0)-(double)s_start)*(double)g_width)/(double)(s_stop-s_start);
        xpos_frq5 = (double)(((vd_rx_freq+bw*2.0/3.0)-(double)s_start)*(double)g_width)/(double)(s_stop-s_start);
        xpos_frq6 = (double)(((vd_rx_freq+bw)-(double)s_start)*(double)g_width)/(double)(s_stop-s_start);//(double)(vd_rx_freq+375.5)*scale_x_inc_pix;
    }
    else if (s_mode==11 || s_mode==13 || s_mode==18 || allq65)//ft8 ft4
    {
        xpos_frq1 = (double)((vd_rx_freq-(double)s_start)*(double)g_width)/(double)(s_stop-s_start);//rx
        xpos_frq2 = (double)((vd_tx_freq-(double)s_start)*(double)g_width)/(double)(s_stop-s_start)-2;//tx
        xpos_frq3 = (double)(((vd_tx_freq+bwt)-(double)s_start)*(double)g_width)/(double)(s_stop-s_start)+2;//tx
        xpos_frq4 = (double)((vd_rx_freq-(double)s_start)*(double)g_width)/(double)(s_stop-s_start);//rx
        xpos_frq5 = (double)((vd_rx_freq-(double)s_start)*(double)g_width)/(double)(s_stop-s_start);//rx
        xpos_frq6 = (double)(((vd_rx_freq+bw)-(double)s_start)*(double)g_width)/(double)(s_stop-s_start);//rx        
    }
    else//jt65
    {
        xpos_frq1 = (double)((vd_rx_freq-(double)s_start)*(double)g_width)/(double)(s_stop-s_start);
        xpos_frq2 = (double)(((vd_rx_freq+20.0*bw/65.0)-(double)s_start)*(double)g_width)/(double)(s_stop-s_start);//(double)vd_rx_freq*scale_x_inc_pix;
        xpos_frq3 = (double)(((vd_rx_freq+30.0*bw/65.0)-(double)s_start)*(double)g_width)/(double)(s_stop-s_start);
        xpos_frq4 = (double)(((vd_rx_freq+40.0*bw/65.0)-(double)s_start)*(double)g_width)/(double)(s_stop-s_start);
        xpos_frq5 = (double)(((vd_rx_freq+bw)-(double)s_start)*(double)g_width)/(double)(s_stop-s_start);//(double)(vd_rx_freq+375.5)*scale_x_inc_pix;
        xpos_frq6 = (double)(((vd_rx_freq+bw)-(double)s_start)*(double)g_width)/(double)(s_stop-s_start);
    }

    lines_vd_rx_freq[0] = xpos_frq1;
    lines_vd_rx_freq[1] = xpos_frq2;
    lines_vd_rx_freq[2] = xpos_frq3;
    lines_vd_rx_freq[3] = xpos_frq4;
    lines_vd_rx_freq[4] = xpos_frq5;
    lines_vd_rx_freq[5] = xpos_frq6;

    //qDebug()<<"FFFF="<<lines_vd_rx_freq[0]<<lines_vd_rx_freq[1]<<lines_vd_rx_freq[2]
    //<<lines_vd_rx_freq[3]<<lines_vd_rx_freq[4]<<lines_vd_rx_freq[5];

    if (s_mode==11 || s_mode==13 || s_mode==18 || allq65)//2.24 ft8/4 df
    {
        xpos_frq1 = (double)(((frq00)-(double)s_start)*(double)g_width)/(double)(s_stop-s_start);
        xpos_frq2 = (double)(((frq01)-(double)s_start)*(double)g_width)/(double)(s_stop-s_start);
        lines_vd_df[0]=xpos_frq1;
        lines_vd_df[1]=xpos_frq2;
    }
    else
    {
        xpos_frq1 = (double)(((vd_rx_freq-(double)s_vd_df)-(double)s_start)*(double)g_width)/(double)(s_stop-s_start);
        xpos_frq2 = (double)(((vd_rx_freq+(double)s_vd_df)-(double)s_start)*(double)g_width)/(double)(s_stop-s_start);
        lines_vd_df[0]=xpos_frq1;
        lines_vd_df[1]=xpos_frq2;
    }
    //qDebug()<<"count_lines_scale_x"<<scale_x_inc_pix<<xpos_frq1;

    update();
}
void DisplayMs::SetVDisplayHight(int i)
{
    if (i==1)
    {
        if (f_disp_v_h)
        {
            setFixedHeight(DATA_HEIGHT);//150pix - 19<-control widget
            PaintData_VD(true);
        }
        else
            setFixedHeight(DATA_HEIGHT);//150pix
    }
    if (i==2)
    {
        if (f_disp_v_h)
        {
            setFixedHeight(DATA_HEIGHT*2);//300pix - 19<-control widget
            PaintData_VD(true);
        }
        else
            setFixedHeight(DATA_HEIGHT*2);//300pix
    }
}
void DisplayMs::SetAdleDisplay_VD(bool f)
{
    f_adle_vd = f;
    PaintData_VD(true);
}
void DisplayMs::SetFlatDisplay_VD(bool f)
{
    f_flat_vd = f;
    PaintData_VD(true);
}
void DisplayMs::FlatDisplay_VD(int *k_segm, int row)
{
    int segm = 52;//15=52 26=30 30=26 39=20 52=15 78=10 156=5
    int pts = DATA_WIDTH/segm;
    int sum_a = 0;
    int c_pts = 0;
    int i_from = 0;
    int pos_bas = 0;// = 190;

    if (f_adle_vd) pos_bas = 180;
    else
    {
        for (int i = 0; i < DATA_WIDTH; ++i)// calc base level
            pos_bas += data_graph_all_v[row][i];
        pos_bas = pos_bas/DATA_WIDTH;
    }


    double k_segm0[DATA_WIDTH+8];
    double prev_sss = pos_bas - data_graph_all_v[row][0];
    for (int i = 0; i < DATA_WIDTH; ++i)// DATA_WIDTH=780
    {
        sum_a += data_graph_all_v[row][i];
        if (c_pts>=pts-1)
        {
            double sss = (double)pos_bas - ((double)sum_a/(double)(c_pts+1));
            double incr = (sss - prev_sss)/(double)(c_pts+1);
            double d0 = 0.0;
            double d1 = 0.0;
            for (int x = i_from; x < i_from+pts; ++x)
            {
                //if (x<1 || x>DATA_WIDTH-2) qDebug()<<x;
                int test1 = data_graph_all_v[row][x];
                int test2 = data_graph_all_v[row][x]+prev_sss;
                double ds = 0.0;
                if (test1>test2)
                    ds = 0.0;
                else
                    ds = prev_sss;

                if (x == i_from) 	   d0 = ds;
                if (x == i_from+pts-1) d1 = ds;

                prev_sss += incr;
            }
            double incc = (d1 - d0)/(double)(pts-1);
            k_segm0[i_from] = d0;
            k_segm0[i_from+pts-1]= d1;
            for (int x = i_from+1; x < i_from+pts-1; ++x)
            {
                d0 += incc;
                k_segm0[x] = d0;
            }
            c_pts = 0;
            sum_a = 0;
            i_from += pts;
        }
        else
            c_pts++;
    }
    double k_segm1[DATA_WIDTH+8];
    prev_sss = pos_bas - data_graph_all_v[row][DATA_WIDTH-1];
    i_from = DATA_WIDTH-1;
    for (int i = DATA_WIDTH-1; i >= 0; --i)// DATA_WIDTH=780
    {
        sum_a += data_graph_all_v[row][i];
        if (c_pts>=pts-1)
        {
            double sss = (double)pos_bas - ((double)sum_a/(double)(c_pts+1));
            double incr = (sss - prev_sss)/(double)(c_pts+1);
            double d0 = 0.0;
            double d1 = 0.0;
            for (int x = i_from; x >= i_from-pts+1; --x)
            {
                //if (x<1 || x>DATA_WIDTH-2) qDebug()<<x;
                int test1 = data_graph_all_v[row][x];
                int test2 = data_graph_all_v[row][x]+prev_sss;
                double ds = 0.0;
                if (test1>test2)
                    ds = 0.0;
                else
                    ds = prev_sss;

                if (x == i_from) 	   d0 = ds;
                if (x == i_from-pts+1) d1 = ds;

                prev_sss += incr;
            }
            double incc = (d1 - d0)/(double)(pts-1);
            k_segm1[i_from] = d0;
            k_segm1[i_from-pts+1]= d1;
            for (int x = i_from-1; x >= i_from-pts+2; --x)
            {
                d0 += incc;
                k_segm1[x] = d0;
            }
            c_pts = 0;
            sum_a = 0;
            i_from -= pts;
        }
        else
            c_pts++;
    }
    for (int i = 0; i < DATA_WIDTH; ++i)
    {
        k_segm[i]=(int)((k_segm0[i]+k_segm1[i])/2.0);
        //k_segm[i]=(int)(k_segm0[i]);
        //k_segm[i]=(int)(k_segm1[i]);
    }

    /*double k_segm0[DATA_WIDTH+5];
    double prev_sss = pos_bas - data_graph_all_v[row][0];
       for (int i = 0; i < DATA_WIDTH; ++i)//230 DATA_WIDTH=780
       {
           sum_a += data_graph_all_v[row][i];
           if (c_pts>=pts-1)
           {
               double sss = (double)pos_bas - ((double)sum_a/(double)(c_pts+1));
               double incr = (sss - prev_sss)/(double)(c_pts+1);
               for (int x = i_from; x < i_from+pts; ++x)
               {
               	//if (x<1 || x>DATA_WIDTH-2) qDebug()<<x;
                   int test1 = data_graph_all_v[row][x];
                   int test2 = data_graph_all_v[row][x]+prev_sss; 
                   if (test1>test2)
                       k_segm0[x] = 0.0;
                   else
                       k_segm0[x] = prev_sss; 
     
                   prev_sss += incr;
               } 
               c_pts = 0;
               sum_a = 0;
               i_from += pts; 
           }
           else
               c_pts++; 
       } 
       double k_segm1[DATA_WIDTH+5];
       prev_sss = pos_bas - data_graph_all_v[row][DATA_WIDTH-1];
       i_from = DATA_WIDTH-1;
       for (int i = DATA_WIDTH-1; i >= 0; --i)//230 DATA_WIDTH=780
       {
           sum_a += data_graph_all_v[row][i];
           if (c_pts>=pts-1)
           {
               double sss = (double)pos_bas - ((double)sum_a/(double)(c_pts+1));
               double incr = (sss - prev_sss)/(double)(c_pts+1);
               for (int x = i_from; x >= i_from-pts+1; --x)
               {
               	//if (x<1 || x>DATA_WIDTH-2) qDebug()<<x;
                   int test1 = data_graph_all_v[row][x];
                   int test2 = data_graph_all_v[row][x]+prev_sss; 
                   if (test1>test2)
                       k_segm1[x] = 0.0;
                   else
                       k_segm1[x] = prev_sss; 
     
                   prev_sss += incr; 
               } 
               c_pts = 0;
               sum_a = 0;
               i_from -= pts; 
           }
           else
               c_pts++; 
       }
       for (int i = 0; i < DATA_WIDTH; ++i)
       {
       	k_segm[i]=(int)((k_segm0[i]+k_segm1[i])/2.0);
      	}*/
}
void DisplayMs::PaintData_VD(bool paint_all)
{
    if (paint_all)
    {
        int dh = var_vdisp_height;// ina4e ne smqta ????
        for (int j = 0; j < var_vdisp_height; ++j)
        {
            int p_draw_dss = j+pos_y_draw_vimgs+1;

            if (f_flat_vd || f_adle_vd)//1.51
            {
                int k_segm[DATA_WIDTH+10];//+k_segm[i]
                FlatDisplay_VD(k_segm,j);
                for (int i = 0; i < DATA_WIDTH; ++i)//1.39 rem DATA_WIDTH/2
                {
                    if (f_img_v0_v1)
                    {
                        if (p_draw_dss<var_vdisp_height)
                            img_tmp_v0.setPixel(i,p_draw_dss,getColor(data_graph_all_v[j][i]+k_segm[i]).rgb());
                        else
                            img_tmp_v1.setPixel(i,p_draw_dss-dh,getColor(data_graph_all_v[j][i]+k_segm[i]).rgb());
                    }
                    else
                    {
                        if (p_draw_dss<var_vdisp_height)
                            img_tmp_v1.setPixel(i,p_draw_dss,getColor(data_graph_all_v[j][i]+k_segm[i]).rgb());
                        else
                            img_tmp_v0.setPixel(i,p_draw_dss-dh,getColor(data_graph_all_v[j][i]+k_segm[i]).rgb());
                    }
                }
            }
            else
            {
                for (int i = 0; i < DATA_WIDTH; ++i)//1.39 rem DATA_WIDTH/2
                {
                    if (f_img_v0_v1)
                    {
                        if (p_draw_dss<var_vdisp_height)
                            img_tmp_v0.setPixel(i,p_draw_dss,getColor(data_graph_all_v[j][i]).rgb());
                        else
                            img_tmp_v1.setPixel(i,p_draw_dss-dh,getColor(data_graph_all_v[j][i]).rgb());
                    }
                    else
                    {
                        if (p_draw_dss<var_vdisp_height)
                            img_tmp_v1.setPixel(i,p_draw_dss,getColor(data_graph_all_v[j][i]).rgb());
                        else
                            img_tmp_v0.setPixel(i,p_draw_dss-dh,getColor(data_graph_all_v[j][i]).rgb());
                    }
                }
            }
        }
    }
    else
    {
        if (lines_vd_t_new && s_time!="NONETM")
        {
            //qDebug()<<"lines_vd_t_new";
            lines_vd_t_new = false;
            for (int j = 40-2; j >= 0; --j)//1.55=40 for new count speads max 40 lines old=20
            {
                posy_vd_lines_t[j+1]=posy_vd_lines_t[j];
                lines_vd_t_text[j+1]=lines_vd_t_text[j];
            }

            /*if (s_mode==11 || s_mode==13)// ft8 ft4
                lines_vd_t_text[0]=s_time.mid(0,2)+":"+s_time.mid(2,2)+":"+s_time.mid(4,2);
            else
                lines_vd_t_text[0]=s_time.mid(0,2)+":"+s_time.mid(2,2);*/
            if (period_time_sec < 60) lines_vd_t_text[0]=s_time.mid(0,2)+":"+s_time.mid(2,2)+":"+s_time.mid(4,2);
            else lines_vd_t_text[0]=s_time.mid(0,2)+":"+s_time.mid(2,2);

            posy_vd_lines_t[0] = x_pos_freq_line;

            if (lines_vd_t_count>0)//min 2 lines
            {
                //qDebug()<<posy_vd_lines_t[1]-posy_vd_lines_t[0]<<fm_X->height();
                if ((posy_vd_lines_t[1]-posy_vd_lines_t[0]) < fm_X->height()-2)
                    lines_vd_t_text[1] = ""; // remove text if no inaf space
            }

            if (lines_vd_t_count<40)//1.55=40 for new count speads max 40 lines old=20
                lines_vd_t_count++;

            if (vd_mouse_reset0<4)
                vd_mouse_reset0++;
            if (vd_mouse_reset1<4)
                vd_mouse_reset1++;

            //for(int i = 0; i < lines_vd_t_count; i++)
            //qDebug()<<posy_vd_lines_t[i]<<lines_vd_t_count;
        }
        for (int i = 0; i < lines_vd_t_count; ++i)
        {
            posy_vd_lines_t[i] = posy_vd_lines_t[i]+1;
        }


        if (posy_vd_mouse0[0]>0)
        {
            posy_vd_mouse0[0] = posy_vd_mouse0[0]+1;
            posy_vd_mouse1[0] = posy_vd_mouse1[0]+1;

            if (vd_mouse_reset0==1 && vd_mouse_reset1==10 )//inportent need for two mouse lines 1.36
            {
                posx_vd_mouse_df[2] = posx_vd_mouse_df[0];
                posx_vd_mouse_df[3] = posx_vd_mouse_df[1];
                posy_vd_mouse0[1] = posy_vd_mouse0[0];
                posy_vd_mouse1[1] = posy_vd_mouse1[0];
                //qDebug()<<"111 DELL="<<vd_mouse_reset1;
                vd_mouse_reset1 = 1;
            }
            if (vd_mouse_reset0 >= 2)
            {
                //qDebug()<<"DELL 0";
                posx_vd_mouse_df[0] = 0;
                posx_vd_mouse_df[1] = 0;
                posy_vd_mouse0[0]=0;
                posy_vd_mouse1[0]=0;
                vd_mouse_reset0 = 10;
            }
        }
        if (posy_vd_mouse0[1]>0)
        {
            posy_vd_mouse0[1] = posy_vd_mouse0[1]+1;
            posy_vd_mouse1[1] = posy_vd_mouse1[1]+1;

            if (vd_mouse_reset1 >=2)
            {
                //qDebug()<<"DELL 888"<<vd_mouse_reset0<<vd_mouse_reset1;
                /*if (vd_mouse_reset0 == 1)
                {
                    posx_vd_mouse_df[2] = posx_vd_mouse_df[0];
                    posx_vd_mouse_df[3] = posx_vd_mouse_df[1];
                    posy_vd_mouse0[1] = posy_vd_mouse0[0];
                    posy_vd_mouse1[1] = posy_vd_mouse1[0];
                    qDebug()<<"DELL="<<vd_mouse_reset1;
                    vd_mouse_reset1 = 1;
                }
                else*/
                //{
                posx_vd_mouse_df[2] = 0;
                posx_vd_mouse_df[3] = 0;
                posy_vd_mouse0[1]=0;
                posy_vd_mouse1[1]=0;
                vd_mouse_reset1 = 10;
                //}
            }
        }

        if (f_flat_vd || f_adle_vd)//1.51
        {
            int k_segm[DATA_WIDTH+10];//+k_segm[i]
            FlatDisplay_VD(k_segm,0);
            for (int i = 0; i < DATA_WIDTH; ++i)//1.39 rem DATA_WIDTH/2 DATA_WIDTH=780
            {
                if (f_img_v0_v1)
                    img_tmp_v0.setPixel(i,pos_y_draw_vimgs,getColor(data_graph_all_v[0][i]+k_segm[i]).rgb());
                else
                    img_tmp_v1.setPixel(i,pos_y_draw_vimgs,getColor(data_graph_all_v[0][i]+k_segm[i]).rgb());
            }
        }
        else
        {
            for (int i = 0; i < DATA_WIDTH; ++i)//1.39 rem DATA_WIDTH/2 DATA_WIDTH=780
            {
                if (f_img_v0_v1)
                    img_tmp_v0.setPixel(i,pos_y_draw_vimgs,getColor(data_graph_all_v[0][i]).rgb());
                else
                    img_tmp_v1.setPixel(i,pos_y_draw_vimgs,getColor(data_graph_all_v[0][i]).rgb());
            }
        }

        pos_y_v0++;
        pos_y_v1++;

        if (pos_y_v0>(var_vdisp_height-1))
            pos_y_v0=-(var_vdisp_height);
        if (pos_y_v1>(var_vdisp_height-1))
            pos_y_v1=-(var_vdisp_height);

        pos_y_draw_vimgs--;
        if (pos_y_draw_vimgs<0)
        {
            pos_y_draw_vimgs=var_vdisp_height-1;

            if (f_img_v0_v1)
                f_img_v0_v1=false;
            else
                f_img_v0_v1=true;
        }
    }

    update();
    //qDebug()<<"pos_y_h0="<<mx<<mi;//<<pos_y_h1<<"pos_y_draw_imgs="<<pos_y_draw_imgs;
}
void DisplayMs::SetValue(double *data_w, int smiter)
{
    if (f_disp_v_h)//jt65
    {
        for (int j = var_vdisp_height-2; j >= 0; --j)
        {
            for (int i = 0; i < DATA_WIDTH; i++)//1.39 rem DATA_WIDTH/2
                data_graph_all_v[j+1][i]=data_graph_all_v[j][i];
        }

        //double k_segm[DATA_WIDTH];//*k_segm[k]
        //FlatDisplay_VDd(data_w, k_segm);

        int k = (DATA_WIDTH)-1;//1.39 rem DATA_WIDTH/2
        for (int i = 0; i < DATA_WIDTH; ++i)////1.39 rem DATA_WIDTH/2   for (int i = 0; i < DATA_WIDTH/2; i++)
        {
            data_graph_all_v[0][i] = (int)(((data_w[k]+(double)rgb_c.rgb_max)*1.8)-30.0);// 2.6)-130); 2.0)-42);
            //data_graph_all_v[0][i] = (int)(((data_w[k]+(double)rgb_c.rgb_max)*2.5)-120.0);

            if (k>0)
                k--;
        }
        PaintData_VD(false);
    }
    else
    {
        for (int i = 0; i < DATA_DSP_HEIGHT; ++i)
        {
            data_graph_all[x_pos][i] = (int)(((data_w[i]+(double)rgb_c.rgb_max)*1.8)-30.0);//*1.8 naduva waterful kartinata -30 e mqstoto nagore andolu
            //data_graph_all[x_pos][i] = (int)(((data_w[i]+(double)rgb_c.rgb_max)*1.3)+45.0);
        }

        data_graph_smiter[x_pos] = ((smiter*1.7)-45);//*1.7 naduva kartinata na smetara -45 e mqstoto nagore andolu

        if (count_refr_disp>=refresh_time)
        {
            count_refr_disp=0;
            PaintData_HD(false);
        }
        else
            count_refr_disp++;

        if (x_pos < DATA_WIDTH)// HV opasno ne = vnimanie samo <
            x_pos++;
    }
}
void DisplayMs::PaintImageWaterfall(int j, int i)
{
    // if (i>offset_wat_sm)//data_graph_all[j][i]
    img_tmp.setPixel(j,i,getColor(data_graph_all[j][i]).rgb());
    //img_tmp.setPixel(j,i,getColor(data_graph_all[j][i]).rgb());
}
void DisplayMs::SetWavePoints(int j)
{
    int temp_w = data_graph_smiter[j];
    if (temp_w < offset_down_sm)
        temp_w = offset_down_sm;

    if (j==0)
    {
        wave_points[j] = QPoint(j, height());
        wave_points[j+1] = QPoint(j,DATA_HEIGHT+offset_down_sm-temp_w-3);
    }
    else
    {
        wave_points[j+1] = QPoint(j,DATA_HEIGHT+offset_down_sm-temp_w-3);
        if (j != DATA_WIDTH-1)
            wave_points[j+2] = QPoint(j, height()-3);
    }
}
void DisplayMs::PaintData_HD(bool paint_all)
{
    int upd_pos_begin = 0;
    int upd_pos_end = 0;

    if (paint_all)
    {//paint_all_Xsize -> pri reset i smiana na cviat updeitva do tam dokadeto ima data bez noise da se vizda

        //for (int j = 0; j < DATA_WIDTH; j++)//1.55=
        for (int j = 0; j < x_pos; j++)
        {
            for (int i = 0; i < DATA_DSP_HEIGHT; ++i)
            {
                PaintImageWaterfall(j,i);
            }
        }
        upd_pos_begin = 0;
        upd_pos_end = DATA_WIDTH;
        //qDebug()<<"PaintData_HD";
    }
    else
    {
        for (int j = s_upd_pos; j < x_pos; ++j)
        {
            for (int i = 0; i < DATA_DSP_HEIGHT; ++i)
            {
                PaintImageWaterfall(j,i);
            }
            SetWavePoints(j);
        }

        upd_pos_begin = (s_upd_pos - 2);//-2
        if (upd_pos_begin<0) //1.27
            upd_pos_begin = 0;
        upd_pos_end = (x_pos - upd_pos_begin + 4);//2 to 4 1.27
        //if(upd_pos_begin+upd_pos_end>DATA_WIDTH)
        //upd_pos_end = (upd_pos_begin+upd_pos_end)-DATA_WIDTH;

        s_upd_pos = x_pos;
        //qDebug()<<upd_pos_begin<<upd_pos_end<<DATA_WIDTH;
    }
    //update(upd_pos_begin,0,upd_pos_end,height());
    //update(upd_pos_begin*trans_fac,0,upd_pos_end*trans_fac+3,height());
    update(upd_pos_begin*trans_fac,0,upd_pos_end*trans_fac,height());
    //update(upd_pos_begin,0,upd_pos_end,height());
    //update();
    //qDebug()<<wave_points[100];
}
void DisplayMs::SetZap(bool f)
{
    s_zap = f;
}
void DisplayMs::SetButtonDecodeAll65(int ident_a)
{
    //qDebug()<<"DECODE BUTTTON================="<<raw_count<<disp_ident<<ident;
    if (ident_a==0)
    {
        if (vd_mouse_reset0 == 1)//inportent need for two mouse lines 1.36
        {
            //qDebug()<<"11111"<<vd_mouse_reset0<<vd_mouse_reset1;
            posx_vd_mouse_df[2] = posx_vd_mouse_df[0];
            posx_vd_mouse_df[3] = posx_vd_mouse_df[1];
            posy_vd_mouse0[1] = posy_vd_mouse0[0];
            posy_vd_mouse1[1] = posy_vd_mouse1[0];
        }

        vd_mouse_reset0 = 0;
        posx_vd_mouse_df[0] = lines_vd_df[0]/trans_fac;
        posx_vd_mouse_df[1] = lines_vd_df[1]/trans_fac;
        posy_vd_mouse0[0] = x_pos_freq_line+2;
        posy_vd_mouse1[0] = posy_vd_lines_t[0]-2;

        int count_t;
        GetRawAll(count_t);
        //mousebutton Left=1, Right=3 fullfile=0 rtd=2
        emit EmitDataToDecode(raw_t,count_t,s_time,0,0,false,true,s_fopen);
    }
    if (ident_a==1)
    {
        vd_mouse_reset1 = 1;
        posx_vd_mouse_df[2] = lines_vd_df[0]/trans_fac;
        posx_vd_mouse_df[3] = lines_vd_df[1]/trans_fac;
        posy_vd_mouse0[1] = posy_vd_lines_t[0]+2;
        posy_vd_mouse1[1] = posy_vd_lines_t[1]-2;

        if (vd_mouse_reset0 == 1)//inportent need for two mouse lines 1.36
        {
            //qDebug()<<"222";
            posx_vd_mouse_df[0] = posx_vd_mouse_df[2];
            posx_vd_mouse_df[1] = posx_vd_mouse_df[3];
            posy_vd_mouse0[0] = posy_vd_mouse0[1];
            posy_vd_mouse1[0] = posy_vd_mouse1[1];
        }
        emit EmitVDMouseDecodeSecondDisp(0);
    }
    update();
}
void DisplayMs::SetVDMouseDecodeSecondDisp(int mousebutton)
{
    //Decode second period ft jt65....
    int count_t;
    GetRawAll(count_t);
    //mousebutton Left=1, Right=3 fullfile=0 rtd=2
    emit EmitDataToDecode(raw_t,count_t,s_time,0,mousebutton,false,true,s_fopen);
    //qDebug()<<"ttttttttttt"<<mousebutton;
}
void DisplayMs::CorrDfW(int i)//2.24 ft8/4 df
{
    //qDebug()<<"1 EMIT"<<i<<ss_prev_res_;
    int res = 10;
    if (i>1000) 	res = 1500;
    else if (i>800) res = 1000;
    else if (i>600) res = 800;
    else if (i>550) res = 600;
    else if (i>500) res = 550;
    else if (i>450) res = 500;
    else if (i>400) res = 450;
    else if (i>350) res = 400;
    else if (i>300) res = 350;
    else if (i>250) res = 300;
    else if (i>200) res = 250;
    else if (i>150) res = 200;
    else if (i>100) res = 150;
    else if (i>50) 	res = 100;
    else if (i>20) 	res = 50;
    else if (i>10) 	res = 20;
    if (s_prev_vd_rx_df==res) return;
    s_prev_vd_rx_df=res;
    emit EmitVDRxDf(res);
    //qDebug()<<"2==== EMIT"<<res;
}
void DisplayMs::leaveEvent(QEvent *)
{
	f_show_bw = false;
	update();
}
void DisplayMs::mouseMoveEvent(QMouseEvent *event)
{
    if (f_disp_v_h)
    {
        int posx = event->pos().x();
        int df;
        if (s_mode==10)
            df = ((double)posx*(double)(s_stop-s_start)/(double)width() + (double)s_start) - 682.8125;  //682.8125 800.0
        else if (s_mode==11 || s_mode==13 || s_mode==18 || allq65)  //FT8 ft4
            df = ((double)posx*(double)(s_stop-s_start)/(double)width() + (double)s_start) - vd_tx_freq;//ft8 1200 vd_rx_freq
        else
            df = ((double)posx*(double)(s_stop-s_start)/(double)width() + (double)s_start) - 1270.46;//1270.46 vd_rx_freq

        if (df>0)
            vd_rx_df_txt = "DF: +"+QString("%1").arg(df)+" Hz";
        else
            vd_rx_df_txt = "DF: "+QString("%1").arg(df)+" Hz";

        //painter.fillRect(45, height()-16-2, 75, 16, QColor(0,0,0,150));//DF
        update(45, height()-16-2, 75, 16);
        //update();

        if (s_mode==11 || s_mode==13 || s_mode==18 || allq65)//2.24 ft8/4 df
        {
            if (f_mouse_pres)
            {
                if (f_r00) frq00 = ((double)posx*(double)(s_stop-s_start)/(double)width() + (double)s_start);
                if (f_r01) frq01 = ((double)posx*(double)(s_stop-s_start)/(double)width() + (double)s_start);
                //CorrFrqStratStop();
                //emit EmitVDRxFreq(vd_rx_freq,frq00,frq01);
                SetVDRxFreqF0F1();
                setVDispFreqScale(s_start,s_stop);
                //SaveFT8RxTxFreq(); //2.41 no needed
                CorrDfW(s_vd_df);//(frq01-frq00)/2
            }
            else
            {
                int istr = lines_vd_df[0];
                int isto = lines_vd_df[1];
                if (istr<0) istr = 0;
                if (isto>width()) isto = width();
                //painter.fillRect(istr-5,0,15,x_pos_freq_line,QColor(0,255,0,90));
                //painter.fillRect(isto-9,0,15,x_pos_freq_line,QColor(0,255,0,90));
                //painter.fillRect(lines_vd_df[0]-10,0,25,x_pos_freq_line,QColor(255,255,0,150));
                //painter.fillRect(lines_vd_df[1]-14,0,25,x_pos_freq_line,QColor(255,255,0,150));
                QRect r0 = QRect(istr-10,0,25,x_pos_freq_line);
                QRect r1 = QRect(isto-14,0,25,x_pos_freq_line);
                int posy = event->pos().y();
                if (r0.contains(posx,posy))
                {
                    setCursor(Qt::SplitHCursor);
                    f_r00 = true;
                }
                else if (r1.contains(posx,posy))
                {
                    setCursor(Qt::SplitHCursor);
                    f_r01 = true;
                }
                else
                {
                    f_r00 = false;
                    f_r01 = false;
                    setCursor(Qt::CrossCursor);
                }
            }  			
			QRect r2 = QRect(1,x_pos_freq_line,width(),height()-x_pos_freq_line);
			if (r2.contains(event->pos()) && s_show_bw)
			{
				mpos_x_bw = posx;
				f_show_bw = true;
				update();						 				
			} 
			else 
			{
				bool pf_show_bw = f_show_bw;
				f_show_bw = false;
				if (pf_show_bw) update();				
			}          
        }
    }
}
void DisplayMs::DecodeVD(int posy,int mousebutton)
{
    if (posy > x_pos_freq_line && posy < posy_vd_lines_t[0])
    {
        if (vd_mouse_reset0 == 1)//inportent need for two mouse lines 1.36
        {
            //qDebug()<<"11111"<<vd_mouse_reset0<<vd_mouse_reset1;
            posx_vd_mouse_df[2] = posx_vd_mouse_df[0];
            posx_vd_mouse_df[3] = posx_vd_mouse_df[1];
            posy_vd_mouse0[1] = posy_vd_mouse0[0];
            posy_vd_mouse1[1] = posy_vd_mouse1[0];
        }

        vd_mouse_reset0 = 0;
        posx_vd_mouse_df[0] = lines_vd_df[0]/trans_fac;
        posx_vd_mouse_df[1] = lines_vd_df[1]/trans_fac;
        posy_vd_mouse0[0] = x_pos_freq_line+2;
        posy_vd_mouse1[0] = posy_vd_lines_t[0]-2;

        int count_t;
        GetRawAll(count_t);
        //mousebutton Left=1, Right=3 fullfile=0 rtd=2
        emit EmitDataToDecode(raw_t,count_t,s_time,0,mousebutton,false,true,s_fopen);
    }
    if (posy > posy_vd_lines_t[0] && posy < posy_vd_lines_t[1])
    {
        vd_mouse_reset1 = 1;
        posx_vd_mouse_df[2] = lines_vd_df[0]/trans_fac;
        posx_vd_mouse_df[3] = lines_vd_df[1]/trans_fac;
        posy_vd_mouse0[1] = posy_vd_lines_t[0]+2;
        posy_vd_mouse1[1] = posy_vd_lines_t[1]-2;

        if (vd_mouse_reset0 == 1)//inportent need for two mouse lines 1.36
        {
            //qDebug()<<"222";
            posx_vd_mouse_df[0] = posx_vd_mouse_df[2];
            posx_vd_mouse_df[1] = posx_vd_mouse_df[3];
            posy_vd_mouse0[0] = posy_vd_mouse0[1];
            posy_vd_mouse1[0] = posy_vd_mouse1[1];
        }
        emit EmitVDMouseDecodeSecondDisp(mousebutton);
    }
}
/*void DisplayMs::SetFtDf1500(bool f)
{
    ft_df1500 = f;
}*/
void DisplayMs::ResetVDFreqDf()
{   //qDebug()<<s_mode<<s_prev_vd_rx_df<<s_vd_df;
    if (s_mode == 10) //pi4
    {
        vd_rx_freq = 682.8125;
        emit EmitVDRxDf(400);
    }
    else if (s_mode == 11 || s_mode==13 || s_mode==18) //ft8  ft4  for deffault df || allq65
    {   
        //if (s_vd_df>1000)//if (ft_df1500) or s_prev_vd_rx_df
        //{
            vd_rx_freq = 1700.0;
            if (s_mode==11 && id_mshf>0) vd_rx_freq = 750;
            vd_tx_freq = vd_rx_freq;
            //if (s_mode==11 && id_mshf==2) vd_tx_freq = 750;//2.76sh
            //if (s_mode==11 && id_mshf==1) vd_rx_freq = 750;//2.76sh
            SetVDTxFreq();
            emit EmitVDRxDf(1500);
            frq00 = 200.0;
            frq01 = 3200.0;
            s_vd_df = 1500;//<- importent
        //}
        /*else
        {
            vd_rx_freq = 1200.0;
            if (s_mode==11 && id_mshf>0) vd_rx_freq = 750;
            vd_tx_freq = vd_rx_freq;
            //if (s_mode==11 && id_mshf==2) vd_tx_freq = 750;//2.76sh
            //if (s_mode==11 && id_mshf==1) vd_rx_freq = 750;//2.76sh
            SetVDTxFreq();
            emit EmitVDRxDf(1000);
            frq00 = 200.0;
            frq01 = 2200.0;
            s_vd_df = 1000;//<- importent
        }*/
    }
    else if (allq65)//remove for df 1500
    {
        vd_rx_freq = 1200.0;
        vd_tx_freq = vd_rx_freq;
        SetVDTxFreq();
        emit EmitVDRxDf(1000);
        frq00 = 200.0;
        frq01 = 2200.0;
        s_vd_df = 1000;//<- importent
    }
    else
    {
        vd_rx_freq = 1270.46;//(double)posx*(double)(s_stop-s_start)/(double)width() + (double)s_start;
        emit EmitVDRxDf(600);
    }

    vd_rx_freq_txt = "RX: "+QString("%1").arg((int)vd_rx_freq)+" Hz";
    //CorrFrqStratStop();
    //emit EmitVDRxFreq(vd_rx_freq,frq00,frq01);
    SetVDRxFreqF0F1();
    setVDispFreqScale(s_start,s_stop);
    SaveFT8RxTxFreq();
}
void DisplayMs::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (f_disp_v_h)
    {
        int posy=event->pos().y();
        if (event->button()== Qt::LeftButton && posy > x_pos_freq_line)//Qt::RightButton Qt::LeftButton
        {
            if (s_mode==10)//pi4
                emit EmitVDRxDf(100);
            else if (s_mode==11)//ft8
                emit EmitVDRxDf(50);
            else if (s_mode==13)//ft4
                emit EmitVDRxDf(100);
            else if (s_mode==18)//ft2
                emit EmitVDRxDf(200);
            else if (allq65)   //q65? limit 10hz ???
                emit EmitVDRxDf(50);
            else
                emit EmitVDRxDf(50);//jt65

            DecodeVD(posy,1);//mousebutton Left=1, Right=3 fullfile=0 rtd=2
            update();
        }
    }
}
void DisplayMs::mouseReleaseEvent(QMouseEvent *)//2.24 ft8/4 df
{
    //if (f_mouse_pres) setCursor(Qt::CrossCursor);
    f_mouse_pres = false;
    f_r00 = false;//2.41 for any case
    f_r01 = false;//2.41 for any case
    //qDebug()<<"replace";
    update();//mouse lines
}
void DisplayMs::mousePressEvent(QMouseEvent *event)
{
    if (f_dec_busy) return;
    if (event->button()== Qt::LeftButton || event->button()== Qt::RightButton)
    {
        if (f_disp_v_h)//((double)posx*(double)(s_stop-s_start)/(double)width() + (double)s_start) - 800.0;
        {
            int posy = event->pos().y();
            int posx = event->pos().x();

            if (event->modifiers()==Qt::ControlModifier && event->button()== Qt::LeftButton && posy > x_pos_freq_line)
            {
                //TX=RX
                vd_rx_freq = (double)posx*(double)(s_stop-s_start)/(double)width() + (double)s_start;
                vd_tx_freq = vd_rx_freq;
                SetVDTxFreq();
                vd_rx_freq_txt = "RX: "+QString("%1").arg((int)vd_rx_freq)+" Hz";
                //CorrFrqStratStop();
                //emit EmitVDRxFreq(vd_rx_freq,frq00,frq01);
                SetVDRxFreqF0F1();
                setVDispFreqScale(s_start,s_stop);
                SaveFT8RxTxFreq();
            }
            else if (event->modifiers()==Qt::ShiftModifier && event->button()== Qt::LeftButton && posy > x_pos_freq_line)
            {
                //TX
                vd_tx_freq = (double)posx*(double)(s_stop-s_start)/(double)width() + (double)s_start;
                SetVDTxFreq();
                if (f_lock_txrx)
                {
                    vd_rx_freq = vd_tx_freq;
                    vd_rx_freq_txt = "RX: "+QString("%1").arg((int)vd_rx_freq)+" Hz";
                    //CorrFrqStratStop();
                    //emit EmitVDRxFreq(vd_rx_freq,frq00,frq01);
                    SetVDRxFreqF0F1();
                }
                setVDispFreqScale(s_start,s_stop);
                SaveFT8RxTxFreq();
            }
            else if (event->button()== Qt::LeftButton && posy > x_pos_freq_line)
            {
                //RX
                vd_rx_freq = (double)posx*(double)(s_stop-s_start)/(double)width() + (double)s_start;
                if (f_lock_txrx)
                {
                    vd_tx_freq = vd_rx_freq;
                    SetVDTxFreq();
                }
                vd_rx_freq_txt = "RX: "+QString("%1").arg((int)vd_rx_freq)+" Hz";
                //CorrFrqStratStop();
                //emit EmitVDRxFreq(vd_rx_freq,frq00,frq01);
                SetVDRxFreqF0F1();
                setVDispFreqScale(s_start,s_stop);
                SaveFT8RxTxFreq();
            }
            if ((event->button()== Qt::RightButton || event->button()== Qt::LeftButton) && (posy > 0 && posy < x_pos_freq_line))
            {
                if ((s_mode==11 || s_mode==13 || s_mode==18 || allq65) && cursor().shape()==Qt::SplitHCursor)//2.24 ft8/4 df
                {
                    f_mouse_pres = true;
                }
                else
                    ResetVDFreqDf();
            }
            if (event->button()== Qt::RightButton &&  posy > x_pos_freq_line)//Qt::LeftButton Qt::RightButton
            {
                vd_rx_freq = (double)posx*(double)(s_stop-s_start)/(double)width() + (double)s_start;
                if (f_lock_txrx)
                {
                    vd_tx_freq = vd_rx_freq;
                    SetVDTxFreq();
                }
                vd_rx_freq_txt = "RX: "+QString("%1").arg((int)vd_rx_freq)+" Hz";
                //CorrFrqStratStop();
                //emit EmitVDRxFreq(vd_rx_freq,frq00,frq01);
                SetVDRxFreqF0F1();
                setVDispFreqScale(s_start,s_stop);
                DecodeVD(posy,3);//mousebutton Left=1, Right=3 fullfile=0 rtd=2
                SaveFT8RxTxFreq();
            } 
        }
        else
        {
            //mousebutton Left=1, Right=3 fullfile=0 rtd=2
            int mousebutton = 1;
            int d_a_p_begin = event->pos().x()/trans_fac-dec_array_pixel_begin;
            int d_a_p_end = event->pos().x()/trans_fac+dec_array_pixel_end;

            if (event->button()== Qt::RightButton)
            {
                mousebutton = 3; //mousebutton Left=1, Right=3 fullfile=0 rtd=2
                d_a_p_begin = event->pos().x()/trans_fac-dec_array_pixel_begin*2;
                d_a_p_end = event->pos().x()/trans_fac+dec_array_pixel_end*2;
            }

            if (s_zap)
            {
                for (int j = 0; j<raw_count; ++j)
                {
                    raw_t[j] = raw[j];
                }
                emit EmitZapData(raw_t,raw_count);
            }

            for (int i = 0; i<COUNT_MOUSE_LINES; ++i)
            {
                if (d_a_p_begin<0)
                {
                    d_a_p_end += abs(d_a_p_begin);
                    d_a_p_begin=0;
                }

                if (d_a_p_end>=DATA_WIDTH-1)//1.27 -1
                {
                    d_a_p_begin -= abs(d_a_p_end-(DATA_WIDTH-1));//1.27 -1
                    d_a_p_end = DATA_WIDTH-1; //1.27 -1 d_a_p_end = DATA_WIDTH;
                }

                if (i==0)
                    lines_mouse[i] = QLine(d_a_p_begin, x_pos_timeline, d_a_p_begin, height());

                if (i==1)
                    lines_mouse[i] = QLine(d_a_p_end, x_pos_timeline, d_a_p_end, height());
            }

            double arr_samp_begin = (double)d_a_p_begin*(double)STAT_FFTW_H30_TIME*((double)DISP_SAMPLE_RATE/(double)DATA_WIDTH);
            double arr_samp_end = (double)d_a_p_end*(double)STAT_FFTW_H30_TIME*((double)DISP_SAMPLE_RATE/(double)DATA_WIDTH);

            int begin_samp = (int)(arr_samp_begin);

            int end_samp = (int)(arr_samp_end);

            int count_t = 0;
            for (int j = begin_samp; j<end_samp; j++)
            {
                raw_t[count_t] = raw[j];
                count_t++;
            }
            //mousebutton Left=1, Right=3 fullfile=0 rtd=2
            emit EmitDataToDecode(raw_t,(end_samp-begin_samp),s_time,begin_samp,mousebutton,false,true,s_fopen);
            //qDebug()<<count_t;
        }
        update();
    }
}
void DisplayMs::resizeEvent(QResizeEvent *)
{
	f_show_bw = false;
    if (f_disp_v_h) setVDispFreqScale(s_start,s_stop);
    else setHDispPeriodTime(period_time_sec,s_mode);
}
void DisplayMs::SetDecLinesPosToDisplay(int count,double pos_ping_beg,double pos_ping,QString p_time)//1.28 p_time for identif perood
{
    //pos_ping=0.001;
    //qDebug()<<"Begin======="<<pos_ping;
    if (p_time!=s_time)// if not from this period try to sent to secondary display
    {
        //qDebug()<<"NOT From This Period"<<pos_ping<<count;
        emit EmitDecLinesPosToSecondDisp(count,pos_ping_beg,pos_ping,p_time);
        return;
    }

    if (pos_ping_beg>=ping_pos_end_draw)
        pos_ping_beg = ping_pos_end_draw;

    int pos_t = ((double)DATA_WIDTH*pos_ping_beg)/STAT_FFTW_H30_TIME;

    if (pos_t==0) //if beginning +2 to be visible v1.32
        pos_t = 2;

    QString lab_txt = "Dec "+QString("%1").arg(count);
    if (s_mode == 4 || s_mode == 5) // iscak-a  iscak-b from iscat decoder only
    {
        if (count==1)
            lab_txt = "From";
        if (count==2)
            lab_txt = "To";
    }

    if (dec_labels_posx_count == 0)
    {
        dec_labels_posx[dec_labels_posx_count] = pos_t;
        dec_labels_text[dec_labels_posx_count] = lab_txt;
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)//2.56
        fm_dec_labels_txt[dec_labels_posx_count] = fm_X->horizontalAdvance(lab_txt);
#else
        fm_dec_labels_txt[dec_labels_posx_count] = fm_X->width(lab_txt);
#endif
        dec_labels_posx_count++;
    }
    else
    {
        if (pos_t==dec_labels_posx[dec_labels_posx_count-1])
        {
            dec_labels_posx[dec_labels_posx_count-1] = pos_t;
            dec_labels_text[dec_labels_posx_count-1] = lab_txt;
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)//2.56
            fm_dec_labels_txt[dec_labels_posx_count-1] = fm_X->horizontalAdvance(lab_txt);
#else
            fm_dec_labels_txt[dec_labels_posx_count-1] = fm_X->width(lab_txt);
#endif
        }
        else if (dec_labels_posx_count < MAX_DEC_LABELS_COUNT)
        {
            dec_labels_posx[dec_labels_posx_count] = pos_t;
            dec_labels_text[dec_labels_posx_count] = lab_txt;
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)//2.56
            fm_dec_labels_txt[dec_labels_posx_count] = fm_X->horizontalAdvance(lab_txt);
#else
            fm_dec_labels_txt[dec_labels_posx_count] = fm_X->width(lab_txt);
#endif
            dec_labels_posx_count++;
        }
    }

    if (dec_lines_posx_count < MAX_DEC_LINES_COUNT)
    {
        int pos_ap = ((double)DATA_WIDTH*pos_ping)/STAT_FFTW_H30_TIME;

        if (pos_ap==0) //if beginning +2 to be visible v1.32
            pos_ap = 2;

        if (dec_lines_posx_count == 0)
            dec_lines_posx[dec_lines_posx_count] = pos_ap;
        else if (pos_ap==dec_lines_posx[dec_lines_posx_count-1])
            dec_lines_posx[dec_lines_posx_count] = pos_ap+1;// pokazwa pinga koito se doblira vednaga sled pyrvia
        else
            dec_lines_posx[dec_lines_posx_count] = pos_ap;

        dec_lines_posx_count++;
    }

    //qDebug()<<"END --------- pos_ping"<<pos_ping;

    f_dec_pings_draw = true;
    update();
}

void DisplayMs::paintEvent(QPaintEvent *)
{
    QPainter painter( this );
    //painter.drawPixmap(0, 0, pupdate_hv);
    //painter.setRenderHint(QPainter::Antialiasing);
    //painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.save();

    painter.fillRect(0, 0, width(), height(), QColor(0,0,0,255));

    QTransform transform;
    transform.scale(trans_fac, 1.0);
    painter.setTransform(transform);

    if (f_disp_v_h)
    {
        painter.drawImage(0,pos_y_v0+x_pos_freq_line,img_tmp_v0);
        painter.drawImage(0,pos_y_v1+x_pos_freq_line,img_tmp_v1);
    }
    else
    {
        painter.drawImage(0,0,img_tmp);

        painter.setPen(s_pen_wave); // ,0 need qt5 v1.27 QPen(QColor(xxx,xxx,xxx),0,Qt::SolidLine) Qt::SolidLine=1
        painter.setBrush(s_brush_wave);
        painter.drawPolygon(wave_points, (DATA_WIDTH+2));

        //transform.scale((1.0/trans_fac), 1.0);
        //painter.setTransform(transform);

        painter.setPen(QPen(QColor(255,255,255,230),0)); // ,0 need qt5 v1.27 QPen(QColor(xxx,xxx,xxx),0,Qt::SolidLine) Qt::SolidLine=1
        painter.drawLines(lines_mouse,COUNT_MOUSE_LINES);

        painter.setPen(QPen(QColor(255,0,0,230),0)); // ,0 need qt5 v1.27 QPen(QColor(xxx,xxx,xxx),0,Qt::SolidLine) Qt::SolidLine=1
        painter.drawLines(lines_rtd,COUNT_MOUSE_LINES);

    }

    transform.scale((1.0/trans_fac), 1.0);
    painter.setTransform(transform);

    if (f_disp_v_h)
    {
        //// ???? jt65
        painter.setPen(QPen(QColor(255,255,255),0));
        for (int i = 0; i < lines_vd_t_count; ++i)
        {
            painter.drawText(10,posy_vd_lines_t[i]-1,lines_vd_t_text[i]);
            painter.drawLine(3, posy_vd_lines_t[i], width()-4, posy_vd_lines_t[i]);
        }

        if (vd_mouse_lines_draw)//2.72 work for all, ON/OFF only for JT65
        {
            painter.setPen(QPen(QColor(255,255,255),0,Qt::DashLine));//Qt::SolidLine Qt::DashLine
            for (int i = 0; i < 4; i++)
            {
                if (i==0 || i==1) painter.drawLine(posx_vd_mouse_df[i]*trans_fac, posy_vd_mouse0[0], posx_vd_mouse_df[i]*trans_fac, posy_vd_mouse1[0]);
                if (i==2 || i==3) painter.drawLine(posx_vd_mouse_df[i]*trans_fac, posy_vd_mouse0[1], posx_vd_mouse_df[i]*trans_fac, posy_vd_mouse1[1]);
            }
        }
        if (f_show_bw)//2.72 
		{   
			//painter.setPen(QPen(QColor(255,255,255),0,Qt::DashLine)); 
			painter.drawLine(mpos_x_bw-1,x_pos_freq_line,mpos_x_bw-1,height()); 
			//int tt0 = mpos_x_bw+line_bw+1;
			//if (tt0<width()) painter.drawLine(tt0,x_pos_freq_line,tt0,height());
			painter.drawLine(mpos_x_bw+line_bw+1,x_pos_freq_line,mpos_x_bw+line_bw+1,height());						      
		}

        if (dsty) painter.fillRect(0,0,width(),x_pos_freq_line,QColor(10,10,10));
        else painter.fillRect(0,0,width(),x_pos_freq_line,QColor(255,255,255));

        QColor cc1 = QColor(212,222,242);
        if (dsty) cc1 = QColor(80,90,110);
        painter.fillRect(0,0,lines_vd_df[0],x_pos_freq_line,cc1);
        painter.fillRect(lines_vd_df[1],0,width(),x_pos_freq_line,cc1);

        if (f_mouse_pres)
        {
            if (f_r00)//left
            {
                painter.fillRect(0,x_pos_freq_line,lines_vd_df[0],height(),QColor(212,222,242,30));
                painter.setPen(QPen(QColor(255,255,255,180),0,Qt::DashLine));
                painter.drawLine(lines_vd_df[0], x_pos_freq_line, lines_vd_df[0], height());
            }
            if (f_r01)//right
            {
                painter.fillRect(lines_vd_df[1],x_pos_freq_line,width(),height(),QColor(212,222,242,30));
                painter.setPen(QPen(QColor(255,255,255,180),0,Qt::DashLine));
                painter.drawLine(lines_vd_df[1], x_pos_freq_line, lines_vd_df[1], height());
            }
        }
		
        //if (dsty) painter.setPen(QPen(QColor(255,255,255),0));
        if (dsty) painter.setPen(QPen(QColor(140,140,140),0));
        else painter.setPen(QPen(QColor(0,0,0),0));
        painter.drawLine(0, 0, width(), 0);
        if (dsty) painter.setPen(QPen(QColor(255,255,255),0));
        painter.drawLine(0, x_pos_freq_line-3, width(), x_pos_freq_line-3);

        painter.drawLines(lines_scale_x,count_lines_scale_x);
        for (int i = 0; i < count_lines_scale_x; i++)
        {
            painter.drawText(pos_text_hor_X[i], pos_text_hor_Y,freq_scale_x[i]);
        }

        QColor crx = QColor(0,240,0);//2.72
        QColor cry = QColor(250,0,0);//2.72
		if (s_mode==11 || s_mode==13 || s_mode==18 || allq65)//2.72
		{
			if (s_show_bwtx)
			{	
				painter.setPen(QPen(cry,0,Qt::DashLine));
				painter.drawLine(lines_vd_rx_freq[1],x_pos_freq_line,lines_vd_rx_freq[1],height());				
				painter.drawLine(lines_vd_rx_freq[2]+1,x_pos_freq_line,lines_vd_rx_freq[2]+1,height());
			}			
			if (s_show_bwrx)
			{
				painter.setPen(QPen(crx,0,Qt::DashLine));			
				painter.drawLine(lines_vd_rx_freq[4],x_pos_freq_line,lines_vd_rx_freq[4],height());			
				painter.drawLine(lines_vd_rx_freq[5],x_pos_freq_line,lines_vd_rx_freq[5],height());				
			}
		}                
        for (int i = 0; i < 6; ++i)
        {
            if (s_mode==10)
            {
                if (i==0 || i==2) painter.setPen(QPen(cry,3));
                else painter.setPen(QPen(crx,3));//2.63 old 250
            }
            else
            {
                if (i>0 && i<4) painter.setPen(QPen(cry,3));
                else painter.setPen(QPen(crx,3));//2.63 old 250
            }            
            painter.drawLine(lines_vd_rx_freq[i], x_pos_freq_line-4, lines_vd_rx_freq[i], x_pos_freq_line-8);//QLine(xpos_frq1,x_pos_freq_line-line_height_pix-3,0+xpos_frq1,x_pos_freq_line-3);                        
        }
        painter.setPen(QPen(crx,2));//2.63 old 250
        painter.drawLine(lines_vd_df[0],x_pos_freq_line-1,lines_vd_df[1],x_pos_freq_line-1);

        /*
        painter.fillRect(1, x_pos_freq_line-16-2, 75, 16, QColor(0,0,0,150));//DF
        painter.fillRect(width()-70-1, x_pos_freq_line-16-2, 70, 16, QColor(0,0,0,150));//RX FREQ vd_rx_freq
        painter.setPen(QPen(QColor(255,255,255),0));
        painter.drawText(1+4, x_pos_freq_line+0-4-2,vd_rx_df_txt);
        painter.drawText(width()-70-1+4, x_pos_freq_line+0-4-2,vd_rx_freq_txt);
        */

        /*
        painter.fillRect(45, x_pos_freq_line+0, 75, 16, QColor(0,0,0,150));//DF
        painter.fillRect(width()-70-45, x_pos_freq_line+0, 70, 16, QColor(0,0,0,150));//RX FREQ vd_rx_freq
        painter.setPen(QPen(QColor(255,255,255),0));
        painter.drawText(45+4, x_pos_freq_line+16-4,vd_rx_df_txt);
        painter.drawText(width()-70-45+4, x_pos_freq_line+16-4,vd_rx_freq_txt);
        */

        painter.fillRect(45, height()-16-2, 75, 16, QColor(0,0,0,150));//DF
        painter.fillRect(width()-70-45, height()-16-2, 70, 16, QColor(0,0,0,150));//RX FREQ vd_rx_freq
        painter.setPen(QPen(QColor(255,255,255),0));
        painter.drawText(45+4, height()-4-2,vd_rx_df_txt);
        painter.drawText(width()-70-45+4, height()-4-2,vd_rx_freq_txt);

        //painter.fillRect(lines_vd_df[0]-5,0,15,x_pos_freq_line,QColor(255,255,0,150));
        //painter.fillRect(lines_vd_df[1]-9,0,15,x_pos_freq_line,QColor(255,255,0,150));
        //painter.fillRect(lines_vd_df[0]-10,0,25,x_pos_freq_line,QColor(255,255,0,150));
        //painter.fillRect(lines_vd_df[1]-14,0,25,x_pos_freq_line,QColor(255,255,0,150));
    }
    else
    {
        if (f_dec_pings_draw)
        {
            painter.setPen(QPen(QColor(255,0,0,230),0,Qt::DashLine));
            for (int i = 0; i < dec_lines_posx_count; ++i)
                painter.drawLine(dec_lines_posx[i]*trans_fac, x_pos_timeline+2, dec_lines_posx[i]*trans_fac, height());

            painter.setPen(QPen(QColor(255,255,255),0)); // ,0 need qt5 v1.27 QPen(QColor(xxx,xxx,xxx),0,Qt::SolidLine) Qt::SolidLine=1
            for (int i = 0; i < dec_labels_posx_count; ++i)
            {
                painter.fillRect((dec_labels_posx[i])*trans_fac-2, x_pos_timeline+2, fm_dec_labels_txt[i]+4, 16, QColor(0,0,0,150));
                painter.drawText(dec_labels_posx[i]*trans_fac, x_pos_timeline+2+12,dec_labels_text[i]);
            }
        }

        painter.setPen(QPen(QColor(255,255,255),0)); // ,0 need qt5 v1.27 QPen(QColor(xxx,xxx,xxx),0,Qt::SolidLine) Qt::SolidLine=1
        painter.drawLine(0, x_pos_timeline, width(), x_pos_timeline);

        painter.drawLines(lines_scale_x,count_lines_scale_x);

        bool f_even = true;
        for (int i = 0; i < count_lines_scale_x; ++i)
        {
            if (period_time_sec>32)
            {
                if (f_even)
                {
                    painter.drawText(pos_text_hor_X[i], pos_text_hor_Y,QString("%1").arg(i));
                    f_even = false;
                }
                else
                    f_even = true;
            }
            else
                painter.drawText(pos_text_hor_X[i], pos_text_hor_Y,QString("%1").arg(i));
        }

        if (f_disp_time)//2.43
        {
            //qDebug()<<disp_ident<<s_fopen<<s_time;
            //QString txt = "00:00:00";
            //if (!s_fopen) txt = s_time.mid(0,2)+":"+s_time.mid(2,2)+":"+s_time.mid(4,2);
            QString txt = s_time.mid(0,2)+":"+s_time.mid(2,2)+":"+s_time.mid(4,2);
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)//2.56
            int widd = fm_X->horizontalAdvance(txt)+4;
#else
            int widd = fm_X->width(txt)+4;
#endif
            painter.fillRect(9, height()-45, widd, 16, QColor(0,0,0,150));//-33 -8
            painter.drawText(11,height()-33,txt);
            /*int off = 50;
            painter.fillRect(8, x_pos_timeline+20+off, widd, 16, QColor(0,0,0,150));
            painter.drawText(10,x_pos_timeline+32+off,txt);*/
        }
    }

    if (dsty) painter.setPen(QPen(QColor(140,140,140),0));
    else painter.setPen(QPen(QColor(255,255,255),0));
    painter.drawLine(0, 0, 0, height());
    painter.drawLine(width()-1, 0, width()-1, height());
    painter.drawLine(0, height()-1, width(), height()-1);

    painter.restore();
}




