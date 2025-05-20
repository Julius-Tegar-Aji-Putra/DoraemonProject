#ifndef KOIN_C
#define KOIN_C

#include "koin.h"
#include <math.h>
#include <GL/glut.h>
#include <stdio.h>

// Variabel untuk koin
static float coinX = 2.0, coinY = 0.0, coinZ = 0.0;  // Posisi koin (di sebelah Doraemon)
static float coinRotation = 0.0;                     // Rotasi koin
static float coinYOffset = 0.0;                      // Offset Y untuk animasi naik-turun
static float coinYDirection = 1.0f;                  // Arah gerakan naik-turun
static float coinYSpeed = 0.000055;                  // Kecepatan naik-turun
static float coinYMax = 0.5;                         // Ketinggian maksimum

// Fungsi untuk menggambar koin
void drawKoin() {
    glPushMatrix();
    
    // Terapkan rotasi pada koin
    glRotatef(coinRotation, 0.0f, 1.0f, 0.0f);
    
    // Gambar koin (silinder pipih berwarna emas)
    glColor3f(1.0f, 0.84f, 0.0f);  // Warna emas
    
    // Gambar silinder pipih
    GLUquadricObj *cylinder = gluNewQuadric();
    gluQuadricDrawStyle(cylinder, GLU_FILL);
    
    // Terapkan rotasi agar koin berdiri tegak
    glRotatef(270.0f, 0.0f, 1.0f, 0.0f);
    
    // Ukuran koin sesuai ukuran body Doraemon
    float coinRadius = 0.5f;     // Radius koin
    float coinThickness = 0.1f;  // Ketebalan koin
    
    // Gambar silinder pipih
    gluCylinder(cylinder, coinRadius, coinRadius, coinThickness, 30, 2);
    
    // Gambar tutup atas
    gluDisk(cylinder, 0.0f, coinRadius, 30, 1);
    
    // Gambar tutup bawah
    glTranslatef(0.0f, 0.0f, coinThickness);
    gluDisk(cylinder, 0.0f, coinRadius, 30, 1);
    
    // Tambahkan detail koin (garis tengah)
    glColor3f(0.8f, 0.7f, 0.0f);  // Warna emas lebih gelap untuk detail
    glTranslatef(0.0f, 0.0f, -coinThickness/2);
    gluDisk(cylinder, coinRadius-0.1f, coinRadius, 30, 1);
    
    gluDeleteQuadric(cylinder);
    
    glPopMatrix();
}

// Function to update coin (called from idle)
void updateKoin() {
    // Rotasi koin
    coinRotation += 0.35f;  // Kecepatan rotasi pelan
    if (coinRotation > 360.0f) {
        coinRotation -= 360.0f;
    }

    // Animasi naik-turun koin
    coinYOffset += coinYSpeed * coinYDirection;
    if (coinYOffset > coinYMax || coinYOffset < 0.0f) {
        coinYDirection *= -1.0f;  // Balik arah
    }
}

// Function to initialize coin
void initKoin() {
    coinX = 2.0;
    coinY = 0.0;
    coinZ = 0.0;
    coinRotation = 0.0;
    coinYOffset = 0.0;
    coinYDirection = 1.0f;
    coinYSpeed = 0.000055;
    coinYMax = 0.5;
}

// Getter for coin position
float getKoinX() {
    return coinX;
}

float getKoinY() {
    return coinY + coinYOffset;
}

float getKoinZ() {
    return coinZ;
}

#endif