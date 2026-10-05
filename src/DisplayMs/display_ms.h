/* MSHV DisplayMs
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#ifndef DISPLAY_MS_H
#define DISPLAY_MS_H

#include "../config.h"


#include <QPainter>
#include <QMouseEvent>
#include <QTimer>
#include <QWidget>

#define RAW_BUFFER_SIZE  (12000*130) //2.53=120 old=90 60s=normal -> for any case 90s  samplerate*time in seconds 1min=60s
#define RAW_BUF_PREV_PI4 (36000)     //2.16 3s from prew perod    1s from prew perod 12000*3 3sec

/*
#if defined _WIN32_
//#include "../../../DirectX90c/include/dsound.h"// zaradi DWORD in pathred create
#include "../Hv_Lib_DirectX90c/dsound.h"
#endif
*/

#define COUNT_MOUSE_LINES 2
#define MAX_LINES_SCALE_X 320 //for=5min old=216 //1.35 60s=normal -> for any case 90s // jt65 5000hz/50hz=100 lines
#define MAX_DEC_LABELS_COUNT 40 //max 40 labels for 30s and decoderms.h
#define MAX_DEC_LINES_COUNT 140 //max 140 lines for 30s and decoderms.h
//#include <QMouseEvent>
//#include <math.h>
#include <unistd.h> // zaradi usleep pthread.h

//#include <QGLWidget>


class DisplayMs : public QWidget
{
    Q_OBJECT
public:
    DisplayMs(int dispident,bool,QWidget * parent = 0);
    ~DisplayMs();

    void SetPalette(int identif);
    void SetDisplayRefr(int val);
    //void SetSampleRate(int val);
    QString default_color()
    {
        return QString("%1").arg(saved_color);
    };

    void ResetToBegin();//QString,QString
    void SetSyncPosition(int,QString,QString,bool decodet);
    void setHDispPeriodTime(int,int);//setPeriodTime(int);
    
    void setVDispFreqScale(int,int);
    
    void setDisplayTime(QString,QString);
    QString getFullFileName()
    {
        return full_file_name;
    }
    void setArrayInPxel(int);
    void DecodeAllData(bool decode,bool external,bool);//2.76
    void SetMode(int);
    void SetVDisplayHight(int);
    void SetButtonDecodeAll65(int ident);//1.49
    void setTune(int,int);//1.55=
    void SetFont(QFont);
    void SetFreqRxTxVD(QString rxtx);
    QString GetFreqRxTxVD();
    void SetAdleDisplay_VD(bool);//2.45
    void SetFlatDisplay_VD(bool);//1.51 
    void SetMsh(uint8_t);

signals:
    void SentData(QList<int*>,QPoint*,int,int*,QString t_min,QString ymd,bool,QLine *mous_lines,
                  int*,QString*,int*,int,int*,int,bool,QLine *linesrtd,int posrtd,int updposrtd,bool endrtd); //1.27 psk rep   fopen bool true    false no file open
    //void EmitDecodetText(QStringList);            
    void EmitDataToDecode(int*,int,QString t_min,int t_istart,int mousebutton,bool f_rtd,bool end_rtd,bool fopen);//1.27 psk rep   fopen bool true    false no file open
    void EmitDataToSaveInFile(int*,int,QString);
    void EmitFileNameChenged();
    void EmitZapData(int*,int);
    
    void EmitDecLinesPosToSecondDisp(int count,double pos,double pos_ping,QString p_time);//1.28 p_time is not from this period sent to second display

