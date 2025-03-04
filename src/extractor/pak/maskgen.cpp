#include "floor.h"

void fillpoly(s16* datas, int n, unsigned char c);
//extern unsigned char* polyBackBuffer;

void createMasks(cameraStruct* camera)
{
	for (int viewedRoomIdx = 0; viewedRoomIdx < camera->numViewedRooms; viewedRoomIdx++)
	{
		cameraViewedRoomStruct* pcameraViewedRoomData = &cameraDataTable[currentCamera]->viewedRoomTable[viewedRoomIdx];

		char* data2 = room_PtrCamera[currentCamera] + pcameraViewedRoomData->offsetToMask;
		char* data = data2;
		data += 2;

		int numMask = *(s16*)(data2);

		for (int maskIdx = 0; maskIdx < numMask; maskIdx++)
		{
			maskStruct* pDestMask = &g_maskBuffers[viewedRoomIdx][maskIdx];
			pDestMask->mask.fill(0);
			polyBackBuffer = &pDestMask->mask[0];

			char* src = data2 + *(u16*)(data + 2);

			int numMaskZone = *(s16*)(data);

			int minX = 319;
			int maxX = 0;
			int minY = 199;
			int maxY = 0;

			/*if(isBgOverlayRequired( actorPtr->zv.ZVX1 / 10, actorPtr->zv.ZVX2 / 10,
			actorPtr->zv.ZVZ1 / 10, actorPtr->zv.ZVZ2 / 10,
			data+4,
			*(s16*)(data) ))*/
			{
				int numMaskPoly = *(s16*)src;
				src += 2;

				for (int maskPolyIdx = 0; maskPolyIdx < numMaskPoly; maskPolyIdx++)
				{
					int numPoints = *(s16*)(src);
					src += 2;

					memcpy(cameraBuffer, src, numPoints * 4);

					fillpoly((short*)src, numPoints, 0xFF);

					for (int verticeId = 0; verticeId < numPoints; verticeId++)
					{
						short verticeX = *(short*)(src + verticeId * 4 + 0);
						short verticeY = *(short*)(src + verticeId * 4 + 2);

						minX = std::min<int>(minX, verticeX);
						minY = std::min<int>(minY, verticeY);
						maxX = std::max<int>(maxX, verticeX);
						maxY = std::max<int>(maxY, verticeY);
					}

					src += numPoints * 4;
					//drawBgOverlaySub2(param);
				}

				//      blitOverlay(src);

				polyBackBuffer = nullptr;

			}

			osystem_createMask(pDestMask->mask, viewedRoomIdx, maskIdx, (unsigned char*)aux, minX - 1, minY - 1, maxX + 1, maxY + 1);

			int numOverlay = *(s16*)(data);
			data += 2;
			data += ((numOverlay * 4) + 1) * 2;
		}

		/*		unsigned char* pViewedRoomMask = g_MaskPtr + READ_LE_U32(g_MaskPtr + i*4);

		for(int j=0; j<pRoomView->numMask; j++)
		{
		unsigned char* pMaskData = pViewedRoomMask + READ_LE_U32(pViewedRoomMask + j*4);

		maskStruct* pDestMask = &g_maskBuffers[i][j];

		memset(pDestMask->mask, 0, 320*200);

		pDestMask->x1 = READ_LE_U16(pMaskData);
		pMaskData += 2;
		pDestMask->y1 = READ_LE_U16(pMaskData);
		pMaskData += 2;
		pDestMask->x2 = READ_LE_U16(pMaskData);
		pMaskData += 2;
		pDestMask->y2 = READ_LE_U16(pMaskData);
		pMaskData += 2;
		pDestMask->deltaX = READ_LE_U16(pMaskData);
		pMaskData += 2;
		pDestMask->deltaY = READ_LE_U16(pMaskData);
		pMaskData += 2;

		assert(pDestMask->deltaX == pDestMask->x2 - pDestMask->x1 + 1);
		assert(pDestMask->deltaY == pDestMask->y2 - pDestMask->y1 + 1);

		for(int k=0; k<pDestMask->deltaY; k++)
		{
		u16 uNumEntryForLine = READ_LE_U16(pMaskData);
		pMaskData += 2;

		unsigned char* pDestBuffer = pDestMask->mask;
		unsigned char* pSourceBuffer = (unsigned char*)aux;

		int offset = pDestMask->x1 + pDestMask->y1 * 320 + k * 320;

		for(int l=0; l<uNumEntryForLine; l++)
		{
		unsigned char uNumSkip = *(pMaskData++);
		unsigned char uNumCopy = *(pMaskData++);

		offset += uNumSkip;

		for(int m=0; m<uNumCopy; m++)
		{
		pDestBuffer[offset] = 0xFF;
		offset++;
		}
		}
		}

		osystem_createMask(pDestMask->mask, i, j, (unsigned char*)aux, pDestMask->x1, pDestMask->y1, pDestMask->x2, pDestMask->y2);
		}*/
	}

	polyBackBuffer = NULL;
}