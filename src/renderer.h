#pragma once
#include <raylib.h>
#include "structures/floor.h"

Camera3D mainCamera = { 0 };

void setCamera( cameraStruct* camera ) {
	mainCamera.position = {0.0f, 1.0f, -10.0f }; // mainCamera position
	mainCamera.target = { 0.0f, 0.0f, 0.0f };    // mainCamera looking at point
	mainCamera.up = { 0.0f, 1.0f, 0.0f };        // mainCamera up vector (rotation towards target)
	mainCamera.fovy = 45.0f;
	mainCamera.projection = CAMERA_PERSPECTIVE;
}

void renderDebug() {
	Vector3 startPos = { -10.0f, 0.0f, 0.0f };
	Vector3 endPos = { 10.0f, 00.0f, 00.0f };	
	//(Vector3)
	DrawLine3D(startPos, endPos, RED);
}