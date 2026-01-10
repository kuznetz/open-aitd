#include "common.h"
#include "opl3.h"
#include "music.h"
#include "music-data.h"
#include "music-commands.h"

#define musicSync 700

int musicVolume = 0x7F;
int currentMusic = -1;
opl3_chip opl;
unsigned char regBDConf = 0xC0;
unsigned char* channelTable;
unsigned char musicParam1 = 0;
u8* currentMusicPtr = NULL;
u8* currentMusicPtr2 = NULL;
u8* currentMusicPtr3 = NULL;
u8 generalVolume = 0;
int musicTimer = 0;
int nextUpdateTimer = 0;
int fadeParam[3];

typedef void(*musicCommandType)(channelTable2Element* entry, int param,u8* ptr);

void sendAdlib(int regIdx, int value)
{
    OPL3_WriteReg(&opl, regIdx, value);
}

void createDefaultChannel(int index)
{
    channelDataTable[index].volumeModifier = 0xFF;
    channelDataTable[index].voiceConfigIndex = 0xFF;
    channelDataTable[index].keyOnFlag = 0x40;
    channelDataTable[index].frequencyAndOctave = 0xFFFF;
    channelDataTable[index].frequencyOffset = 0x9C;
    channelDataTable[index].channelStateFlag = 0xFFFF;
}

void resetChannelFrequency(int channelIdx)
{
    sendAdlib(0xA0+channelIdx,0);
    sendAdlib(0xB0+channelIdx,0);
}

void setupChannelFrequency(int channelIdx, int cl, int dx,int bp)
{
    u16* di;
    u16 frequency;
    u8 frequencyLow;
    u8 frequencyHigh;
    u8 blockNumber;

    if(!(bp&0x8000))
    {
        sendAdlib(0xB0+channelIdx, 0);
    }

    di = globTable[cl&0xF];

    if(bp & 0x80)
    {
        // assert(0);
    }

    if(cl & 0x80)
    {
        dx = 0x40;
    }

    frequency = di[bp&0xFF];

    frequencyLow = frequency&0xFF;

    sendAdlib(0xA0+channelIdx,frequencyLow);

    blockNumber = (cl&0x70)>>2;

    frequencyHigh = ((frequency>>8)&0x3)|blockNumber;

    if(!(dx&0x40))
    {
        frequencyHigh |= 0x20; // set key on
    }

    sendAdlib(0xB0+channelIdx,frequencyHigh);
}

int musicStart(void* dummy)
{
    int i;

    nextUpdateTimer = musicSync;

    sendAdlib(1,0x20);
    sendAdlib(8,0);
    sendAdlib(0xBD,regBDConf);

    for(i=0;i<18;i++)
    {
        sendAdlib(0x60+channelTableMelodic[i],0xFF);
        sendAdlib(0x80+channelTableMelodic[i],0xFF);
    }

    for(i=0;i<9;i++)
    {
        resetChannelFrequency(i);
    }

    for(i=0;i<11;i++)
    {
        createDefaultChannel(i);
    }

    if(!musicParam1)
    {
        resetChannelFrequency(6);
        setupChannelFrequency(6,0,0x40,0);

        resetChannelFrequency(7);
        setupChannelFrequency(7,7,0x40,0);

        resetChannelFrequency(8);
        setupChannelFrequency(8,0,0x40,0);
    }

    return 0;
}

int musicLoad(void* ptr)
{
    int i;
    u8 flag1;
    u8* musicPtr = (u8*)ptr;

    channelTable = channelTableMelodic;

    flag1 = musicPtr[0x3C] & 0xC0;
    musicParam1 = musicPtr[0x3D];

    if(!musicParam1)
    {
        flag1 |= 0x20;
        channelTable = channelTableRythme;
    }

    regBDConf = flag1;

    for(i=0;i<11;i++)
    {
        unsigned long int offset;

        offset = READ_LE_U32(musicPtr + i*4 + 8);

        if(offset)
        {
            channelTable2[i].dataPtr = musicPtr + offset;
        }
        else
        {
            channelTable2[i].dataPtr = NULL;
        }

        channelTable2[i].statusFlags |= 0x40;
    }

    currentMusicPtr = musicPtr + READ_LE_U16(musicPtr + 0x34);

    return 0;
}

