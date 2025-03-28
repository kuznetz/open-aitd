#pragma once
#include <math.h>
#include "../extractor/structs/floor.h"
//#include "../extractor/structs/camera.h"

namespace LegacyCamera {
  
  //0x000-0x100 - cos
  //0x100-0x200 - sin

  typedef struct Vec {
    float x; 
    float y;
    float z;
  } Vec;

  extern s16 cosTable[];

  extern int translateX;
  extern int translateY;
  extern int translateZ;
  extern int transformX;
  extern int transformY;
  extern int transformZ;
  extern int transformXCos;
  extern int transformXSin;
  extern int transformYCos;
  extern int transformYSin;
  extern int transformZCos;
  extern int transformZSin;
  extern bool transformUseX;
  extern bool transformUseY;
  extern bool transformUseZ;
  extern int cameraCenterX;
  extern int cameraCenterY;
  extern int cameraPerspective;
  extern int cameraFovX;
  extern int cameraFovY;

  void setupCameraProjection(int centerX, int centerY, int x, int fovX, int fovY);
  void SetAngleCamera(int x, int y, int z);
  void transformPoint(float* ax, float* bx, float* cx);
  void setupCamera(cameraStruct* pCamera);
  void setupCameraRoom(cameraStruct* pCamera, roomStruct* room);
  Vec projectPoint(int xs, int ys, int zs);

  //void mulMartrix3x3(float* results, float* matrix1, float* matrix2) {
  //    for (int i = 0; i < 9; i++) {
  //        results[i] = 0;
  //    }
  //    for (int i = 0; i < 3; i++)
  //    {
  //        for (int j = 0; j < 3; j++)
  //        {
  //            for (int u = 0; u < 3; u++)
  //            {
  //                //results[i][j] += matrix1[i][u] * matrix2[u][j];
  //                results[i * 3 + j] += matrix1[i * 3 + u] * matrix2[u * 3 + j];
  //            }
  //        }
  //    }
  //}

  //void eulerToMatrix(float* res, cameraStruct* cam)
  //{
  //    float x[] = {
  //        -1,0,0,
  //        0,cosTable[(cam->alpha + 0x100) & 0x3FF] / 32768.0,-cosTable[cam->alpha & 0x3FF] / 32768.0,
  //        0,cosTable[cam->alpha & 0x3FF] / 32768.0,cosTable[(cam->alpha + 0x100) & 0x3FF] / 32768.0,
  //    };
  //    float y[] = {
  //        cosTable[(cam->beta + 0x100) & 0x3FF] / 32768.0,0,-cosTable[cam->beta & 0x3FF] / 32768.0,
  //        0,1,0,
  //        cosTable[cam->beta & 0x3FF] / 32768.0,0,cosTable[(cam->beta + 0x100) & 0x3FF] / 32768.0
  //    };
  //    float z[] = {
  //        cosTable[(cam->gamma + 0x100) & 0x3FF] / 32768.0,-cosTable[cam->gamma & 0x3FF] / 32768.0,0,
  //        cosTable[cam->gamma & 0x3FF] / 32768.0,cosTable[(cam->gamma + 0x100) & 0x3FF] / 32768.0,0,
  //        0,0,1
  //    };
  //    float t[9];
  //    mulMartrix3x3(t, z, x);
  //    mulMartrix3x3(res, t, y);
  //}

}
