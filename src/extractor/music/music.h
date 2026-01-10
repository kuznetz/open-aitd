#ifndef _MUSIC_H_
#define _MUSIC_H_

#include "common.h"

extern unsigned int musicChrono;
int initMusicDriver(void);

void playMusic(int musicNumber, char* musicPtr);
int musicUpdate(void *udata, int16 *stream, int len);

void destroyMusicDriver(void);
int fadeMusic(int param1, int param2, int param3);
int isMusicFinished(void);

#endif