int initialialize(void* dummy)
{
    OPL3_Reset(&opl, 44100); //49716
    musicTimer = 0;
    nextUpdateTimer = 0;
    currentMusic = -1;
    regBDConf = 0xC0;
    musicParam1 = 0;
    currentMusicPtr = NULL;
    currentMusicPtr2 = NULL;
    currentMusicPtr3 = NULL;    

    int i;

    for (int i = 0; i < 11; i++)
    {
        channelDataTable[i] = {0xFFFF, 0x40, 0xFF, 0xFF, 0xFF, 0x9C, 0xFFFF};
    }

    for (int i = 0; i < 11; i++)
    {
        channelTable2[i] = { 0, 0, 0x8040,NULL,NULL,0,0,0,0,0,0,0,0x7F,1,1,0x7F,0},
        channelTable2[i].channelIndex = i;
        channelTable2[i].nextChannel = &channelTable3[i];
        channelTable3[i] = { 0, 0, 0x0040,NULL,NULL,0,0,0,0,0,0,0,0x7F,1,1,0x7F,0},
        channelTable3[i].channelIndex = i;
        channelTable3[i].nextChannel = &channelTable2[i];
    }

    for(i=0;i<11;i++)
    {
        channelTable2[i].statusFlags |= 0x20;
        channelTable2[i].nextChannel->statusFlags |= 0x20;
        createDefaultChannel(i);
    }

    musicStart(NULL);

    return 0;
}

void applyDirectFrequency(int index, int param1, int param2, int param3)
{
    if(musicParam1)
    {
        setupChannelFrequency(index,param1,param2,param3);
        return;
    }
    else
    {
        int ah;

        if(index<6)
        {
            setupChannelFrequency(index,param1,param2,param3);
            return;
        }

        if(index==6)
        {
            setupChannelFrequency(index,param1,0x40,param3);
        }
        else if(index==8 && !(param1&0x80))
        {
            int indexBackup = index;
            int param1Backup = param1;

            setupChannelFrequency(8,param1,0x40,param3);
            {
                int al = param1&0x70;

                index = 7;

                param1 &= 0xF;
                param1+=7;

                if(param1 >= 0xC)
                {
                    param1 -= 0xC;

                    if(al != 0x70)
                        al += 0x10;
                }

                setupChannelFrequency( index, param1, 0x40,param3);

            }

            param1 = param1Backup;
            index = indexBackup;
        }

        ah = (~(smallTable[index-6])) & regBDConf;

        sendAdlib(0xBD,ah);

        if(!(param2 & 0x40) && !(param1 & 0x80))
        {
            ah |= smallTable[index-6];

            sendAdlib(0xBD,ah);
        }

        regBDConf = ah;
    }
}

void configChannel(u8 value, u8* data)
{
    if(smallData2[value] != 0xFF)
    {
        sendAdlib(0xC0 + smallData2[value], data[2]);
    }

    sendAdlib(0x60 + value, data[4]); // Attack Rate  Decay Rate
    sendAdlib(0x80 + value, data[5]); // Sustain Level  Release Rate
    sendAdlib(0x20 + value, data[1]); // Tremolo  Vibrato   Sustain   KSR   Frequency Multiplication Factor
    sendAdlib(0xE0 + value, data[3]); //  Waveform Select
}

void changeOuputLevel(u8 value, u8* data,int bp)
{
    int keyScaleLevel;
    int outputLevel;

    if(value == 0xFF)
        return;

    data++;

    outputLevel = (*data)&0x3F;

    outputLevel = 0x3F - ((((outputLevel*bp)*2) + 0x7F)/0xFE);

    ASSERT((outputLevel & 0x3F) == outputLevel);

    keyScaleLevel = data[0]&0xC0;

    sendAdlib(0x40+value,(data[0]&0xC0) | (outputLevel&0x3F));
}

