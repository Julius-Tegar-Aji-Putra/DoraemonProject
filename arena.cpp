#ifndef ARENA_C
#define ARENA_C

#include "arena.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <GL/glut.h>
extern GLuint texture_tembok_pembatas;
extern GLuint texture_jalan_paving;

// Arena properties
static float arenaSize = 50.0f;
static float wallHeight = 100.0f;

// Camera properties
static float posX = 0.0f;
static float posY = 2.0f;
static float posZ = 15.0f;
static float lookX = 0.0f;
static float lookY = 0.0f;
static float lookZ = 0.0f;
static float angleX = 0.0f;
static float angleY = 0.0f;

// Buildings collection
static std::vector<Building> buildings;

// Warna-warna untuk gedung
static Color colors[] = {
    {0.8f, 0.8f, 0.8f}, 
    {0.7f, 0.7f, 0.8f}, 
    {0.6f, 0.6f, 0.7f}, 
    {0.8f, 0.8f, 0.7f}, 
    {0.7f, 0.8f, 0.7f}  
};

// Fungsi untuk menambahkan gedung ke vektor
static void addBuilding(float x, float z, float width, float depth, float height, Color color) {
    Building b = {x, z, width, depth, height, color};
    buildings.push_back(b);
}

// Fungsi untuk membuat gedung
static void drawBuilding(float x, float z, float width, float depth, float height, Color color, float alpha) {
    glPushMatrix();
    glTranslatef(x, height/2, z);
    glColor4f(color.r, color.g, color.b, alpha);
    
    // Gedung utama
    glPushMatrix();
    glScalef(width, height, depth);
    glutSolidCube(1.0);
    glPopMatrix();
    
    // Garis-garis untuk jendela
    glColor4f(0.1f, 0.1f, 0.2f, alpha);
    float windowSize = 0.5f;
    float spacing = 0.6f;
    
    // Jendela depan
    for (float wx = -width/2 + spacing; wx < width/2; wx += spacing) {
        for (float wy = -height/2 + spacing; wy < height/2; wy += spacing) {
            glPushMatrix();
            glTranslatef(wx, wy, depth/2 + 0.01f);
            glScalef(windowSize, windowSize, 0.01f);
            glutSolidCube(1.0);
            glPopMatrix();
        }
    }
    
    // Jendela belakang
    for (float wx = -width/2 + spacing; wx < width/2; wx += spacing) {
        for (float wy = -height/2 + spacing; wy < height/2; wy += spacing) {
            glPushMatrix();
            glTranslatef(wx, wy, -depth/2 - 0.01f);
            glScalef(windowSize, windowSize, 0.01f);
            glutSolidCube(1.0);
            glPopMatrix();
        }
    }
    
    // Jendela samping kiri
    for (float wz = -depth/2 + spacing; wz < depth/2; wz += spacing) {
        for (float wy = -height/2 + spacing; wy < height/2; wy += spacing) {
            glPushMatrix();
            glTranslatef(-width/2 - 0.01f, wy, wz);
            glScalef(0.01f, windowSize, windowSize);
            glutSolidCube(1.0);
            glPopMatrix();
        }
    }
    
    // Jendela samping kanan
    for (float wz = -depth/2 + spacing; wz < depth/2; wz += spacing) {
        for (float wy = -height/2 + spacing; wy < height/2; wy += spacing) {
            glPushMatrix();
            glTranslatef(width/2 + 0.01f, wy, wz);
            glScalef(0.01f, windowSize, windowSize);
            glutSolidCube(1.0);
            glPopMatrix();
        }
    }
    
    glPopMatrix();
}

// FUNGSI BARU: Untuk mengecek apakah kamera di dalam gedung
static bool isCameraInsideThisBuilding(const Building& building, float camX, float camY, float camZ) {
    float buildingMinX = building.x - building.width / 2.0f;
    float buildingMaxX = building.x + building.width / 2.0f;
    float buildingMinY = 0.0f; // Asumsi gedung mulai dari Y=0
    float buildingMaxY = building.height;
    float buildingMinZ = building.z - building.depth / 2.0f;
    float buildingMaxZ = building.z + building.depth / 2.0f;

    if (camX >= buildingMinX && camX <= buildingMaxX &&
        camY >= buildingMinY && camY <= buildingMaxY && 
        camZ >= buildingMinZ && camZ <= buildingMaxZ) {
        return true;
    }
    return false;
}

