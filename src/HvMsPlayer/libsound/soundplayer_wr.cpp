/* MPEG/WAVE Sound library

   (C) 1997 by Jung woo-jae */

// Soundplayer.cc
// Superclass of Rawplayer and Rawtofile
// It's used for set player of Mpegtoraw & Wavetoraw

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mpegsound.h"

/*********************/
/* Soundplayer class */
/*********************/
Soundplayer_wr::~Soundplayer_wr()
{
  // Nothing...
}
void Soundplayer_wr::abort(void)
{
  // Nothing....
}
bool Soundplayer_wr::resetsoundtype(void)
{
  return true;
}
int Soundplayer_wr::getblocksize(void)
{
  return 1024; // Default value
}
/*
int Soundplayer_win::fix_samplesize(void *buffer, int size)
{
    buffer=buffer;
    return size;
}
*/