void applyMusicCommandToOPL(channelTable2Element* element2, channelTableElement* element)
{
    char al;
    u16 dx;
    u16 bp;
    u8 operator1;
    u8 operator2;

    if((element2->statusFlags & 0x40) != element->keyOnFlag)
    {
        element->keyOnFlag = element2->statusFlags & 0x40;

        if(element2->statusFlags & 0x40)
        {
            applyDirectFrequency(element2->channelIndex,element2->noteNumber | 0x80, 0x40, element2->frequencyBlock);
            createDefaultChannel(element2->channelIndex);
            return;
        }
    }

    if(element2->statusFlags & 0x40)
        return;

    if((element->channelStateFlag & 1) || (element->channelStateFlag != (element2->statusFlags&0x8000)))
    {
        element->channelStateFlag = element2->statusFlags&0x8000;
        element->volumeModifier = 0xFF;
        element->voiceConfigIndex = 0xFF;
        element->frequencyAndOctave = 0xFFFF;
        element->frequencyOffset = 0x9C;
    }

    operator1 = channelTable[element2->channelIndex*2];
    operator2 = channelTable[(element2->channelIndex*2)+1];

    if(operator1 == 0xFF && operator2 == 0xFF) // do we have an operator ?
        return;

    if(element2->voiceIndex != element->voiceConfigIndex) // change channel main config
    {
        element->voiceConfigIndex = element2->voiceIndex;

        configChannel(operator1,(currentMusicPtr2+0xD*element2->voiceIndex)+1);

        if(operator2 != 0xFF)
        {
            configChannel(operator2,(currentMusicPtr2+0xD*element2->voiceIndex)+7);
        }

        element->volumeModifier = 0xFF;
    }

    // Ouput level handling

    al = element2->volumeMax - element2->volumeMin;

    if(al < 0)
        al = 0;

    if(element->volumeModifier != al)
    {
        int dx;

        element->volumeModifier = al;

        dx = element2->volumeMax;

        if(operator2==0xFF)
        {
            dx = element->volumeModifier;
        }

        changeOuputLevel(operator1,currentMusicPtr2+0xD*element2->voiceIndex,dx);

        if(operator2 != 0xFF)
        {
            changeOuputLevel(operator2,(currentMusicPtr2+0xD*element2->voiceIndex)+6,element->volumeModifier);
        }
    }

    ////

    bp = dx = element2->frequencyBlock;

    if(element2->frequencyBlock != element->frequencyOffset)
    {
        element->frequencyOffset = element2->frequencyBlock;

        if(element2->noteNumber == element->frequencyAndOctave)
        {
            bp |= 0x8000;
        }
    }
    else
    {
        if(element2->noteNumber == element->frequencyAndOctave)
            return;
    }

    element->frequencyAndOctave = element2->noteNumber = element2->noteNumber | 0x8000;

    applyDirectFrequency(element2->channelIndex,element->frequencyAndOctave&0xFF,element2->statusFlags,bp);
}

int update(void* dummy)
{
    int i;
    channelTable2Element* si;

    if(generalVolume & 0xFF)
    {
        return 0;
    }

    for(i=0;i<11;i++)
    {
        currentMusicPtr2 = currentMusicPtr;
        executeMusicCommand(&channelTable2[i]);

        si = &channelTable2[i];
        if(channelTable2[i].statusFlags & 4)
        {
            currentMusicPtr2 = currentMusicPtr3;
            si = channelTable2[i].nextChannel;
            executeMusicCommand(channelTable2[i].nextChannel);
        }

        applyMusicCommandToOPL(si,&channelDataTable[i]);
    }

    return 0;
}

