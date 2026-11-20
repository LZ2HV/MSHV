/* MSHV Part from hvutils bcd conversion
 * Copyright 2015 Hrisimir Hristov, LZ2HV
 * May be used under the terms of the GNU General Public License (GPL)
 */ 
unsigned char *to_bcd_(unsigned char *bcd_data, unsigned long long freq, int bcd_len)
{
    int i;
    //unsigned char a;
    // '450'/4-> 5,0;0,4 /
    // '450'/3-> 5,0;x,4 /
    for (i=0; i < bcd_len/2; i++)
    {
        unsigned char a;
        a = freq%10;
        freq /= 10;
        a |= (freq%10)<<4;
        freq /= 10;
        bcd_data[i] = a;
    }
    if (bcd_len&1)
    {
        bcd_data[i] &= 0xf0;
        bcd_data[i] |= freq%10;	// NB: high nibble is left uncleared /
    }
    return bcd_data;
}
unsigned long long from_bcd_(const unsigned char *bcd_data, int bcd_len)
{
    int i;
    unsigned long long f = 0;

    if (bcd_len&1)
        f = bcd_data[bcd_len/2] & 0x0f;

    for (i=(bcd_len/2)-1; i >= 0; i--)
    {
        f *= 10;
        f += bcd_data[i]>>4;
        f *= 10;
        f += bcd_data[i] & 0x0f;
    }
    return f;
}
unsigned char *to_bcd_be_(unsigned char *bcd_data, unsigned long long freq, int bcd_len)
{
	int i;
	//unsigned char a;
	/* '450'/4 -> 0,4;5,0 */
	/* '450'/3 -> 4,5;0,x */
	if (bcd_len&1) {
		bcd_data[bcd_len/2] &= 0x0f;
		bcd_data[bcd_len/2] |= (freq%10)<<4;	/* NB: low nibble is left uncleared */
		freq /= 10;
	}
	for (i=(bcd_len/2)-1; i >= 0; i--) 
	{
		unsigned char a;
		a = freq%10;
		freq /= 10;
		a |= (freq%10)<<4;
		freq /= 10;
		bcd_data[i] = a;
	}
	return bcd_data;
}
unsigned long long from_bcd_be_(const unsigned char *bcd_data, int bcd_len)
{
	int i;
	unsigned long long f = 0;

	for (i=0; i < bcd_len/2; i++) {
		f *= 10;
		f += bcd_data[i]>>4;
		f *= 10;
		f += bcd_data[i] & 0x0f;
	}
	if (bcd_len&1) {
		f *= 10;
		f += bcd_data[bcd_len/2]>>4;
	}
	return f;
}
/*unsigned char *to_be(unsigned char *data,unsigned long long freq,int byte_len)
{
    for (int i = byte_len - 1; i >= 0; i--)
    {
        unsigned char a = freq & 0xFF;
        freq >>= 8;
        data[i] = a;
    }
    return data;
}
unsigned long long from_be(const unsigned char *data,int byte_len)
{
    unsigned long long f = 0;
    for (int i = 0; i < byte_len; i++) f = (f << 8) + data[i];
    return f;
}*/