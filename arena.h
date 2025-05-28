#ifndef ARENA_H
#define ARENA_H

#include <GL/glut.h>
#include <stdio.h>
#include <vector>

using namespace std;

// Structures for arena
typedef struct {
    float r, g, b;
} Color;

typedef struct {
    float x, z;          
    float width, depth;  
    float height;       
    Color color;         
} Building;

void initArena();
void drawArena(float camX, float camY, float camZ);

bool checkCollision(float x, float y, float z, float radius, float* responseX, float* responseY, float* responseZ);

void setCameraPosition(float x, float y, float z);
void updateCameraLook(float angleX, float angleY);
void getCameraPosition(float* x, float* y, float* z);
void getCameraLookAt(float* x, float* y, float* z);

void drawArenaText(float x, float y, const char* text, float r, float g, float b, void* font);

void setArenaSize(float size);
float getArenaSize();

const vector<Building>& getArenaBuildings();

#endif // ARENA_H