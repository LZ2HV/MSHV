/* MSHV Part from hvutils bcd conversion
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */
#ifndef HVUTILS_H
#define HVUTILS_H

unsigned char *to_bcd_(unsigned char *bcd_data, unsigned long long freq, int bcd_len);
unsigned long long from_bcd_(const unsigned char *bcd_data, int bcd_len);
unsigned char *to_bcd_be_(unsigned char *bcd_data, unsigned long long freq, int bcd_len);
unsigned long long from_bcd_be_(const unsigned char *bcd_data, int bcd_len);
//unsigned char *to_be(unsigned char *data,unsigned long long freq,int byte_len);
//unsigned long long from_be(const unsigned char *data,int byte_len);

#endif
