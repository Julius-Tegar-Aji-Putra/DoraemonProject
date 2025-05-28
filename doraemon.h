#ifndef DORAEMON_H
#define DORAEMON_H

#include <GL/glut.h>
#include <stdio.h>

void initDoraemon();
void resetDoraemonState();
void drawDoraemon();
void updateDoraemon();
void setKeyState(unsigned char key, bool state);
void setCameraMode(int mode);
int getCameraMode();

void specialKeyPressed(int key, int x, int y);
float getCameraX();
float getCameraY();
float getCameraZ();
float getLookX();
float getLookY();
float getLookZ();

void drawText(const char* text, float x, float y,
              float rText, float gText, float bText,         
              float rBg, float gBg, float bBg, float aBg,   
              void* font, float bgPaddingX, float bgPaddingY);
void displayControlInfo();
void drawGround();

bool checkCameraCollision(float cameraX, float cameraY, float cameraZ, float* responseX, float* responseY, float* responseZ);
float getDoraemonX();
float getDoraemonY();
float getDoraemonZ();

#endif // DORAEMON_H