// Fungsi untuk menggambar tembok arena
static void drawWalls() {
    float boundarySize = arenaSize / 2.0f;
    float wallThickness = 1.0f; 

    if (texture_tembok_pembatas == 0) { 
        glColor3f(0.5f, 0.5f, 0.5f); 
    } else {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texture_tembok_pembatas); 
        glColor3f(1.0f, 1.0f, 1.0f); 
    }

    float texRepeatX_longWall = (arenaSize + wallThickness * 2.0f) / 5.0f; 
    float texRepeatX_shortWall = arenaSize / 5.0f; 
    float texRepeatY_wall = wallHeight / 5.0f;   

    glNormal3f(0.0f, 0.0f, 0.0f); 

    // Tembok Selatan (Bagian Dalam - Menghadap -Z)
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, boundarySize); 
    glBegin(GL_QUADS);
        glNormal3f(0.0f, 0.0f, -1.0f); // Normal menghadap ke dalam arena (-Z)
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-boundarySize - wallThickness, 0.0f, 0.0f);
        glTexCoord2f(texRepeatX_longWall, 0.0f); glVertex3f(boundarySize + wallThickness, 0.0f, 0.0f);
        glTexCoord2f(texRepeatX_longWall, texRepeatY_wall); glVertex3f(boundarySize + wallThickness, wallHeight, 0.0f);
        glTexCoord2f(0.0f, texRepeatY_wall); glVertex3f(-boundarySize - wallThickness, wallHeight, 0.0f);
    glEnd();
    glPopMatrix();

    // Tembok Utara (Bagian Dalam - Menghadap +Z)
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -boundarySize); 
    glBegin(GL_QUADS);
        glNormal3f(0.0f, 0.0f, 1.0f); 
        glTexCoord2f(0.0f, 0.0f); glVertex3f(boundarySize + wallThickness, 0.0f, 0.0f);
        glTexCoord2f(texRepeatX_longWall, 0.0f); glVertex3f(-boundarySize - wallThickness, 0.0f, 0.0f);
        glTexCoord2f(texRepeatX_longWall, texRepeatY_wall); glVertex3f(-boundarySize - wallThickness, wallHeight, 0.0f);
        glTexCoord2f(0.0f, texRepeatY_wall); glVertex3f(boundarySize + wallThickness, wallHeight, 0.0f);
    glEnd();
    glPopMatrix();

    // Tembok Timur (Bagian Dalam - Menghadap -X)
    glPushMatrix();
    glTranslatef(boundarySize, 0.0f, 0.0f); 
    glBegin(GL_QUADS);
        glNormal3f(-1.0f, 0.0f, 0.0f); 
        glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f, 0.0f, boundarySize + wallThickness);
        glTexCoord2f(texRepeatX_shortWall, 0.0f); glVertex3f(0.0f, 0.0f, -boundarySize - wallThickness);
        glTexCoord2f(texRepeatX_shortWall, texRepeatY_wall); glVertex3f(0.0f, wallHeight, -boundarySize - wallThickness);
        glTexCoord2f(0.0f, texRepeatY_wall); glVertex3f(0.0f, wallHeight, boundarySize + wallThickness);
    glEnd();
    glPopMatrix();
    
    // Tembok Barat (Bagian Dalam - Menghadap +X)
    glPushMatrix();
    glTranslatef(-boundarySize, 0.0f, 0.0f); 
    glBegin(GL_QUADS);
        glNormal3f(1.0f, 0.0f, 0.0f); 
        glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f, 0.0f, -boundarySize - wallThickness);
        glTexCoord2f(texRepeatX_shortWall, 0.0f); glVertex3f(0.0f, 0.0f, boundarySize + wallThickness);
        glTexCoord2f(texRepeatX_shortWall, texRepeatY_wall); glVertex3f(0.0f, wallHeight, boundarySize + wallThickness);
        glTexCoord2f(0.0f, texRepeatY_wall); glVertex3f(0.0f, wallHeight, -boundarySize - wallThickness);
    glEnd();
    glPopMatrix();

    if (texture_tembok_pembatas != 0) {
        glDisable(GL_TEXTURE_2D); // [cite: 66]
        glBindTexture(GL_TEXTURE_2D,0);
    }
}

