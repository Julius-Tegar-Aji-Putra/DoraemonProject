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
    {0.8f, 0.8f, 0.8f}, // Abu-abu muda
    {0.7f, 0.7f, 0.8f}, // Abu-abu kebiruan
    {0.6f, 0.6f, 0.7f}, // Abu-abu tua
    {0.8f, 0.8f, 0.7f}, // Cream
    {0.7f, 0.8f, 0.7f}  // Abu-abu kehijauan
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
    // Anda sudah punya wallHeight dari variabel statis di arena.cpp
    // static float wallHeight = 100.0f; (jika ini definisi Anda)

    if (texture_tembok_pembatas == 0) { // Jika tekstur gagal dimuat, gambar warna solid saja
        glColor3f(0.5f, 0.5f, 0.5f); // Warna fallback
    } else {
        glEnable(GL_TEXTURE_2D); // [cite: 59]
        glBindTexture(GL_TEXTURE_2D, texture_tembok_pembatas); // [cite: 59]
        glColor3f(1.0f, 1.0f, 1.0f); // Warna putih agar warna asli tekstur muncul [cite: 60]
    }

    // Definisikan berapa kali tekstur ingin diulang pada permukaan tembok
    // Sesuaikan nilai ini untuk mendapatkan skala tekstur yang diinginkan
    float texRepeatX_longWall = (arenaSize + wallThickness * 2.0f) / 5.0f; // Ulang setiap 5 unit lebar
    float texRepeatX_shortWall = arenaSize / 5.0f; // Untuk tembok timur/barat (lebarnya adalah arenaSize)
    float texRepeatY_wall = wallHeight / 5.0f;   // Ulang setiap 5 unit tinggi

    glNormal3f(0.0f, 0.0f, 0.0f); // Normal akan di-set per sisi

    // Tembok bagian dalam (yang menghadap ke arena)

    // Tembok Selatan (Bagian Dalam - Menghadap -Z)
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, boundarySize); // Posisikan ke Z positif (tepi luar)
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
    glTranslatef(0.0f, 0.0f, -boundarySize); // Posisikan ke Z negatif (tepi luar)
    glBegin(GL_QUADS);
        glNormal3f(0.0f, 0.0f, 1.0f); // Normal menghadap ke dalam arena (+Z)
        glTexCoord2f(0.0f, 0.0f); glVertex3f(boundarySize + wallThickness, 0.0f, 0.0f);
        glTexCoord2f(texRepeatX_longWall, 0.0f); glVertex3f(-boundarySize - wallThickness, 0.0f, 0.0f);
        glTexCoord2f(texRepeatX_longWall, texRepeatY_wall); glVertex3f(-boundarySize - wallThickness, wallHeight, 0.0f);
        glTexCoord2f(0.0f, texRepeatY_wall); glVertex3f(boundarySize + wallThickness, wallHeight, 0.0f);
    glEnd();
    glPopMatrix();

    // Tembok Timur (Bagian Dalam - Menghadap -X)
    glPushMatrix();
    glTranslatef(boundarySize, 0.0f, 0.0f); // Posisikan ke X positif (tepi luar)
    glBegin(GL_QUADS);
        glNormal3f(-1.0f, 0.0f, 0.0f); // Normal menghadap ke dalam arena (-X)
        glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f, 0.0f, boundarySize + wallThickness);
        glTexCoord2f(texRepeatX_shortWall, 0.0f); glVertex3f(0.0f, 0.0f, -boundarySize - wallThickness);
        glTexCoord2f(texRepeatX_shortWall, texRepeatY_wall); glVertex3f(0.0f, wallHeight, -boundarySize - wallThickness);
        glTexCoord2f(0.0f, texRepeatY_wall); glVertex3f(0.0f, wallHeight, boundarySize + wallThickness);
    glEnd();
    glPopMatrix();
    
    // Tembok Barat (Bagian Dalam - Menghadap +X)
    glPushMatrix();
    glTranslatef(-boundarySize, 0.0f, 0.0f); // Posisikan ke X negatif (tepi luar)
    glBegin(GL_QUADS);
        glNormal3f(1.0f, 0.0f, 0.0f); // Normal menghadap ke dalam arena (+X)
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
    // Bagian utama jalan (permukaan bertekstur)
    glPushMatrix();
    if (texture_jalan_paving != 0) { // Hanya terapkan tekstur jika berhasil dimuat
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texture_jalan_paving);
        // Set warna ke putih agar warna asli tekstur muncul, dimodulasi pencahayaan
        glColor3f(1.0f, 1.0f, 1.0f); 
    } else {
        glColor3f(0.2f, 0.2f, 0.2f); // Warna fallback jika tekstur gagal dimuat
    }
    
    glTranslatef(0.0f, 0.0f, 0.0f); // Posisi jalan (Y sedikit di bawah origin)
    
    // Menggambar jalan sebagai satu quad besar di atas untuk UV mapping yang lebih baik
    // Ini menggantikan glScalef dan glutSolidCube untuk permukaan utama jalan
    // agar kita bisa mengontrol pengulangan tekstur.
    float halfArena = arenaSize / 2.0f;
    // Tentukan berapa kali tekstur diulang. Sesuaikan angka 5.0f untuk skala yang berbeda.
    float texRoadRepeat = arenaSize / 10.0f; 

    glBegin(GL_QUADS);
        glNormal3f(0.0f, 1.0f, 0.0f); // Normal menghadap ke atas
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-halfArena, 0.0f, -halfArena);
        glTexCoord2f(texRoadRepeat, 0.0f); glVertex3f(halfArena, 0.0f, -halfArena);
        glTexCoord2f(texRoadRepeat, texRoadRepeat); glVertex3f(halfArena, 0.0f, halfArena);
        glTexCoord2f(0.0f, texRoadRepeat); glVertex3f(-halfArena, 0.0f, halfArena);
    glEnd();
    
    glPopMatrix(); // Selesai dengan bagian utama jalan

    if (texture_jalan_paving != 0) {
        glDisable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind
    }

    // Marka jalan (tetap digambar sebagai kubus solid di atas jalan bertekstur)
    // Pastikan lighting aktif untuk marka jika sebelumnya termodifikasi
    glEnable(GL_LIGHTING); // Jika sebelumnya ter-disable untuk bagian tertentu
    glColor3f(1.0f, 1.0f, 1.0f); // Warna putih untuk marka jalan
    
    glPushMatrix();
    // Posisikan marka sedikit di atas permukaan jalan utama untuk menghindari Z-fighting
    // Jalan utama ada di Y = -0.05f. Ketebalan marka 0.01f. Jadi Y marka = -0.05f + 0.005f (tengahnya)
    // Atau, jika jalan digambar di Y=0 pada quad di atas, maka marka di Y=0.01f
    // Mengikuti logika quad di atas (Y=0 untuk permukaan jalan), maka marka di Y sedikit di atas 0.
    glTranslatef(0.0f, 0.01f, 0.0f); // Y marka sedikit di atas permukaan jalan (Y=0)
    
    glPopMatrix(); // Selesai dengan marka jalan
}

