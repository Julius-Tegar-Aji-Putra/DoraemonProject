#ifndef DORAEMON_C
#define DORAEMON_C

#include "doraemon.h"
#include "arena.h"
#include <stdio.h>
#include <math.h>
#include <GL/glut.h>

// Definisikan M_PI jika tidak tersedia
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Posisi dan orientasi Doraemon
static float posX = 0.0, posY = 0.0, posZ = 0.0;
static float angleY = 0.0;
static float tiltX = 0.0, tiltZ = 0.0;  // Kemiringan untuk condong saat bergerak
// Target tilt values (what we're aiming for)
static float targetTiltX = 0.0f, targetTiltZ = 0.0f;
// Tilt increment speed (smaller = smoother but slower)
static float tiltSpeed = 0.5f;

// Rotasi baling-baling
static float propellerRotation = 0.0;

// Parameter kamera
static float cameraDistance = 5.0;  // Jarak kamera dari Doraemon
static float cameraHeight = 2.0;    // Ketinggian kamera relatif thd Doraemon
static float freeCamX = 10.0, freeCamY = 8.0, freeCamZ = 10.0;  // Posisi kamera bebas
static int cameraMode = 1;  // 1 = third person, 0 = kamera bebas

// Status tombol
static bool keyStates[256];

// Variabel untuk orientasi kamera
static float camAngle = 0.0f;
static float camLookX = 0.0f, camLookZ = -1.0f; // Vektor arah kamera

float arenaSize = getArenaSize();

// Fungsi untuk menggambar silinder
static void drawCylinder(float radius, float height, int slices) {
    float angle, x, y;
    
    // Tutup atas
    glBegin(GL_POLYGON);
    glVertex3f(0.0f, height, 0.0f);
    for (int i = 0; i <= slices; i++) {
        angle = 2.0f * M_PI * i / slices;
        x = radius * cos(angle);
        y = radius * sin(angle);
        glVertex3f(x, height, y);
    }
    glEnd();
    
    // Tutup bawah
    glBegin(GL_POLYGON);
    glVertex3f(0.0f, 0.0f, 0.0f);
    for (int i = slices; i >= 0; i--) {
        angle = 2.0f * M_PI * i / slices;
        x = radius * cos(angle);
        y = radius * sin(angle);
        glVertex3f(x, 0.0f, y);
    }
    glEnd();
    
    // Badan silinder
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= slices; i++) {
        angle = 2.0f * M_PI * i / slices;
        x = radius * cos(angle);
        y = radius * sin(angle);
        glVertex3f(x, 0.0f, y);
        glVertex3f(x, height, y);
    }
    glEnd();
}

// Fungsi untuk menampilkan text di OpenGL
void drawText(const char* text, float x, float y, float r, float g, float b) {
    // Set warna teks
    glColor3f(r, g, b);
    glRasterPos2f(x, y);
    
    // Render teks karakter per karakter
    for (const char* c = text; *c; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }
}

// Function to smoothly adjust tilt angles
static void smoothTilt() {
    // Gradually change tiltX towards targetTiltX
    if (tiltX < targetTiltX) {
        tiltX += tiltSpeed;
        if (tiltX > targetTiltX) tiltX = targetTiltX;
    } else if (tiltX > targetTiltX) {
        tiltX -= tiltSpeed;
        if (tiltX < targetTiltX) tiltX = targetTiltX;
    }
    
    // Gradually change tiltZ towards targetTiltZ
    if (tiltZ < targetTiltZ) {
        tiltZ += tiltSpeed;
        if (tiltZ > targetTiltZ) tiltZ = targetTiltZ;
    } else if (tiltZ > targetTiltZ) {
        tiltZ -= tiltSpeed;
        if (tiltZ < targetTiltZ) tiltZ = targetTiltZ;
    }
}

