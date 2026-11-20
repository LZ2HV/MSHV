/* MSHV
 *
 * By Hrisimir Hristov - LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */

#ifndef HVRAWFILTER_H
#define HVRAWFILTER_H

#include <math.h>
#include <string.h>
//#include <stdlib.h> // zaradi malloc
//#include <QtGui>
#define M_LN2_HV	0.69314718055994530942
#define M_PI_HV		3.14159265358979323846

class HvRawFilter
{
public:
    HvRawFilter(int all_gain, double min_down_frq, double max_up_frq, bool type_lhsh_lhpf);
    ~HvRawFilter();

    double band_l(double);
    //double band_r(double);
    void set_rate(double);
    void set_lpf_hpf_parm(double,double);
    //void set_down_up(double,double);
    //void filter_parm(int all_gain, double min_down_frq, double max_up_frq, bool type_lhsh_lhpf);

private:
    typedef struct
    {
        double f1a0_l;
        double f1a1_l;
        double f1a2_l;
        double f1a3_l;
        double f1a4_l;
        double f1x1_l;
        double f1x2_l;
        double f1y1_l;
        double f1y2_l;
        /*double f1a0_r;
        double f1a1_r;
        double f1a2_r;
        double f1a3_r;
        double f1a4_r;
        double f1x1_r;
        double f1x2_r;
        double f1y1_r;
        double f1y2_r;*/

        /*double f2a0_l;
        double f2a1_l;
        double f2a2_l;
        double f2a3_l;
        double f2a4_l;
        double f2x1_l;
        double f2x2_l;
        double f2y1_l;
        double f2y2_l;
        double f2a0_r;
        double f2a1_r;
        double f2a2_r;
        double f2a3_r;
        double f2a4_r;
        double f2x1_r;
        double f2x2_r;
        double f2y1_r;
        double f2y2_r;*/

        double f10a0_l;
        double f10a1_l;
        double f10a2_l;
        double f10a3_l;
        double f10a4_l;
        double f10x1_l;
        double f10x2_l;
        double f10y1_l;
        double f10y2_l;
        /*double f10a0_r;
        double f10a1_r;
        double f10a2_r;
        double f10a3_r;
        double f10a4_r;
        double f10x1_r;
        double f10x2_r;
        double f10y1_r;
        double f10y2_r;*/
        
        /*double f11a0_l;
        double f11a1_l;
        double f11a2_l;
        double f11a3_l;
        double f11a4_l;
        double f11x1_l;
        double f11x2_l;
        double f11y1_l;
        double f11y2_l;
        double f11a0_r;
        double f11a1_r;
        double f11a2_r;
        double f11a3_r;
        double f11a4_r;
        double f11x1_r;
        double f11x2_r;
        double f11y1_r;
        double f11y2_r;*/        
    }
    EQSTATE;

    EQSTATE eq;
    double srate;
    double a0, a1, a2, b0, b1, b2;
    double omega, sn, cs;
    double bandw_LPF, bandw_HPF, alpha;
    double A, beta;
    bool TYPE_LHSH_LHPF;

    double freq1;
    //double freq2;
    double freq10;
    //double freq11;

    double Gain_all;

    void dbGain1(double);
    //void dbGain2(double);
    void dbGain10(double);
    //void dbGain11(double);
    void AlldbGain(double);
    //bool two_pass_filt;
};
#endif