    void EmitRriorityDisp(bool);
    void EmitResetToBegin(); 
    void EmitVDMouseDecodeSecondDisp(int mousebutton);
    void EmitVDRxFreqF0F1(double,double,double);
    void EmitVDRxDf(int);
    void EmitVDTxFreq(double);
    //void EmitButtonVDDecodeAllToSecondDsp65();//1.49
    void EmitIsDispHaveDataForDec65(int,bool);//1.49

public slots:
    //void SetFtDf1500(bool);
	void Decode3intFt(bool);//2.39 remm
    void SetDecLinesPosToDisplay(int count,double pos,double pos_ping,QString p_time);//1.28 p_time for identif perood
    void SetValue(double*, int);
    void SetRaw(int*,int,bool); //1.27 psk rep   fopen bool true    false no file open
    void ReciveData(QList<int*>,QPoint*,int,int*,QString t_min,QString ymd,bool,QLine *mous_lines,
                    int*,QString*,int*,int,int*,int,bool,QLine *linesrtd,int posrtd,int updposrtd,bool endrtd);//1.27 psk rep   fopen bool true    false no file open
    void ReciveClarDisplay();
    //void setTune(int,int);
    //void setTuneCont(int,int);
    void SaveFile(QString);
    void SetAutoDecodeAll(bool);
    void SetZap(bool);
    ///rtd//
    void SetDecBusy(bool,int);
    void SetStartStopRtd(bool);
    void SetRriorityDisp(bool);
    void SetVDMouseDecodeSecondDisp(int mousebutton);
    //void SetButtonVDDecodeAllIfHaveInafDataOrSetToSecondDsp65();//1.49
    //void SetButtonVDDecodeAllToSecondDsp65();//1.49
    void SetVDMouseDrawLines(bool);//1.49
    //void SetVDMouseDecodeSecondDisp2(int mousebutton);
    void SetVDdf(int,int);
    void SetZeroDfVScale(bool);
    void SetCustomPalette(QPixmap,QColor,QColor);
    void SetTxToRx(bool);
    void SetRxToTx(bool);//2.63
    void SetLockTxrx(bool);
    void SetFreqExternal(double);
    void SetVDMouseBWDrawLines(bool);//2.72
    void SetVDMouseBWRXDrawLines(bool);
    void SetVDMouseBWTXDrawLines(bool);
    ///rtd//
private slots:
    ///rtd//
    void RtdDecode();
    ///rtd//
    //ft8 3 dec
    void Ft8Decode();

private:
	uint8_t id_mshf;
	bool dsty;
	bool allq65;
    //bool ft_df1500;
    bool f_is_decodet3int;
    bool s_3intFt_d_;//2.39 remm
    int dflimit;
    void SetVDRxFreqF0F1();
    void SetVDTxFreq();
    void CorrDfW(int i);
    int count_disp_data_dec65;//1.49
    bool one_emit_disp_data_dec65;//1.49
    bool vd_mouse_lines_draw;//1.49
    bool s_vd_mouse_lines_draw65;//1.49
    int s_mode;
    bool s_fopen; //1.27 psk rep   fopen bool true    false no file open
    QFontMetrics *fm_X;
    int dec_labels_posx[MAX_DEC_LABELS_COUNT+10];
    QString dec_labels_text[MAX_DEC_LABELS_COUNT+10];
    bool f_dec_pings_draw;
    int dec_labels_posx_count;
    double ping_pos_end_draw;
    int fm_dec_labels_txt[MAX_DEC_LABELS_COUNT+10];
    int dec_lines_posx[MAX_DEC_LINES_COUNT+10];
    int dec_lines_posx_count;

    double DISP_SAMPLE_RATE;
    bool s_zap;
    void SeavPrevRaw();
    void GetRawAll(int &cou_t);
    int raw[RAW_BUFFER_SIZE];
    int raw_t[RAW_BUFFER_SIZE];
    int raw_prev_pi4[RAW_BUF_PREV_PI4];
    int raw_count;
    //int sampe_rate;

    int period_time_sec;
    //int time_pos;

    void PaintImageWaterfall(int j, int i);
    //QPoint draw_point[DATA_WIDTH_DRAW];
    int data_graph_all[DATA_WIDTH+100][DATA_DSP_HEIGHT+100]; //1.35 +100 moze da izleze ako pc frizne
    int data_graph_smiter[DATA_WIDTH+100];                  //1.35 +100 moze da izleze ako pc frizne

    int x_pos;
    QImage img_tmp;
    
    int s_start,s_stop;
    int x_pos_freq_line;
    QString freq_scale_x[MAX_LINES_SCALE_X];
    bool f_disp_v_h;
    int var_vdisp_height;
    int data_graph_all_v[DATA_HEIGHT*2+100][DATA_WIDTH+100];//1.39 rem DATA_WIDTH/2 
    
    bool f_adle_vd;
    bool f_flat_vd;
     
    void FlatDisplay_VD(int *k_segm, int row);//1.51
    void PaintData_VD(bool);
    int pos_y_v0;
    int pos_y_v1;
    int pos_y_draw_vimgs;
    bool f_img_v0_v1;
    QImage img_tmp_v0;
    QImage img_tmp_v1;
    int lines_vd_t_count;
    bool lines_vd_t_new;
    int posy_vd_lines_t[50];    //1.55=40 for new count speads max 40 lines old=20 
    QString lines_vd_t_text[50];//1.55=40 for new count speads max 40 lines old=20
    