const vector<Building>& getArenaBuildings() {
    return buildings;
}

// Initialize the arena
void initArena() {
    // Clear any existing buildings
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

// Draw the entire arena
void drawArena(float camX, float camY, float camZ) {
    // Draw the ground/road
    drawRoad();
    
    // Draw walls
    drawWalls();
    
    // Draw all buildings
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

// Enhanced collision detection that allows horizontal movement on surfaces
bool checkCollision(float x, float y, float z, float radius, float* responseX, float* responseY, float* responseZ) {
    bool collision = false;
    *responseX = 0.0f;
    *responseY = 0.0f;
    *responseZ = 0.0f;
    
    // Height offset to keep Doraemon properly positioned above surfaces
    float heightOffset = 1.0f;
    
    // Flag to track if we're standing on a surface
    bool standingOnSurface = false;
    float surfaceHeight = 0.0f;
    
    // Check collision with buildings
    for (size_t i = 0; i < buildings.size(); i++) {
        Building b = buildings[i];
        
        // Calculate building dimensions
        float halfWidth = b.width / 2.0f;
        float halfDepth = b.depth / 2.0f;
        
        // Special case for standing on top of buildings
        // If Doraemon is above the building in X-Z coordinates
        if (fabs(x - b.x) < halfWidth && fabs(z - b.z) < halfDepth) {
            // If we're close to the top of the building
            if (y < b.height + radius + heightOffset && y > b.height - radius) {
                // Remember we're standing on a surface
                standingOnSurface = true;
                
                // Record the surface height if it's higher than any previous surface
                if (b.height > surfaceHeight) {
                    surfaceHeight = b.height;
                }
                
                // Don't add horizontal collision response when standing on top
                continue;
            }
        }
        
        // Regular 3D collision check for sides of buildings
        // Only if we're not already standing on this building
        if (!standingOnSurface || surfaceHeight != b.height) {
            // Calculate closest point on building box to Doraemon's center
            float closestX = fmax(b.x - halfWidth, fmin(x, b.x + halfWidth));
            float closestY = fmax(0, fmin(y, b.height)); 
            float closestZ = fmax(b.z - halfDepth, fmin(z, b.z + halfDepth));
            
            // Calculate distance from Doraemon to closest point
            float distanceX = x - closestX;
            float distanceY = y - closestY;
            float distanceZ = z - closestZ;
            
            float distanceSquared = distanceX * distanceX + 
                                   distanceY * distanceY + 
                                   distanceZ * distanceZ;
            
            // Check if distance is less than Doraemon's radius squared (collision)
            if (distanceSquared < radius * radius) {
                float distance = sqrt(distanceSquared);
                if (distance > 0) {
                    float pushDistance = radius - distance;
                    
                    // Calculate normalized response vector
                    float normalX = distanceX / distance;
                    float normalY = distanceY / distance;
                    float normalZ = distanceZ / distance;
                    
                    // Add to response vector
                    *responseX += normalX * pushDistance * 1.05f;
                    *responseY += normalY * pushDistance * 1.05f;
                    *responseZ += normalZ * pushDistance * 1.05f;
                }
                collision = true;
            }
        }
    }
    
    // Check collision with arena boundaries (walls)
    float boundarySize = arenaSize / 2;
    float wallRadius = radius + 0.2f;
    
    // Walls extend from y=0 to y=wallHeight
    float wallTop = wallHeight;
    
    // Only check wall collision if Doraemon is below wall height
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
    
    // Check collision with road (at y=0)
    if (!standingOnSurface && y < radius + heightOffset) {
        standingOnSurface = true;
        surfaceHeight = 0.0f;
    }
    
    // Apply vertical position correction if standing on a surface
    if (standingOnSurface) {
        // Target height is surface height + radius + height offset
        float targetHeight = surfaceHeight + radius + heightOffset;
        
        // Apply a gentler vertical correction to avoid abrupt jumps
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

// Update camera view direction based on angles
void updateCameraLook(float newAngleX, float newAngleY) {
    angleX = newAngleX;
    angleY = newAngleY;
    
    // Convert angles to radians
    float angleXRad = angleX * 3.14159f / 180.0f;
    float angleYRad = angleY * 3.14159f / 180.0f;
    
    // Calculate look-at point based on angles
    lookX = posX + sin(angleYRad) * cos(angleXRad);
    lookY = posY + sin(angleXRad);
    lookZ = posZ - cos(angleYRad) * cos(angleXRad);
}

// Get camera position
void getCameraPosition(float* x, float* y, float* z) {
    *x = posX;
    *y = posY;
    *z = posZ;
}

// Get camera look-at point
void getCameraLookAt(float* x, float* y, float* z) {
    *x = lookX;
    *y = lookY;
    *z = lookZ;
}

// Draw text on screen
void drawArenaText(float x, float y, const char* text) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, glutGet(GLUT_WINDOW_WIDTH), 0.0, glutGet(GLUT_WINDOW_HEIGHT));
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(x, y);
    
    for (const char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }
    
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
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