int musicFade(void * param)
{
    int i;
    int cx;
    int si;
    int dx;
    int bp;

    cx = ((int*)param)[0];
    si = ((int*)param)[1];
    dx = ((int*)param)[2];

    bp = si;

    si = -1;

    if(!bp)
        bp = 0x7FF;

    for(i=0;i<11;i++)
    {
        {
            if(channelTable2[i].dataPtr)
            {
                if(dx & 0x100)
                {
                    assert(0);
                }

                if(dx & 0x40)
                {
                    if(!(channelTable2[i].statusFlags&0x40))
                        channelTable2[i].statusFlags|=0x40;
                }

                if(dx & 0x80) // start all
                {
                    channelTable2[i].statusFlags = 0x40;
                    cx &= 0x7F;

                    channelTable2[i].volumeMax = cx;
                    channelTable2[i].targetVolume = cx;

                    channelTable2[i].volumeMin = 0;

                    createDefaultChannel(channelTable2[i].channelIndex);

                    channelTable2[i].statusFlags = 2;
                }

                if(dx & 0x20)
                {
                    assert(0);
                }

                if(dx & 0x2000)
                {
                    assert(0);
                }

                if(dx & 0x8000)
                {
                    channelTable2[i].targetVolume = cx;
                }

                if(dx & 0x1000)
                {
                    assert(0);
                }

                if(dx & 0x10) // still running ?
                {
                    if(!(dx & 0x2000))
                    {
                        if(!(channelTable2[i].statusFlags & 0x40))
                        {
                            if(si < channelTable2[i].noteDurationCounter)
                                si = channelTable2[i].noteDurationCounter;
                        }
                    }
                    else
                    {
                        if(channelTable2[i].volumeMax != cx)
                        {
                            si = 0;
                        }
                    }
                }
            }
        }
    }

    return si;
}

int initMusicDriver(void)
{
    initialialize(NULL);
    return 0;
}

void loadMusic(int param, char* musicPtr)
{
    musicLoad(musicPtr);
    musicStart(NULL);
}

int fadeMusic(int param1, int param2, int param3)
{
    fadeParam[0] = param1;
    fadeParam[1] = param2;
    fadeParam[2] = param3;
    return musicFade(&fadeParam);
}

void playMusic(int musicNumber, char* musicPtr)
{
    if(currentMusic == musicNumber) return;
    
    currentMusic = musicNumber;
    int trackNumber = musicNumber;
    
    currentMusic = musicNumber;
    if(musicNumber>=0)
    {
        //fadeMusic(0,0,0x40);
        if(musicPtr)
        {
            loadMusic(0,musicPtr);
            fadeMusic(musicVolume,0,0x80);
        }
    }
}

int musicUpdate(void *udata, int16 *stream, int len)
{
    int fillStatus = 0;

    while(fillStatus < len)
    {
        int timeBeforNextUpdate = nextUpdateTimer - musicTimer;

        if(timeBeforNextUpdate > (len - fillStatus))
        {
            timeBeforNextUpdate = len-fillStatus;
        }

        if(timeBeforNextUpdate) // generate
        {
            int16_t sample[2];
            for (int i = 0; i < timeBeforNextUpdate; i++) {
                OPL3_GenerateResampled(&opl, sample);
                stream[(fillStatus+i)*2 + 0] = sample[0];
                stream[(fillStatus+i)*2 + 1] = sample[1];
            }
            fillStatus+=timeBeforNextUpdate;
            musicTimer+=timeBeforNextUpdate;
        }

        if(musicTimer == nextUpdateTimer)
        {
            update(NULL);
            nextUpdateTimer += musicSync;
        }
    }

    return 0;
}

void destroyMusicDriver(void)
{
}

int isMusicFinished(void)
{
    int i;
    if (currentMusic == -1 || currentMusicPtr == NULL)
        return 1;
    for (i = 0; i < 11; i++)
    {
        channelTable2Element* channel = &channelTable2[i];
        if (channel->dataPtr != NULL)
        {
            if (!(channel->statusFlags & 0x40))
            {
                if (channel->noteDurationCounter > 0) return 0;
                if (channel->statusFlags & 4)
                {
                    channelTable2Element* next = channel->nextChannel;
                    if (next->noteDurationCounter > 0) return 0;
                }
            }
        }
    }
    return 1;
}