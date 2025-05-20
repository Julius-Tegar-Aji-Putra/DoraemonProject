#ifndef KOIN_H
#define KOIN_H

#include <GL/glut.h>
#include <stdio.h>

// Function declarations for Coin
void initKoin();
void drawKoin();
void updateKoin();

// Getter for coin position
float getKoinX();
float getKoinY();
float getKoinZ();

#endif // KOIN_H