    int vd_mouse_reset0;
    int vd_mouse_reset1;
    int posx_vd_mouse_df[4];
    int lines_vd_df[2];
    int posy_vd_mouse0[2];
    int posy_vd_mouse1[2];
    //int posy_vd_mouse2[COUNT_MOUSE_LINES];
    //int posy_vd_mouse3[COUNT_MOUSE_LINES];
    
	double frq00;
	double frq01;
	double frq00_limit;
	double frq01_limit;
	double bwQ65;
	void RefreshLimits();
	bool f_mouse_pres;
	bool f_r00;
	bool f_r01;

    double vd_rx_freq;
    double vd_tx_freq;//// only for ft8 for the moment 1.43
    QString vd_rx_freq_txt;
    QString vd_rx_df_txt;
    double s_ft8_vd_rxtx_freq[2];
    void SaveFT8RxTxFreq();
    int lines_vd_rx_freq[6];
    int s_vd_df;
    int s_prev_vd_rx_df;
    void DecodeVD(int pos, int mouse);
    void ResetVDFreqDf();
    //bool df_vd_50;
    //QRect visual_rect_vd0;
    //QRect visual_rect_vd1;
    
    //int img_tmp_format;
    int count_refr_disp;
    QPoint wave_points[DATA_WIDTH+1+3+100];
    void SetWavePoints(int);
    //int count_wave_points;

    void PaintData_HD(bool paint_all);
    void ClarDisplay();
    void ClarHDisplay();

    int s_upd_pos;
    void SetPaletteParm(QPixmap pic);
    QColor getColor(int val);
    QImage palette_tmp;
    typedef struct
    {
        int rgb_max;
        int rgb_img_width_center;
    }
    coef_rgb;
    coef_rgb rgb_c;
    QPixmap temp_pal0;
    QPixmap temp_pal1;
    QPixmap temp_pal2;
    QPixmap temp_pal3;
    QPixmap temp_pal4;
    QPixmap temp_pal5;
    QPixmap temp_pal6;
    QPixmap temp_pal7;
    QPixmap temp_pal_custom;
    QColor s_custom_wave;
    QColor s_custom_pen_wave;
    QPen s_pen_wave;
    QColor s_brush_wave;

    int saved_color;
    int refresh_time;

    int x_pos_timeline;
    QLine lines_scale_x[MAX_LINES_SCALE_X];
    int pos_text_hor_X[MAX_LINES_SCALE_X];
    int count_lines_scale_x;
    int  pos_text_hor_Y;
    bool f_zero_freq_scale;
    //int x_pos_timeline;

    QLine lines_mouse[COUNT_MOUSE_LINES];
    int dec_array_pixel_begin;
    int dec_array_pixel_end;
    QString full_file_name;
    //int offset_wat_sm;
    int offset_down_sm;
    //bool f_s_decode;
    bool f_is_decodet;
    bool f_auto_decode_all;
    bool f_lock_txrx;

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseDoubleClickEvent(QMouseEvent * event );
    void mouseMoveEvent(QMouseEvent *);
    
    int last_offset_dsp;
    int last_contr_dsp;
    int s_offset_dsp;
    int s_contr_dsp;
    int diplay_ofset;
    double diplay_contr;

    bool tune_display_thred_busy;
    void DrawAll();

    pthread_t th_t0;
    static void *ThreadDrawAll0(void *);
    pthread_t th_t1;
    static void *ThreadDrawAll1(void *);

	bool f_disp_time;
    QString s_time;
    QString s_ymd;
    QString p_s_time;
    QString p_s_ymd;    
    //bool tune_display_thred_busy;
    void resizeEvent(QResizeEvent *);
    double trans_fac;

    ///rtd//
    QTimer *timer_rtd_;
    bool s_start_stop_rtd_timer;
    bool s_rtd_timer_is_active;
    bool f_dec_busy;
    bool f_priority_disp1;
    int pos_rtd;
    QLine lines_rtd[COUNT_MOUSE_LINES];
    double koef_pos_lines_rtd;
    int s_upd_pos_lines_rtd;
    int s_raw_count;
    //int s_pos_rtd_end;
    bool f_once_sw_timer_rtd_speed;
    bool end_rtd;      
    int disp_ident;
    
    //ft8 3 dec
    bool f_timerft8;
    QTimer *timer_ft8_;
    
    int line_bw;
    int mpos_x_bw ;
    bool s_show_bw;
    bool f_show_bw;
    bool s_show_bwrx;    
    bool s_show_bwtx;
    void leaveEvent(QEvent *event);    
    //void wheelEvent(QWheelEvent *);
};
#endif