// Fungsi untuk menampilkan info kontrol
void displayControlInfo() {
    int yPos = 30; // Posisi Y awal (dari bawah)
    int lineHeight = 20; // Tinggi setiap baris
    
    // Informasi kontrol utama
    drawText("0: Mode kamera bebas", 10, yPos, 1.0f, 1.0f, 1.0f);
    yPos += lineHeight;
    drawText("1: Mode third person", 10, yPos, 1.0f, 1.0f, 1.0f);
    yPos += lineHeight;
    
    if (cameraMode == 0) {
        // Info kontrol untuk mode kamera bebas
        drawText("--- Mode Free Camera ---", 800/2 - 100, 600 - 50, 1.0f, 1.0f, 0.0f);
        drawText("Tombol panah: Gerak kamera secara horizontal", 800/2 - 150, 600 - 70, 1.0f, 1.0f, 0.0f);
        drawText("W/S: Gerak kamera secara vertikal", 800/2 - 150, 600 - 90, 1.0f, 1.0f, 0.0f);
        drawText("A/D: Rotasi kamera", 800/2 - 150, 600 - 110, 1.0f, 1.0f, 0.0f);
    } else {
        // Info kontrol untuk mode third person
        drawText("--- Mode Third Person ---", 800/2 - 100, 600 - 50, 1.0f, 1.0f, 0.0f);
        drawText("W/S: Gerak Doraemon maju/mundur", 800/2 - 150, 600 - 70, 1.0f, 1.0f, 0.0f);
        drawText("A/D: Putar Doraemon kiri/kanan", 800/2 - 150, 600 - 90, 1.0f, 1.0f, 0.0f);
        drawText("Q/E: Terbang naik/turun", 800/2 - 150, 600 - 110, 1.0f, 1.0f, 0.0f);
    }
}

// Fungsi untuk orientasi kamera
void orientCamera(float ang) {
    camLookX = sin(ang);
    camLookZ = -cos(ang);
}

// Fungsi untuk gerak kamera
void moveCameraFlat(int direction) {
    freeCamX += direction * (camLookX * 1.0f);
    freeCamZ += direction * (camLookZ * 1.0f);
}

// Fungsi untuk menangani special keys (arrow keys)
void specialKeyPressed(int key, int x, int y) {
    if (cameraMode == 0) { // Hanya di mode free camera
        switch (key) {
            case GLUT_KEY_LEFT:
                camAngle -= 0.05f;
                orientCamera(camAngle);
                break;
            case GLUT_KEY_RIGHT:
                camAngle += 0.05f;
                orientCamera(camAngle);
                break;
            case GLUT_KEY_UP:
                moveCameraFlat(1);
                break;
            case GLUT_KEY_DOWN:
                moveCameraFlat(-1);
                break;
        }
    }
    glutPostRedisplay();
}

// Fungsi untuk menggambar baling-baling bambu
static void drawPropeller() {
    glPushMatrix();
    
    // Batang baling-baling (vertikal)
    glColor3f(0.5f, 0.35f, 0.05f);  // Warna coklat
    glPushMatrix();
    drawCylinder(0.025f, 0.25f, 20);
    glPopMatrix();
    
    // Baling-baling
    glTranslatef(0.0f, 0.3f, 0.0f);
    glRotatef(propellerRotation, 0.0f, 1.0f, 0.0f);
    
    glColor3f(0.5f, 0.38f, 0.05f);  // Warna coklat
    
    // 4 bilah baling-baling
    for (int i = 0; i < 6; i++) {
        glPushMatrix();
        
        glTranslatef(0.0f, -0.04f, 0.0f);             // Posisikan sedikit ke bawah
        glRotatef(90.0f * i, 0.0f, 1.0f, 0.0f);       // Rotasi tiap bilah 90 derajat
        
        glPushMatrix();
        glTranslatef(0.2f, 0.0f, 0.0f);               // Geser ke ujung bilah
        glScalef(0.25f, 0.045f, 0.1f);                // Panjang, ketebalan, lebar bilah
        glutSolidCube(1.0f);                          // Gambar kubus (akan diskalakan jadi balok)
        glPopMatrix();

        glPopMatrix();
    }
    
    glPopMatrix();
}

