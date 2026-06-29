#include "music-commands.h"

void commandNop(channelTable2Element* entry, int param, u8* ptr)
{
}

void commandSetChannelActive(channelTable2Element* entry, int param, u8* ptr)
{
    entry->statusFlags |= 2;

    if(entry->statusFlags & 0x20)
    {
        return;
    }

    entry->statusFlags |= 0x40;

    if(!(entry->statusFlags & 0x8000))
    {
        return;
    }

    entry->nextChannel->statusFlags &= 0xFFFB;
}

void commandSetDelay(channelTable2Element* entry, int param, u8* ptr)
{
    u16 ax;

    ax = READ_LE_U16(ptr-1);

    entry->delayResetValue = entry->delayCounter = ax + entry->baseNotePitch;

    entry->commandPtr++;
}

void commandSetNoteNumber(channelTable2Element* entry, int param, u8* ptr)
{
    entry->noteDurationCounter++;
    entry->noteNumber = param;
}

void commandSetVoiceIndex(channelTable2Element* entry, int param, u8* ptr)
{
    entry->voiceIndex = param;
}

void commandSetVolumeMin(channelTable2Element* entry, int param, u8* ptr)
{
    entry->volumeMin = param;
}

void commandSetFrequencyBlock(channelTable2Element* entry, int param, u8* ptr)
{
    entry->frequencyBlock = param;
}

void commandUnknown6(channelTable2Element* entry, int param, u8* ptr)
{
    assert(0);
}

musicCommandType musicCommandTable[10] = 
{
    commandSetChannelActive,
    commandSetDelay,
    commandSetNoteNumber,
    commandSetVoiceIndex,
    commandSetVolumeMin,
    commandSetFrequencyBlock,
    commandUnknown6,
    commandNop,
    commandNop,
    commandNop,
};

void executeMusicCommand(channelTable2Element* entry)
{
    u16 opcode;

    if(entry->statusFlags & 0x40) return;

    if(entry->statusFlags & 0x02) // start channel
    {
        entry->commandPtr = entry->dataPtr;
        entry->statusFlags &= 0xFFFD;
        entry->noteDurationCounter = 0;
    }
    else
    {
        if(entry->targetVolume != entry->volumeMax)
        {
            assert(0);
        }

        entry->delayCounter--; // voice delay

        if(entry->delayCounter<=0)
        {
            entry->delayCounter = entry->delayResetValue;
        }
        else
        {
            return;
        }
    }

    do
    {
        opcode = READ_LE_U16(entry->commandPtr);
        entry->commandPtr+=2;

        ASSERT(musicCommandTable[opcode&0x7F]);
        ASSERT((opcode&0x7F)<=10);

        musicCommandTable[opcode&0x7F](entry,opcode>>8,entry->commandPtr);
    } while (!(opcode&0x80));
}