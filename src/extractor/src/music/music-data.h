#ifndef _MUSIC_STRUCT_H_
#define _MUSIC_STRUCT_H_

#include "common.h"

struct channelTableElement
{
    u16 frequencyAndOctave; // var0: frequency and octave (likely a combined value)
    u16 keyOnFlag;        // var2: note enable flag (Key On) (0x40 = off, 0 = on)
    u8 voiceConfigIndex;  // var4: instrument configuration index (voice/opll)
    u8 volumeModifier;    // var5: volume modifier (0xFF = no change)
    u8 var6;              // var6: (unknown purpose)
    u8 frequencyOffset;   // var7: frequency/block offset
    u16 channelStateFlag; // var8: channel state flags (bit 0 = changed, bit 15 from var4)
};
typedef struct channelTableElement channelTableElement;

struct channelTable2Element
{
    u16 channelIndex; // index: OPL channel index (0-10)
    // var2: pointer to a related channel (for polyphony?)
    struct channelTable2Element *nextChannel;
    // var4: status flags (bits: 0x40 = muted, 0x20 = paused, 0x02 = start)
    u16 statusFlags;
    u8* dataPtr; // dataPtr: pointer to track data (notes, commands)
    u8* commandPtr; // commandPtr: current position in track
    s16 delayCounter; // varE: delay counter until next note
    u16 delayResetValue; // var10: initial delay value
    u8 voiceIndex; // var12: voice/instrument index
    u16 baseNotePitch; // var13: base note pitch/tone
    u16 noteNumber; // var15: current note number (MIDI-like)
    u8 frequencyBlock; // var17: frequency block (octave + tone)
    u16 noteDurationCounter; // var18: note duration counter
    u8 targetVolume; // var1A: target volume
    u8 currentVolume; // var1B: current volume (likely)
    u8 volumeFadeSpeed; // var1C: volume fade speed
    u8 volumeMax; // var1D: maximum volume
    u8 volumeMin; // var1E: minimum volume
};
typedef struct channelTable2Element channelTable2Element;

extern channelTableElement channelDataTable[11];
extern channelTable2Element channelTable3[11];
extern channelTable2Element channelTable2[11];
extern unsigned char channelTableMelodic[];
extern unsigned char channelTableRythme[];
extern u16 globTableEntry[300];
extern u16* globTable[13];
extern unsigned char smallData2[];
extern u8 smallTable[];

#endif