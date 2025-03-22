﻿#include "../structs/int_types.h"
#include "../structs/animation.h"
#include <vector>
#include <string>
#include <iomanip>

using namespace std;

Animation loadAnimation(u8* animPtr) {
    Animation result;

    int FramesCount = READ_LE_U16(animPtr); animPtr += 2;
    int BonesCount = READ_LE_U16(animPtr); animPtr += 2;
    //var isAITD2 = ((boneCount * 16 + 8) * frameCount + 4) == buffer.Length;

    result.frames.resize(FramesCount);
    for (int i = 0; i < FramesCount; i++)
    {
        auto& frame = result.frames[i];
        frame.timestamp = READ_LE_U16(animPtr); animPtr += 2;
        frame.offset[0] = READ_LE_S16(animPtr); animPtr += 2;
        frame.offset[1] = READ_LE_S16(animPtr); animPtr += 2;
        frame.offset[2] = READ_LE_S16(animPtr); animPtr += 2;

        frame.bones.resize(BonesCount);
        for (int i = 0; i < BonesCount; i++)
        {
            auto& bone = frame.bones[i];
            bone.type = READ_LE_S16(animPtr); animPtr += 2;
            bone.delta[0] = READ_LE_S16(animPtr); animPtr += 2;
            bone.delta[1] = READ_LE_S16(animPtr); animPtr += 2;
            bone.delta[2] = READ_LE_S16(animPtr); animPtr += 2;
        }
    }

    //if (isAITD2)
    //{
    //    boneTransform = buffer.ReadVector(i + 0);
    //    b.Rotate = GetRotation(new Vector3(-boneTransform.x * 360 / 1024.0f, -boneTransform.y * 360 / 1024.0f, -boneTransform.z * 360 / 1024.0f));
    //    i += 8;
    //}

    return result;
}

//for (int bone = 0; bone < boneCount; bone++)
//{
//	Bone b = new Bone();
//	b.Type = buffer.ReadShort(i + 0);
//	Vector3Int boneTransform = buffer.ReadVector(i + 2);
//
//	switch (b.Type)
//	{
//	case 0: //rotate
//		if (!isAITD2)
//		{
//			b.Rotate = GetRotation(new Vector3(-boneTransform.x * 360 / 1024.0f, -boneTransform.y * 360 / 1024.0f, -boneTransform.z * 360 / 1024.0f));
//		}
//		break;
//
//	case 1: //translate
//		b.Position = new Vector3(boneTransform.x / 1000.0f, -boneTransform.y / 1000.0f, boneTransform.z / 1000.0f);
//		break;
//
//	case 2: //scale
//		b.Scale = new Vector3(boneTransform.x / 256.0f + 1.0f, boneTransform.y / 256.0f + 1.0f, boneTransform.z / 256.0f + 1.0f);
//		break;
//	}
//
//	i += 8;
//	if (isAITD2)
//	{
//		boneTransform = buffer.ReadVector(i + 0);
//		b.Rotate = GetRotation(new Vector3(-boneTransform.x * 360 / 1024.0f, -boneTransform.y * 360 / 1024.0f, -boneTransform.z * 360 / 1024.0f));
//		i += 8;
//	}
//
//	f.Bones.Add(b);
//}