// Fungsi untuk menggambar jalan
static void drawRoad() {
    glPushMatrix();
    if (texture_jalan_paving != 0) { 
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texture_jalan_paving);
        glColor3f(1.0f, 1.0f, 1.0f); 
    } else {
        glColor3f(0.2f, 0.2f, 0.2f); 
    }
    
    glTranslatef(0.0f, 0.0f, 0.0f); 
    
    float halfArena = arenaSize / 2.0f;
    float texRoadRepeat = arenaSize / 10.0f; 

    glBegin(GL_QUADS);
        glNormal3f(0.0f, 1.0f, 0.0f); 
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-halfArena, 0.0f, -halfArena);
        glTexCoord2f(texRoadRepeat, 0.0f); glVertex3f(halfArena, 0.0f, -halfArena);
        glTexCoord2f(texRoadRepeat, texRoadRepeat); glVertex3f(halfArena, 0.0f, halfArena);
        glTexCoord2f(0.0f, texRoadRepeat); glVertex3f(-halfArena, 0.0f, halfArena);
    glEnd();
    
    glPopMatrix(); 

    if (texture_jalan_paving != 0) {
        glDisable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0); 
    }

    glEnable(GL_LIGHTING); 
    glColor3f(1.0f, 1.0f, 1.0f); 
    
    glPushMatrix();
    glTranslatef(0.0f, 0.01f, 0.0f); 
    
    glPopMatrix(); 
}

const vector<Building>& getArenaBuildings() {
    return buildings;
}

// Prosedur untuk inisialisasi arena
void initArena() {
    buildings.clear();
    
    // Add buildings to the arena
    // Kuadran 1
    addBuilding(5.0f, 5.0f, 5.0f, 5.0f, 20.05f, colors[0]);
    addBuilding(5.0f, 15.0f, 5.0f, 5.0f, 15.35f, colors[1]);
    addBuilding(15.0f, 5.0f, 5.0f, 5.0f, 10.07f, colors[2]);
    addBuilding(15.0f, 15.0f, 5.0f, 5.0f, 15.35f, colors[2]);
    
    // Kuadran 2
    addBuilding(-5.0f, 5.0f, 5.0f, 5.0f, 20.05f, colors[0]);
    addBuilding(-5.0f, 15.0f, 5.0f, 5.0f, 15.35f, colors[1]);
    addBuilding(-15.0f, 5.0f, 5.0f, 5.0f, 10.07f, colors[2]);
    addBuilding(-15.0f, 15.0f, 5.0f, 5.0f, 15.35f, colors[2]);

    // Kuadran 3
    addBuilding(5.0f, -5.0f, 5.0f, 5.0f, 20.05f, colors[0]);
    addBuilding(5.0f, -15.0f, 5.0f, 5.0f, 15.35f, colors[1]);
    addBuilding(15.0f, -5.0f, 5.0f, 5.0f, 10.07f, colors[2]);
    addBuilding(15.0f, -15.0f, 5.0f, 5.0f, 15.35f, colors[2]);

    // Kuadran 4
    addBuilding(-5.0f, -5.0f, 5.0f, 5.0f, 20.05f, colors[0]);
    addBuilding(-5.0f, -15.0f, 5.0f, 5.0f, 15.35f, colors[1]);
    addBuilding(-15.0f, -5.0f, 5.0f, 5.0f, 10.07f, colors[2]);
    addBuilding(-15.0f, -15.0f, 5.0f, 5.0f, 15.35f, colors[2]);
}

// Fungsi untuk menggambar arena
void drawArena(float camX, float camY, float camZ) {
    drawRoad();
    
    drawWalls();
    
    for (size_t i = 0; i < buildings.size(); i++) { 
        Building b = buildings[i]; 
        
        bool cameraIsInside = isCameraInsideThisBuilding(b, camX, camY, camZ);
        
        if (cameraIsInside) {
            glEnable(GL_BLEND); 
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

            drawBuilding(b.x, b.z, b.width, b.depth, b.height, b.color, 0.4f); 

            glDisable(GL_BLEND); 
                                
                                 
        } else {
            drawBuilding(b.x, b.z, b.width, b.depth, b.height, b.color, 1.0f);
        }
    }
}

