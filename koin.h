#ifndef KOIN_H
#define KOIN_H

#include <GL/glut.h>
#include <stdio.h>
#include <vector>

using namespace std;

// Struktur untuk merepresentasikan satu koin
typedef struct {
    float x, y, z;        // Posisi koin
    bool isActive;        // Status apakah koin aktif/sudah diambil
    float rotation;       // Rotasi individu koin
    float yOffset;        // Offset Y untuk animasi naik-turun
    float yDirection;     // Arah gerakan Y
    // Tambahkan properti lain jika perlu, misal ID unik, nilai skor, dll.
} KoinData;

// Function declarations for Coin
void initKoin();
void drawKoin();
void updateKoin();
void resetKoin();

const vector<KoinData>& getAllKoinData();
void setKoinCollected(int index);

#endif // KOIN_H