// Fungsi untuk menggambar kepala Doraemon
static void drawHead() {
    // Kepala (bulat) - biru
    glColor3f(0.0f, 0.6f, 1.0f);
    glutSolidSphere(0.5f, 30, 30);
    
    // Muka (putih)
    glColor3f(1.0f, 1.0f, 1.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.25f);
    glScalef(0.8f, 0.8f, 0.5f);
    glutSolidSphere(0.5f, 30, 30);
    glPopMatrix();
    
    // Mata kiri
    glColor3f(1.0f, 1.0f, 1.0f);
    glPushMatrix();
    glTranslatef(-0.15f, 0.2f, 0.425f);
    glutSolidSphere(0.1f, 20, 20);
    
    // Pupil kiri
    glColor3f(0.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, 0.0f, 0.07f);
    glutSolidSphere(0.04f, 10, 10);
    glPopMatrix();
    
    // Mata kanan
    glColor3f(1.0f, 1.0f, 1.0f);
    glPushMatrix();
    glTranslatef(0.15f, 0.2f, 0.425f);
    glutSolidSphere(0.1f, 20, 20);
    
    // Pupil kanan
    glColor3f(0.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, 0.0f, 0.07f);
    glutSolidSphere(0.04f, 10, 10);
    glPopMatrix();
    
    // Hidung (merah)
    glColor3f(1.0f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.05f, 0.5f);
    glutSolidSphere(0.08f, 20, 20);
    glPopMatrix();
    
    // Mulut
    glColor3f(1.0f, 0.0f, 0.0f); // Warna merah
    glPushMatrix();
    glTranslatef(0.0f, -0.1f, 0.5f); // Posisi mulut
    glRotatef(270.0f, 0.0f, 0.0f, 1.0f); // Rotasi agar menghadap ke depan
    glScalef(0.15f, 0.1f, 0.1f); // Skala untuk ukuran mulut

    // Membuat setengah tabung (setengah lingkaran)
    GLUquadricObj *mulut = gluNewQuadric();
    gluQuadricDrawStyle(mulut, GLU_FILL);
    gluPartialDisk(mulut, 0.0f, 1.0f, 20, 1, 0, 180); // Membuat setengah lingkaran
    gluDeleteQuadric(mulut);
    glPopMatrix();
    
    // Kumis
    glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(2.0f);
    
    // Kumis kiri
    glBegin(GL_LINES);
    glVertex3f(-0.2f, -0.05f, 0.5f);
    glVertex3f(-0.4f, 0.0f, 0.4f);
    
    glVertex3f(-0.2f, -0.1f, 0.5f);
    glVertex3f(-0.4f, -0.1f, 0.4f);
    
    glVertex3f(-0.2f, -0.15f, 0.5f);
    glVertex3f(-0.4f, -0.2f, 0.4f);
    glEnd();
    
    // Kumis kanan
    glBegin(GL_LINES);
    glVertex3f(0.2f, -0.05f, 0.5f);
    glVertex3f(0.4f, 0.0f, 0.4f);
    
    glVertex3f(0.2f, -0.1f, 0.5f);
    glVertex3f(0.4f, -0.1f, 0.4f);
    
    glVertex3f(0.2f, -0.15f, 0.5f);
    glVertex3f(0.4f, -0.2f, 0.4f);
    glEnd();
    
    glLineWidth(1.0f);
}

