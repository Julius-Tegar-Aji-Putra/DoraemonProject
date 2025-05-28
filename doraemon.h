#ifndef DORAEMON_H
#define DORAEMON_H

#include <GL/glut.h>
#include <stdio.h>

// Function declarations for Doraemon
void initDoraemon();
void drawDoraemon();
void updateDoraemon();
void setKeyState(unsigned char key, bool state);
void setCameraMode(int mode);
int getCameraMode();

// Camera control functions
void specialKeyPressed(int key, int x, int y);
float getCameraX();
float getCameraY();
float getCameraZ();
float getLookX();
float getLookY();
float getLookZ();

// UI & Environment functions
void drawText(const char* text, float x, float y, float r, float g, float b);
void displayControlInfo();
void drawGround();

// Position getters for Doraemon
bool checkCameraCollision(float cameraX, float cameraY, float cameraZ, float* responseX, float* responseY, float* responseZ);
float getDoraemonX();
float getDoraemonY();
float getDoraemonZ();

#endif // DORAEMON_H