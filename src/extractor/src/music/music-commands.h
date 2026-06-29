#pragma once
#include "common.h"
#include "music-data.h"

typedef void(*musicCommandType)(channelTable2Element* entry, int param,u8* ptr);
extern musicCommandType musicCommandTable[10];
void executeMusicCommand(channelTable2Element* entry);