// Fungsi untuk menggambar badan Doraemon
static void drawBody() {
    // Badan (biru)
    glColor3f(0.0f, 0.6f, 1.0f);
    glPushMatrix();
    glTranslatef(0.0f, -0.7f, 0.0f);
    glScalef(0.8f, 1.0f, 0.6f);
    glutSolidSphere(0.5f, 30, 30);
    glPopMatrix();
    
    // Perut (putih)
    glColor3f(1.0f, 1.0f, 1.0f);
    glPushMatrix();
    glTranslatef(0.0f, -0.75f, 0.245f);
    glScalef(0.8f, 0.8f, 0.3f);
    glutSolidSphere(0.3f, 30, 30);
    glPopMatrix();
    
    // Kalung (merah)
    glColor3f(1.0f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.0f, -0.4f, 0.0f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    glutSolidTorus(0.05f, 0.3f, 20, 30);
    glPopMatrix();
    
    // Lonceng (kuning)
    glColor3f(1.0f, 1.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.0f, -0.45f, 0.35f);
    glutSolidSphere(0.07f, 20, 20);
    glPopMatrix();
}

// Fungsi untuk menggambar tangan Doraemon
static void drawArms() {
    // Tangan kiri (biru)
    glColor3f(0.0f, 0.6f, 1.0f);
    glPushMatrix();
    glTranslatef(-0.4f, -0.6f, 0.0f);
    glScalef(0.25f, 0.4f, 0.2f);
    glutSolidSphere(0.475f, 20, 20);
    glPopMatrix();
    
    // Tangan kanan (biru)
    glPushMatrix();
    glTranslatef(0.4f, -0.6f, 0.0f);
    glScalef(0.25f, 0.4f, 0.2f);
    glutSolidSphere(0.475f, 20, 20);
    glPopMatrix();
    
    // Sarung tangan kiri (putih)
    glColor3f(1.0f, 1.0f, 1.0f);
    glPushMatrix();
    glTranslatef(-0.4f, -0.85f, 0.0f);
    glutSolidSphere(0.1f, 20, 20);
    glPopMatrix();
    
    // Sarung tangan kanan (putih)
    glPushMatrix();
    glTranslatef(0.4f, -0.85f, 0.0f);
    glutSolidSphere(0.1f, 20, 20);
    glPopMatrix();
}

// Fungsi untuk menggambar kaki Doraemon
static void drawLegs() {
    // Kaki kiri (biru)
    glColor3f(0.0f, 0.6f, 1.0f);
    glPushMatrix();
    glTranslatef(-0.2f, -1.2f, 0.0f);
    glScalef(0.3f, 0.5f, 0.2f);
    glutSolidSphere(0.5f, 20, 20);
    glPopMatrix();
    
    // Kaki kanan (biru)
    glPushMatrix();
    glTranslatef(0.2f, -1.2f, 0.0f);
    glScalef(0.3f, 0.5f, 0.2f);
    glutSolidSphere(0.5f, 20, 20);
    glPopMatrix();
    
    // Sepatu kiri (putih)
    glColor3f(1.0f, 1.0f, 1.0f);
    glPushMatrix();
    glTranslatef(-0.2f, -1.4f, 0.025f);
    glScalef(0.2f, 0.1f, 0.2f);
    glutSolidSphere(0.6f, 20, 20);
    glPopMatrix();
    
    // Sepatu kanan (putih)
    glPushMatrix();
    glTranslatef(0.2f, -1.4f, 0.025f);
    glScalef(0.2f, 0.1f, 0.2f);
    glutSolidSphere(0.6f, 20, 20);
    glPopMatrix();
}

// Fungsi untuk menggambar Doraemon lengkap
void drawDoraemon() {
    glPushMatrix();
    
    // Rotasi sesuai dengan sudut orientasi
    glRotatef(angleY, 0.0f, 1.0f, 0.0f);
    
    // Terapkan kemiringan (tilt) sesuai arah gerakan
    glRotatef(tiltX, 0.0f, 0.0f, 1.0f); // Kemiringan kiri-kanan
    glRotatef(tiltZ, 1.0f, 0.0f, 0.0f); // Kemiringan depan-belakang
    
    // Gambar kepala
    drawHead();
    
    // Gambar baling-baling bambu di atas kepala
    glPushMatrix();
    glTranslatef(0.0f, 0.5f, 0.0f);
    drawPropeller();
    glPopMatrix();
    
    // Gambar badan, tangan, dan kaki
    drawBody();
    drawArms();
    drawLegs();
    
    glPopMatrix();
}

// Fungsi untuk menggambar bidang datar (tanah)
void drawGround() {
    glColor3f(0.2f, 0.8f, 0.2f);  // Warna hijau untuk tanah
    
    glBegin(GL_QUADS);
    glVertex3f(-50.0f, -1.55f, -50.0f);
    glVertex3f(-50.0f, -1.55f, 50.0f);
    glVertex3f(50.0f, -1.55f, 50.0f);
    glVertex3f(50.0f, -1.55f, -50.0f);
    glEnd();
    
    // Tambahkan grid untuk orientasi
    glColor3f(0.5f, 0.5f, 0.5f);
    glBegin(GL_LINES);
    for (int i = -50; i <= 50; i += 5) {
        // Garis pada sumbu X
        glVertex3f(i, -1.54f, -50.0f);
        glVertex3f(i, -1.54f, 50.0f);
        
        // Garis pada sumbu Z
        glVertex3f(-50.0f, -1.54f, i);
        glVertex3f(50.0f, -1.54f, i);
    }
    glEnd();
}

// Fungsi untuk memproses tombol yang ditekan
static void processKeys() {
    float speed = 0.3f;
    float maxTilt = 15.0f; // Kemiringan maksimum dalam derajat
    float camSpeed = 0.25f; // Kecepatan gerakan kamera bebas
    float collisionRadius = 0.5f; // Approximate radius of Doraemon
    
    // Set target tilt to return to neutral when no keys are pressed
    targetTiltX = 0.0f;
    targetTiltZ = 0.0f;
    
    if (cameraMode == 0) {
        // Free camera controls - unchanged
        if (keyStates['w'] || keyStates['W']) {
            freeCamY += camSpeed;
        }
        
        if (keyStates['s'] || keyStates['S']) {
            if (freeCamY > 0.5f) {
                freeCamY -= camSpeed;
            }
        }
        
        // A/D for camera rotation
        if (keyStates['a'] || keyStates['A']) {
            camAngle -= 0.05f;
            orientCamera(camAngle);
        }
        
        if (keyStates['d'] || keyStates['D']) {
            camAngle += 0.05f;
            orientCamera(camAngle);
        }
    } else {
        float newX = posX;
        float newY = posY;
        float newZ = posZ;
        bool moved = false;
        
        // Doraemon controls
        if (keyStates['w'] || keyStates['W']) {
            // Calculate movement
            newX = posX + speed * sin(angleY * M_PI / 180.0);
            newZ = posZ + speed * cos(angleY * M_PI / 180.0);
            moved = true;
            targetTiltZ = maxTilt; // Set target tilt forward
        }
        
        if (keyStates['s'] || keyStates['S']) {
            // Calculate movement
            newX = posX - speed * sin(angleY * M_PI / 180.0);
            newZ = posZ - speed * cos(angleY * M_PI / 180.0);
            moved = true;
            targetTiltZ = -maxTilt; // Set target tilt backward
        }
        
        if (keyStates['a'] || keyStates['A']) {
            // Rotate left
            angleY += 1.0f;
            targetTiltX = -maxTilt; // Set target tilt left
        }
        
        if (keyStates['d'] || keyStates['D']) {
            // Rotate right
            angleY -= 1.0;
            targetTiltX = maxTilt; // Set target tilt right
        }
        
        if (keyStates['q'] || keyStates['Q']) {
            // Move up
            newY = posY + speed;
            moved = true;
        }
        
        if (keyStates['e'] || keyStates['E']) {
            // Move down
            newY = posY - speed;
            moved = true;
        }
        
        // Handle movement and collision
        if (moved) {
            // Try X and Z movement (horizontal) separately from Y (vertical)
            
            // First try horizontal movement (X and Z)
            float responseX = 0.0f;
            float responseY = 0.0f;
            float responseZ = 0.0f;
            
            // Check horizontal movement only (no Y change)
            bool horizontalCollision = checkCollision(newX, posY, newZ, collisionRadius, 
                                                   &responseX, &responseY, &responseZ);
            
            // Apply horizontal movement
            posX = newX + responseX;
            posZ = newZ + responseZ;
            
            // Now apply vertical movement
            responseX = 0.0f;
            responseY = 0.0f;
            responseZ = 0.0f;
            
            bool verticalCollision = checkCollision(posX, newY, posZ, collisionRadius,
                                                 &responseX, &responseY, &responseZ);
            
            // Apply vertical response
            posY = newY + responseY;
            
            // Final check to ensure we're at the right height
            responseX = 0.0f;
            responseY = 0.0f;
            responseZ = 0.0f;
            
            checkCollision(posX, posY, posZ, collisionRadius,
                          &responseX, &responseY, &responseZ);
            
            // Apply any additional adjustment (mainly for Y positioning)
            posX += responseX;
            posY += responseY;
            posZ += responseZ;
        }
        
        // Add respawn functionality
        if (keyStates['r'] || keyStates['R']) {
            // Respawn Doraemon at initial position
            posX = 10.0f;
            posY = 10.0f;
            posZ = 0.0f;
            angleY = 0.0f;
        }
    }
}

// Function to update Doraemon (called from idle)
void updateDoraemon() {
    // Rotasi baling-baling
    propellerRotation += 10.0f;
    if (propellerRotation > 360.0f) {
        propellerRotation -= 360.0f;
    }
    
    // Process keyboard input
    processKeys();
    
    // Apply smooth tilting
    smoothTilt();
}

// Function to set key state
void setKeyState(unsigned char key, bool state) {
    keyStates[key] = state;
}

// Function to set camera mode
void setCameraMode(int mode) {
    cameraMode = mode;
}

// Function to get camera mode
int getCameraMode() {
    return cameraMode;
}

// Fungsi untuk memeriksa tabrakan kamera dengan objek di arena
bool checkCameraCollision(float cameraX, float cameraY, float cameraZ, float* responseX, float* responseY, float* responseZ) {
    // Radius kamera sekitar Doraemon (untuk menghindari menembus objek)
    float cameraRadius = 1.0f;
    
    // Pemeriksaan tabrakan dengan arena
    bool collision = false;
    *responseX = 0.0f;
    *responseY = 0.0f;
    *responseZ = 0.0f;

    // Periksa tabrakan dengan tembok arena
    float boundarySize = arenaSize / 2;
    
    // Jika kamera lebih dekat dengan dinding, kita dorong kamera ke luar
    if (cameraX + cameraRadius > boundarySize) {
        *responseX = (boundarySize - (cameraX + cameraRadius));
        collision = true;
    }
    if (cameraX - cameraRadius < -boundarySize) {
        *responseX = (-boundarySize - (cameraX - cameraRadius));
        collision = true;
    }
    if (cameraZ + cameraRadius > boundarySize) {
        *responseZ = (boundarySize - (cameraZ + cameraRadius));
        collision = true;
    }
    if (cameraZ - cameraRadius < -boundarySize) {
        *responseZ = (-boundarySize - (cameraZ - cameraRadius));
        collision = true;
    }
    
    return collision;
}

// Getter functions for camera
float getCameraX() {
    if (cameraMode == 1) {
        float responseX = 0.0f, responseY = 0.0f, responseZ = 0.0f;
        float cameraX = posX - cameraDistance * sin(angleY * M_PI / 180.0);
        // Periksa tabrakan kamera dengan objek dan atur posisi jika diperlukan
        if (checkCameraCollision(cameraX, posY, posZ, &responseX, &responseY, &responseZ)) {
            cameraX += responseX;  // Sesuaikan posisi kamera
        }
        return cameraX;
    } else {
        return freeCamX;
    }
}

float getCameraY() {
    if (cameraMode == 1) {
        return posY + cameraHeight;
    } else {
        return freeCamY;
    }
}

float getCameraZ() {
    if (cameraMode == 1) {
        float responseX = 0.0f, responseY = 0.0f, responseZ = 0.0f;
        float cameraZ = posZ - cameraDistance * cos(angleY * M_PI / 180.0);
        
        // Periksa tabrakan kamera dengan objek dan atur posisi jika diperlukan
        if (checkCameraCollision(posX, posY, cameraZ, &responseX, &responseY, &responseZ)) {
            cameraZ += responseZ;  // Sesuaikan posisi kamera
        }
        return cameraZ;
    } else {
        return freeCamZ;
    }
}

float getLookX() {
    if (cameraMode == 1) {
        return posX;
    } else {
        return freeCamX + camLookX;
    }
}

float getLookY() {
    if (cameraMode == 1) {
        return posY;
    } else {
        return freeCamY;
    }
}

float getLookZ() {
    if (cameraMode == 1) {
        return posZ;
    } else {
        return freeCamZ + camLookZ;
    }
}

// Function to initialize Doraemon
void initDoraemon() {
    // Initialize key states
    for (int i = 0; i < 256; i++) {
        keyStates[i] = false;
    }
    
    // Initialize position, orientation, etc.
    posX = 10.0;
    posY = 10.0;
    posZ = 0.0;
    angleY = 0.0;
    tiltX = 0.0;
    tiltZ = 0.0;
    targetTiltX = 0.0;
    targetTiltZ = 0.0;
    propellerRotation = 0.0;
    
    // Initialize camera
    camAngle = 0.0f;
    camLookX = 0.0f;
    camLookZ = -1.0f;
    orientCamera(camAngle);
}

// Implementasi fungsi resetDoraemonState jika Anda menambahkannya
void resetDoraemonState() {
    // Panggil initDoraemon() atau salin logika reset posisi & state ke sini
    initDoraemon(); // Cara termudah adalah memanggil init lagi
    printf("Doraemon state reset.\n");
}

// Position getters for Doraemon
float getDoraemonX() {
    return posX;
}

float getDoraemonY() {
    return posY;
}

float getDoraemonZ() {
    return posZ;
}



#endif // DORAEMON_C