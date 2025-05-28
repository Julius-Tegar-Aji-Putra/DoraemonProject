#ifndef KOIN_H
#define KOIN_H

#include <GL/glut.h>
#include <stdio.h>
#include <vector>

using namespace std;

typedef struct {
    float x, y, z;        // Posisi koin
    bool isActive;        // Status apakah koin aktif/sudah diambil
    float rotation;       // Rotasi individu koin
    float yOffset;        // Offset Y untuk animasi naik-turun
    float yDirection;     // Arah gerakan Y
} KoinData;

void initKoin();
void drawKoin();
void updateKoin();
void resetKoin();

const vector<KoinData>& getAllKoinData();
void setKoinCollected(int index);

#endif // KOIN_H