// fungsi untuk mengecek apakah ada kolisi antara Doraemon dan gedung
bool checkCollision(float x, float y, float z, float radius, float* responseX, float* responseY, float* responseZ) {
    bool collision = false;
    *responseX = 0.0f;
    *responseY = 0.0f;
    *responseZ = 0.0f;
    
    float heightOffset = 1.0f;
    
    bool standingOnSurface = false;
    float surfaceHeight = 0.0f;
    
    for (size_t i = 0; i < buildings.size(); i++) {
        Building b = buildings[i];
        
        float halfWidth = b.width / 2.0f;
        float halfDepth = b.depth / 2.0f;
        
        if (fabs(x - b.x) < halfWidth && fabs(z - b.z) < halfDepth) {
            if (y < b.height + radius + heightOffset && y > b.height - radius) {
                standingOnSurface = true;
                
                if (b.height > surfaceHeight) {
                    surfaceHeight = b.height;
                }
                
                continue;
            }
        }
        

        if (!standingOnSurface || surfaceHeight != b.height) {
            float closestX = fmax(b.x - halfWidth, fmin(x, b.x + halfWidth));
            float closestY = fmax(0, fmin(y, b.height)); 
            float closestZ = fmax(b.z - halfDepth, fmin(z, b.z + halfDepth));
            
            float distanceX = x - closestX;
            float distanceY = y - closestY;
            float distanceZ = z - closestZ;
            
            float distanceSquared = distanceX * distanceX + 
                                   distanceY * distanceY + 
                                   distanceZ * distanceZ;
            
            if (distanceSquared < radius * radius) {
                float distance = sqrt(distanceSquared);
                if (distance > 0) {
                    float pushDistance = radius - distance;
                    
                    float normalX = distanceX / distance;
                    float normalY = distanceY / distance;
                    float normalZ = distanceZ / distance;
                    
                    *responseX += normalX * pushDistance * 1.05f;
                    *responseY += normalY * pushDistance * 1.05f;
                    *responseZ += normalZ * pushDistance * 1.05f;
                }
                collision = true;
            }
        }
    }
    
    float boundarySize = arenaSize / 2;
    float wallRadius = radius + 0.2f;
    
    float wallTop = wallHeight;
    
    if (y < wallTop + radius) {
        // East wall
        if (x + wallRadius > boundarySize) {
            *responseX += (boundarySize - (x + wallRadius));
            collision = true;
        }
        // West wall
        if (x - wallRadius < -boundarySize) {
            *responseX += (-boundarySize - (x - wallRadius));
            collision = true;
        }
        // South wall
        if (z + wallRadius > boundarySize) {
            *responseZ += (boundarySize - (z + wallRadius));
            collision = true;
        }
        // North wall
        if (z - wallRadius < -boundarySize) {
            *responseZ += (-boundarySize - (z - wallRadius));
            collision = true;
        }
    }
    
    if (!standingOnSurface && y < radius + heightOffset) {
        standingOnSurface = true;
        surfaceHeight = 0.0f;
    }
    
    if (standingOnSurface) {
        float targetHeight = surfaceHeight + radius + heightOffset;
        
        *responseY = (targetHeight - y) * 0.5f;
        collision = true;
    }
    
    return collision;
}

// Set camera position
void setCameraPosition(float x, float y, float z) {
    posX = x;
    posY = y;
    posZ = z;
    updateCameraLook(angleX, angleY);
}

// Prosedur untuk mengubah orientasi kamera
void updateCameraLook(float newAngleX, float newAngleY) {
    angleX = newAngleX;
    angleY = newAngleY;
    
    float angleXRad = angleX * 3.14159f / 180.0f;
    float angleYRad = angleY * 3.14159f / 180.0f;
    
    lookX = posX + sin(angleYRad) * cos(angleXRad);
    lookY = posY + sin(angleXRad);
    lookZ = posZ - cos(angleYRad) * cos(angleXRad);
}

// Prosedur untuk mengambil posisi kamera
void getCameraPosition(float* x, float* y, float* z) {
    *x = posX;
    *y = posY;
    *z = posZ;
}

// Prosedur untuk mengambil orientasi kamera
void getCameraLookAt(float* x, float* y, float* z) {
    *x = lookX;
    *y = lookY;
    *z = lookZ;
}

// Fungsi untuk menggambar teks di arena
void drawArenaText(float x, float y, const char* text, float r, float g, float b, void* font) {
    glColor3f(r, g, b);
    
    glRasterPos2f(x, y);
    for (const char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(font, *c);
    }

    glRasterPos2f(x + 1, y); // Offset 1 piksel ke kanan
    for (const char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(font, *c);
    }
}

// Set arena size
void setArenaSize(float size) {
    arenaSize = size;
}

// Get arena size
float getArenaSize() {
    return arenaSize;
}



#endif