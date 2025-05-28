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
    float x, z;          // Posisi
    float width, depth;  // Ukuran alas
    float height;        // Tinggi
    Color color;         // Warna
} Building;

// Function declarations for arena initialization and rendering
void initArena();
void drawArena(float camX, float camY, float camZ);

// Updated function prototype with Y response
bool checkCollision(float x, float y, float z, float radius, float* responseX, float* responseY, float* responseZ);

// Camera functions (optional - can use if you want to replace Doraemon's camera system)
void setCameraPosition(float x, float y, float z);
void updateCameraLook(float angleX, float angleY);
void getCameraPosition(float* x, float* y, float* z);
void getCameraLookAt(float* x, float* y, float* z);

// Utility functions
void drawArenaText(float x, float y, const char* text);

// Arena configuration
void setArenaSize(float size);
float getArenaSize();

const vector<Building>& getArenaBuildings();

#endif // ARENA_H