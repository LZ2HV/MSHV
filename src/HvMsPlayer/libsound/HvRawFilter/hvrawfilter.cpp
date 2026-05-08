/* MSHV RawFilter
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#include "hvrawfilter.h"
//#include <QtGui>

HvRawFilter::HvRawFilter(int all_gain, double min_down_frq, double max_up_frq, bool type_lhsh_lhpf)
{
    Gain_all = 1.00;

    a0 = 0.00;
    a1 = 0.00;
    a2 = 0.00;
    b0 = 0.00;
    b1 = 0.00;
    b2 = 0.00;
 
    memset(&eq,0,sizeof(EQSTATE));//2.12 0.0 or 0 ?  malloc(sizeof(EQSTATE));

    srate = 0.0;//fiktivno posle se smenia ot vzavisimost ot GenMessage::testwave() set_rate(double)
    A = pow(10, 0.00/40); //dbGine = 0 za 0 db (0/40)
    beta = sqrt(A + A);
    TYPE_LHSH_LHPF = type_lhsh_lhpf;//TX true=LSH&HSH, RX false=LPF&HPF

    bandw_LPF = 1.0;//2.76.3 Q factor=0.707 //=1.0 v1.15 za ravna harakteristika
    bandw_HPF = 1.0;//2.76.3 Q factor=0.707 //=1.0 v1.15 za ravna harakteristika
    alpha = 1.0;

    //two_pass_filt = false;
    freq1 = min_down_frq;//62.0;//62 - 100.0
    //freq2 = min_down_frq;
    freq10 = max_up_frq;//10000.0;// 13000.0
    //freq11 = max_up_frq;//10000.0;// 13000.0
    dbGain1((double)(0 - 50)/4);//45hz
    //dbGain2((double)(0 - 50)/4);//45hz
    dbGain10((double)(0 - 50)/4);//13000hz
    //dbGain11((double)(0 - 50)/4);//13000hz
    AlldbGain((double)(all_gain - 50)/4);//AlldbGain11((double)(45 - 50)/4);//gine 45 zaradi windows
    //qDebug()<<"1two_pass_filt"<<two_pass_filt;
}
HvRawFilter::~HvRawFilter()
{}
void HvRawFilter::set_rate(double rate)
{
    //qDebug()<<"RATE"<<rate;
    srate = rate;
    dbGain1((double)(0 - 50)/4);
    //dbGain2((double)(0 - 50)/4);
    dbGain10((double)(0 - 50)/4);
    //dbGain11((double)(0 - 50)/4);
}
void HvRawFilter::set_lpf_hpf_parm(double l,double h)//2.76.3
{
    bandw_LPF = l;//2.76.3 za ravna harakteristika
    bandw_HPF = h;//2.76.3 za ravna harakteristika	
    dbGain1((double)(0 - 50)/4);
    //dbGain2((double)(0 - 50)/4);
    dbGain10((double)(0 - 50)/4);
    //dbGain11((double)(0 - 50)/4);    
}
/*void HvRawFilter::set_down_up(double min_down_frq,double max_up_frq)
{
	freq1 = min_down_frq;
    freq10 = max_up_frq;
    dbGain1((double)(0 - 50)/4);
    dbGain10((double)(0 - 50)/4);
}*/
/*
void HvRawFilter::filter_parm(int all_gain, double min_down_frq, double max_up_frq, bool type_lhsh_lhpf)
{
    two_pass_filt = true;
    TYPE_LHSH_LHPF = type_lhsh_lhpf;
    freq1 = min_down_frq;// - 500;
    //freq2 = min_down_frq;
    freq10 = max_up_frq;//10000.0;// 13000.0
    freq11 = max_up_frq + 300;
    AlldbGain11((double)(all_gain - 50)/4);
    //qDebug()<<"2two_pass_filt"<<two_pass_filt;
}
*/
void HvRawFilter::dbGain1(double val)
{
    A = pow(10, val/40);
    beta = sqrt(A + A);
    omega = (double)2 * M_PI_HV * freq1 /(double)srate;
    sn = sin(omega);
    cs = cos(omega);

    if (TYPE_LHSH_LHPF)
    {
        //LSH: /* Low shelf filter */
        b0 = A * ((A + 1) - (A - 1) * cs + beta * sn);
        b1 = 2 * A * ((A - 1) - (A + 1) * cs);
        b2 = A * ((A + 1) - (A - 1) * cs - beta * sn);
        a0 = (A + 1) + (A - 1) * cs + beta * sn;
        a1 = -2 * ((A - 1) + (A + 1) * cs);
        a2 = (A + 1) + (A - 1) * cs - beta * sn;
    }
    else
    {
        //HPF: /* High pass filter */
        alpha = sn/(2.0*bandw_HPF);
        b0 = (1 + cs)/2.0;
        b1 = -(1 + cs);
        b2 = (1 + cs)/2.0;
        a0 = 1 + alpha;
        a1 = -2 * cs;
        a2 = 1 - alpha;
        //NOTCH:
        /*alpha = sn/(2.0*bandw_HPF);
        b0 = 1;
        b1 = -2 * cs;
        b2 = 1;
        a0 = 1 + alpha;
        a1 = -2 * cs;
        a2 = 1 - alpha;*/
    }

    /*eq.f1a0_l = eq.f1a0_r = b0 /a0;
    eq.f1a1_l = eq.f1a1_r = b1 /a0;
    eq.f1a2_l = eq.f1a2_r = b2 /a0;
    eq.f1a3_l = eq.f1a3_r = a1 /a0;
    eq.f1a4_l = eq.f1a4_r = a2 /a0;
    eq.f1x1_l = eq.f1x2_l = eq.f1x1_r = eq.f1x2_r = 0;
    eq.f1y1_l = eq.f1y2_l = eq.f1y1_r = eq.f1y2_r = 0;*/
    
    eq.f1a0_l = b0 /a0;
    eq.f1a1_l = b1 /a0;
    eq.f1a2_l = b2 /a0;
    eq.f1a3_l = a1 /a0;
    eq.f1a4_l = a2 /a0;
    eq.f1x1_l = eq.f1x2_l = 0;
    eq.f1y1_l = eq.f1y2_l = 0;
}
/*
void HvRawFilter::dbGain2(double val)
{
    A = pow(10, val/40);
    beta = sqrt(A + A);
    omega = (double)2 * M_PI_HV * freq2 /(double)srate;
    sn = sin(omega);
    cs = cos(omega);

    if (TYPE_LHSH_LHPF)
    {
        //LSH: // Low shelf filter /
        b0 = A * ((A + 1) - (A - 1) * cs + beta * sn);
        b1 = 2 * A * ((A - 1) - (A + 1) * cs);
        b2 = A * ((A + 1) - (A - 1) * cs - beta * sn);
        a0 = (A + 1) + (A - 1) * cs + beta * sn;
        a1 = -2 * ((A - 1) + (A + 1) * cs);
        a2 = (A + 1) + (A - 1) * cs - beta * sn;
    }
    else
    {
        //HPF: // High pass filter
        alpha = sn/(2.0*bandw_HPF);
        b0 = (1 + cs)/2.0;
        b1 = -(1 + cs);
        b2 = (1 + cs)/2.0;
        a0 = 1 + alpha;
        a1 = -2 * cs;
        a2 = 1 - alpha;
    }

    eq.f2a0_l = eq.f2a0_r = b0 /a0;
    eq.f2a1_l = eq.f2a1_r = b1 /a0;
    eq.f2a2_l = eq.f2a2_r = b2 /a0;
    eq.f2a3_l = eq.f2a3_r = a1 /a0;
    eq.f2a4_l = eq.f2a4_r = a2 /a0;
    eq.f2x1_l = eq.f2x2_l = eq.f2x1_r = eq.f2x2_r = 0;
    eq.f2y1_l = eq.f2y2_l = eq.f2y1_r = eq.f2y2_r = 0;
}
*/
void HvRawFilter::dbGain10(double val)
{
    A = pow(10, val/40);
    beta = sqrt(A + A);
    omega = (double)2 * M_PI_HV * freq10 /(double)srate;
    sn = sin(omega);
    cs = cos(omega);

    if (TYPE_LHSH_LHPF)
    {
        //HSH: /* High shelf filter */
        b0 = A * ((A + 1) + (A - 1) * cs + beta * sn);
        b1 = -2 * A * ((A - 1) + (A + 1) * cs);
        b2 = A * ((A + 1) + (A - 1) * cs - beta * sn);
        a0 = (A + 1) - (A - 1) * cs + beta * sn;
        a1 = 2 * ((A - 1) - (A + 1) * cs);
        a2 = (A + 1) - (A - 1) * cs - beta * sn;
    }
    else
    {
        //LPF: /* low pass filter */
        alpha = sn/(2.0*bandw_LPF);
        b0 = (1 - cs)/2.0;
        b1 = 1 - cs;
        b2 = (1 - cs)/2.0;
        a0 = 1 + alpha;
        a1 = -2 * cs;
        a2 = 1 - alpha;
        //NOTCH:
        /*alpha = sn/(2.0*bandw_HPF);
        b0 = 1;
        b1 = -2 * cs;
        b2 = 1;
        a0 = 1 + alpha;
        a1 = -2 * cs;
        a2 = 1 - alpha;*/
    }

    /*eq.f10a0_l = eq.f10a0_r = b0 /a0;
    eq.f10a1_l = eq.f10a1_r = b1 /a0;
    eq.f10a2_l = eq.f10a2_r = b2 /a0;
    eq.f10a3_l = eq.f10a3_r = a1 /a0;
    eq.f10a4_l = eq.f10a4_r = a2 /a0;
    eq.f10x1_l = eq.f10x2_l = eq.f10x1_r = eq.f10x2_r = 0;
    eq.f10y1_l = eq.f10y2_l = eq.f10y1_r = eq.f10y2_r = 0;*/
    
    eq.f10a0_l = b0 /a0;
    eq.f10a1_l = b1 /a0;
    eq.f10a2_l = b2 /a0;
    eq.f10a3_l = a1 /a0;
    eq.f10a4_l = a2 /a0;
    eq.f10x1_l = eq.f10x2_l = 0;
    eq.f10y1_l = eq.f10y2_l = 0;
}
/*
void HvRawFilter::dbGain11(double val)
{
    A = pow(10, val/40);
    beta = sqrt(A + A);
    omega = (double)2 * M_PI_HV * freq11 /(double)srate;
    sn = sin(omega);
    cs = cos(omega);

    if (TYPE_LHSH_LHPF)
    {
        //HSH:  High shelf filter 
        b0 = A * ((A + 1) + (A - 1) * cs + beta * sn);
        b1 = -2 * A * ((A - 1) + (A + 1) * cs);
        b2 = A * ((A + 1) + (A - 1) * cs - beta * sn);
        a0 = (A + 1) - (A - 1) * cs + beta * sn;
        a1 = 2 * ((A - 1) - (A + 1) * cs);
        a2 = (A + 1) - (A - 1) * cs - beta * sn;
    }
    else
    {
        //LPF:  low pass filter 
        alpha = sn/(2.4*bandw_LPF);
        //alpha = sn/(1.5);
        b0 = (1 - cs)/2.0;
        b1 = 1 - cs;
        b2 = (1 - cs)/2.0;
        a0 = 1 + alpha;
        a1 = -2 * cs;
        a2 = 1 - alpha;
        
        //NOTCH:
        //alpha = sn/(2.0*bandw_HPF);
        //b0 = 1;
        //b1 = -2 * cs;
        //b2 = 1;
        //a0 = 1 + alpha;
        //a1 = -2 * cs;
        //a2 = 1 - alpha;
    }

    eq.f11a0_l = eq.f11a0_r = b0 /a0;
    eq.f11a1_l = eq.f11a1_r = b1 /a0;
    eq.f11a2_l = eq.f11a2_r = b2 /a0;
    eq.f11a3_l = eq.f11a3_r = a1 /a0;
    eq.f11a4_l = eq.f11a4_r = a2 /a0;
    eq.f11x1_l = eq.f11x2_l = eq.f11x1_r = eq.f11x2_r = 0;
    eq.f11y1_l = eq.f11y2_l = eq.f11y1_r = eq.f11y2_r = 0;
}
*/
void HvRawFilter::AlldbGain(double val)
{
    Gain_all = pow(10, val/20);	// +- 12.5db
    //qDebug()<<Gain_all;//49=0.971628
}
double HvRawFilter::band_l(double sample)
{
    double out1, out10, sl;//out2, out11,
    out1 = eq.f1a0_l * sample + eq.f1a1_l * eq.f1x1_l + eq.f1a2_l * eq.f1x2_l - eq.f1a3_l * eq.f1y1_l - eq.f1a4_l * eq.f1y2_l;
    eq.f1x2_l = eq.f1x1_l;
    eq.f1x1_l = sample;
    eq.f1y2_l = eq.f1y1_l;
    eq.f1y1_l = out1;

    /*if (two_pass_filt)
    {
        out2 = eq.f2a0_l * out1 + eq.f2a1_l * eq.f2x1_l + eq.f2a2_l * eq.f2x2_l - eq.f2a3_l * eq.f2y1_l - eq.f2a4_l * eq.f2y2_l;
        eq.f2x2_l = eq.f2x1_l;
        eq.f2x1_l = out1;
        eq.f2y2_l = eq.f2y1_l;
        eq.f2y1_l = out2;
    }
    else
        out2 = out1;*/

    out10 = eq.f10a0_l * out1 + eq.f10a1_l * eq.f10x1_l + eq.f10a2_l * eq.f10x2_l - eq.f10a3_l * eq.f10y1_l - eq.f10a4_l * eq.f10y2_l;
    eq.f10x2_l = eq.f10x1_l;
    eq.f10x1_l = out1;
    eq.f10y2_l = eq.f10y1_l;
    eq.f10y1_l = out10;

    /*if (two_pass_filt)
    {
        out11 = eq.f11a0_l * out10 + eq.f11a1_l * eq.f11x1_l + eq.f11a2_l * eq.f11x2_l - eq.f11a3_l * eq.f11y1_l - eq.f11a4_l * eq.f11y2_l;
        eq.f11x2_l = eq.f11x1_l;
        eq.f11x1_l = out10;
        eq.f11y2_l = eq.f11y1_l;
        eq.f11y1_l = out11;
    }
    else
    	out11 = out10;*/

    sl = out10*Gain_all;
    // hv limiter max +32767 min -32768 <- !!!! 8 hi for 16bit*/
    /*if (sl > 32766)
        sl = 32766;
    if (sl < -32767)
        sl = -32767;*/

    return sl;
}
/*double HvRawFilter::band_r(double sample)
{
    double out1, out10, sl;//out2, out11, 
    out1 = eq.f1a0_r * sample + eq.f1a1_r * eq.f1x1_r + eq.f1a2_r * eq.f1x2_r - eq.f1a3_r * eq.f1y1_r - eq.f1a4_r * eq.f1y2_r;
    eq.f1x2_r = eq.f1x1_r;
    eq.f1x1_r = sample;
    eq.f1y2_r = eq.f1y1_r;
    eq.f1y1_r = out1;

    if (two_pass_filt)
    {
        out2 = eq.f2a0_r * out1 + eq.f2a1_r * eq.f2x1_r + eq.f2a2_r * eq.f2x2_r - eq.f2a3_r * eq.f2y1_r - eq.f2a4_r * eq.f2y2_r;
        eq.f2x2_r = eq.f2x1_r;
        eq.f2x1_r = out1;
        eq.f2y2_r = eq.f2y1_r;
        eq.f2y1_r = out2;
    }
    else
        out2 = out1;

    out10 = eq.f10a0_r * out1 + eq.f10a1_r * eq.f10x1_r + eq.f10a2_r * eq.f10x2_r - eq.f10a3_r * eq.f10y1_r - eq.f10a4_r * eq.f10y2_r;
    eq.f10x2_r = eq.f10x1_r;
    eq.f10x1_r = out1;
    eq.f10y2_r = eq.f10y1_r;
    eq.f10y1_r = out10;

    if (two_pass_filt)
    {
        out11 = eq.f11a0_r * out10 + eq.f11a1_r * eq.f11x1_r + eq.f11a2_r * eq.f11x2_r - eq.f11a3_r * eq.f11y1_r - eq.f11a4_r * eq.f11y2_r;
        eq.f11x2_r = eq.f11x1_r;
        eq.f11x1_r = out10;
        eq.f11y2_r = eq.f11y1_r;
        eq.f11y1_r = out11;
    }
    else
    	out11 = out10;

    sl = out10*Gain_all;
    // hv limiter max +32767 min -32768 <- !!!! 8 hi for 16bit
    if (sl > 32766)
        sl = 32766;
    if (sl < -32767)
        sl = -32767;

    return sl;
}*/







