#ifndef DORAEMON_C
#define DORAEMON_C

#include "doraemon.h"
#include "arena.h"
#include <stdio.h>
#include <math.h>
#include <GL/glut.h>
#include <string>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


static float posX = 0.0, posY = 0.0, posZ = 0.0;
static float angleY = 0.0;
static float tiltX = 0.0, tiltZ = 0.0;  
static float targetTiltX = 0.0f, targetTiltZ = 0.0f;
static float tiltSpeed = 0.5f;

static float propellerRotation = 0.0;

static float cameraDistance = 5.0;  
static float cameraHeight = 2.0;    
static float freeCamX = 10.0, freeCamY = 8.0, freeCamZ = 10.0;  
static int cameraMode = 1;  // 1 = third person, 0 = kamera bebas

static bool keyStates[256];

static float camAngle = 0.0f;
static float camLookX = 0.0f, camLookZ = -1.0f; 

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
void drawText(const char* text, float x, float y,
              float rText, float gText, float bText,         
              float rBg, float gBg, float bBg, float aBg,   
              void* font, float bgPaddingX, float bgPaddingY) {

    float charHeight = 0.0f;
    if (font == GLUT_BITMAP_HELVETICA_10) { charHeight = 10.0f; }
    else if (font == GLUT_BITMAP_HELVETICA_12) { charHeight = 12.0f; }
    else if (font == GLUT_BITMAP_HELVETICA_18) { charHeight = 18.0f; }
    else if (font == GLUT_BITMAP_TIMES_ROMAN_10) { charHeight = 10.0f; }
    else if (font == GLUT_BITMAP_TIMES_ROMAN_24) { charHeight = 24.0f; }
    else if (font == GLUT_BITMAP_9_BY_15) { charHeight = 15.0f; }
    else if (font == GLUT_BITMAP_8_BY_13) { charHeight = 13.0f; }
    else { charHeight = 18.0f; } 

    GLboolean lightingEnabled = glIsEnabled(GL_LIGHTING);
    GLboolean textureEnabled = glIsEnabled(GL_TEXTURE_2D);
    GLboolean blendEnabled = glIsEnabled(GL_BLEND);
    GLboolean depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);

    if (lightingEnabled) glDisable(GL_LIGHTING);
    if (textureEnabled) glDisable(GL_TEXTURE_2D);
    if (depthTestEnabled) glDisable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

    int totalTextWidth = 0;
    for (const char* c = text; *c; c++) {
        totalTextWidth += glutBitmapWidth(font, *c);
    }

    if (aBg > 0.0f && totalTextWidth > 0) { 
        glColor4f(rBg, gBg, bBg, aBg); 
        glBegin(GL_QUADS);
            glVertex2f(x - bgPaddingX,               y - bgPaddingY);
            glVertex2f(x + totalTextWidth + bgPaddingX, y - bgPaddingY);
            glVertex2f(x + totalTextWidth + bgPaddingX, y + charHeight + bgPaddingY);
            glVertex2f(x - bgPaddingX,               y + charHeight + bgPaddingY);
        glEnd();
    }

    glColor3f(rText, gText, bText); 
    
    glRasterPos2f(x, y);
    for (const char* c = text; *c; c++) {
        glutBitmapCharacter(font, *c);
    }

    glRasterPos2f(x + 1, y); 
    for (const char* c = text; *c; c++) {
        glutBitmapCharacter(font, *c);
    }

    if (lightingEnabled) glEnable(GL_LIGHTING);
    if (textureEnabled) glEnable(GL_TEXTURE_2D);
    if (depthTestEnabled) glEnable(GL_DEPTH_TEST);
    if (!blendEnabled) glDisable(GL_BLEND); 
}

// Fungsi untuk mengatur kemiringan terus menerus secara halus
static void smoothTilt() {
    if (tiltX < targetTiltX) {
        tiltX += tiltSpeed;
        if (tiltX > targetTiltX) tiltX = targetTiltX;
    } else if (tiltX > targetTiltX) {
        tiltX -= tiltSpeed;
        if (tiltX < targetTiltX) tiltX = targetTiltX;
    }
    
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
    void* font = GLUT_BITMAP_HELVETICA_18; 
    float bgPaddingX = 4.0f; 
    float bgPaddingY = 3.0f; 

    float rText = 0.0f, gText = 0.0f, bText = 0.0f; 
    float rBg = 1.0f, gBg = 1.0f, bBg = 1.0f, aBg = 0.85f; 

    string panduanStr; 

    panduanStr += "0: Mode Kamera Bebas || 1: Mode Third Person";

    if (cameraMode == 0) { 
        panduanStr += " || Panah: Gerak Horizontal || W/S: Gerak Vertikal || A/D: Rotasi Kamera";
    } else { 
        panduanStr += " || W/S: Maju/Mundur || A/D: Putar Kiri/Kanan || Q/E: Terbang Naik/Turun";
    }

    int totalWidth = 0;
    for (size_t i = 0; i < panduanStr.length(); ++i) { 
        totalWidth += glutBitmapWidth(font, panduanStr[i]);
    }

    float screenWidth = glutGet(GLUT_WINDOW_WIDTH);
    float xPos = (screenWidth - totalWidth) / 2.0f; 
    float yPos = 30; 

    drawText(panduanStr.c_str(), xPos, yPos,
             rText, gText, bText,
             rBg, gBg, bBg, aBg,
             font, bgPaddingX, bgPaddingY);
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
    GLboolean lighting_on;
    glGetBooleanv(GL_LIGHTING, &lighting_on);

    glPushMatrix();
    
    // Batang baling-baling (vertikal)
    if (lighting_on) glColor3f(0.5f, 0.35f, 0.05f);  // Warna coklat
    glPushMatrix();
    drawCylinder(0.025f, 0.25f, 20);
    glPopMatrix();
    
    // Baling-baling
    glTranslatef(0.0f, 0.3f, 0.0f);
    glRotatef(propellerRotation, 0.0f, 1.0f, 0.0f);
    
    if (lighting_on) glColor3f(0.5f, 0.38f, 0.05f);  // Warna coklat
    
    // 4 bilah baling-baling
    for (int i = 0; i < 6; i++) {
        glPushMatrix();
        
        glTranslatef(0.0f, -0.04f, 0.0f);           
        glRotatef(90.0f * i, 0.0f, 1.0f, 0.0f);      
        
        glPushMatrix();
        glTranslatef(0.2f, 0.0f, 0.0f);             
        glScalef(0.25f, 0.045f, 0.1f);               
        glutSolidCube(1.0f);                          
        glPopMatrix();

        glPopMatrix();
    }
    
    glPopMatrix();
}

// Fungsi untuk menggambar kepala Doraemon
static void drawHead() {
    GLboolean lighting_on;
    glGetBooleanv(GL_LIGHTING, &lighting_on);
    // Kepala (bulat) - biru
    if (lighting_on) glColor3f(0.0f, 0.6f, 1.0f);
    glutSolidSphere(0.5f, 30, 30);
    
    // Muka (putih)
    if (lighting_on) glColor3f(1.0f, 1.0f, 1.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.25f);
    glScalef(0.8f, 0.8f, 0.5f);
    glutSolidSphere(0.5f, 30, 30);
    glPopMatrix();
    
    // Mata kiri
    if (lighting_on) glColor3f(1.0f, 1.0f, 1.0f);
    glPushMatrix();
    glTranslatef(-0.15f, 0.2f, 0.425f);
    glutSolidSphere(0.1f, 20, 20);
    
    // Pupil kiri
    if (lighting_on) glColor3f(0.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, 0.0f, 0.07f);
    glutSolidSphere(0.04f, 10, 10);
    glPopMatrix();
    
    // Mata kanan
    if (lighting_on) glColor3f(1.0f, 1.0f, 1.0f);
    glPushMatrix();
    glTranslatef(0.15f, 0.2f, 0.425f);
    glutSolidSphere(0.1f, 20, 20);
    
    // Pupil kanan
    if (lighting_on) glColor3f(0.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, 0.0f, 0.07f);
    glutSolidSphere(0.04f, 10, 10);
    glPopMatrix();
    
    // Hidung (merah)
    if (lighting_on) glColor3f(1.0f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.05f, 0.5f);
    glutSolidSphere(0.08f, 20, 20);
    glPopMatrix();
    
    // Mulut
    if (lighting_on) glColor3f(1.0f, 0.0f, 0.0f); // Warna merah
    glPushMatrix();
    glTranslatef(0.0f, -0.1f, 0.5f); // Posisi mulut
    glRotatef(270.0f, 0.0f, 0.0f, 1.0f); // Rotasi agar menghadap ke depan
    glScalef(0.15f, 0.1f, 0.1f); // Skala untuk ukuran mulut
    GLUquadricObj *mulut = gluNewQuadric();
    gluQuadricDrawStyle(mulut, GLU_FILL);
    gluPartialDisk(mulut, 0.0f, 1.0f, 20, 1, 0, 180); // Membuat setengah lingkaran
    gluDeleteQuadric(mulut);
    glPopMatrix();
    
    // Kumis
    if (lighting_on) glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(2.0f);
    
    // Kumis kiri
    glBegin(GL_LINES);
    glVertex3f(-0.2f, -0.05f, 0.5f); glVertex3f(-0.4f, 0.0f, 0.4f);
    glVertex3f(-0.2f, -0.1f, 0.5f); glVertex3f(-0.4f, -0.1f, 0.4f);
    glVertex3f(-0.2f, -0.15f, 0.5f); glVertex3f(-0.4f, -0.2f, 0.4f);
    glEnd();
    
    // Kumis kanan
    glBegin(GL_LINES);
    glVertex3f(0.2f, -0.05f, 0.5f); glVertex3f(0.4f, 0.0f, 0.4f);
    glVertex3f(0.2f, -0.1f, 0.5f); glVertex3f(0.4f, -0.1f, 0.4f);
    glVertex3f(0.2f, -0.15f, 0.5f); glVertex3f(0.4f, -0.2f, 0.4f);
    glEnd();
    
    glLineWidth(1.0f);
}

// Fungsi untuk menggambar badan Doraemon
static void drawBody() {
    GLboolean lighting_on;
    glGetBooleanv(GL_LIGHTING, &lighting_on); 

    // Badan (biru)
    if (lighting_on) glColor3f(0.0f, 0.6f, 1.0f);
    glPushMatrix();
    glTranslatef(0.0f, -0.7f, 0.0f);
    glScalef(0.8f, 1.0f, 0.6f);
    glutSolidSphere(0.5f, 30, 30);
    glPopMatrix();
    
    // Perut (putih)
    if (lighting_on) glColor3f(1.0f, 1.0f, 1.0f);
    glPushMatrix();
    glTranslatef(0.0f, -0.75f, 0.245f);
    glScalef(0.8f, 0.8f, 0.3f);
    glutSolidSphere(0.3f, 30, 30);
    glPopMatrix();
    
    // Kalung (merah)
    if (lighting_on) glColor3f(1.0f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.0f, -0.4f, 0.0f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    glutSolidTorus(0.05f, 0.3f, 20, 30);
    glPopMatrix();
    
    // Lonceng (kuning)
    if (lighting_on) glColor3f(1.0f, 1.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.0f, -0.45f, 0.35f);
    glutSolidSphere(0.07f, 20, 20);
    glPopMatrix();
}

// Fungsi untuk menggambar tangan Doraemon
static void drawArms() {
    GLboolean lighting_on;
    glGetBooleanv(GL_LIGHTING, &lighting_on); 

    // Tangan kiri (biru)
    if (lighting_on) glColor3f(0.0f, 0.6f, 1.0f);
    glPushMatrix();
    glTranslatef(-0.4f, -0.6f, 0.0f);
    glScalef(0.25f, 0.4f, 0.2f);
    glutSolidSphere(0.475f, 20, 20);
    glPopMatrix();
    
    // Tangan kanan (biru)
    if (lighting_on) glColor3f(0.0f, 0.6f, 1.0f); 
    glPushMatrix();
    glTranslatef(0.4f, -0.6f, 0.0f);
    glScalef(0.25f, 0.4f, 0.2f);
    glutSolidSphere(0.475f, 20, 20);
    glPopMatrix();
    
    // Sarung tangan kiri (putih)
    if (lighting_on) glColor3f(1.0f, 1.0f, 1.0f);
    glPushMatrix();
    glTranslatef(-0.4f, -0.85f, 0.0f);
    glutSolidSphere(0.1f, 20, 20);
    glPopMatrix();
    
    // Sarung tangan kanan (putih)
    if (lighting_on) glColor3f(1.0f, 1.0f, 1.0f); 
    glPushMatrix();
    glTranslatef(0.4f, -0.85f, 0.0f);
    glutSolidSphere(0.1f, 20, 20);
    glPopMatrix();
}

// Fungsi untuk menggambar kaki Doraemon
static void drawLegs() {
    GLboolean lighting_on;
    glGetBooleanv(GL_LIGHTING, &lighting_on); 

    // Kaki kiri (biru)
    if (lighting_on) glColor3f(0.0f, 0.6f, 1.0f);
    glPushMatrix();
    glTranslatef(-0.2f, -1.2f, 0.0f);
    glScalef(0.3f, 0.5f, 0.2f);
    glutSolidSphere(0.5f, 20, 20);
    glPopMatrix();
    
    // Kaki kanan (biru)
    if (lighting_on) glColor3f(0.0f, 0.6f, 1.0f); // Tambahkan glColor di sini juga
    glPushMatrix();
    glTranslatef(0.2f, -1.2f, 0.0f);
    glScalef(0.3f, 0.5f, 0.2f);
    glutSolidSphere(0.5f, 20, 20);
    glPopMatrix();
    
    // Sepatu kiri (putih)
    if (lighting_on) glColor3f(1.0f, 1.0f, 1.0f);
    glPushMatrix();
    glTranslatef(-0.2f, -1.4f, 0.025f);
    glScalef(0.2f, 0.1f, 0.2f);
    glutSolidSphere(0.6f, 20, 20);
    glPopMatrix();
    
    // Sepatu kanan (putih)
    if (lighting_on) glColor3f(1.0f, 1.0f, 1.0f); 
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
    float maxTilt = 15.0f; 
    float camSpeed = 0.25f;
    float collisionRadius = 0.5f; 
    
    targetTiltX = 0.0f;
    targetTiltZ = 0.0f;
    
    if (cameraMode == 0) {
        if (keyStates['w'] || keyStates['W']) {
            freeCamY += camSpeed;
        }
        
        if (keyStates['s'] || keyStates['S']) {
            if (freeCamY > 0.5f) {
                freeCamY -= camSpeed;
            }
        }
        
        if (keyStates['a'] || keyStates['A']) {
            camAngle -= 0.15f;
            orientCamera(camAngle);
        }
        
        if (keyStates['d'] || keyStates['D']) {
            camAngle += 0.15f;
            orientCamera(camAngle);
        }
    } else {
        float newX = posX;
        float newY = posY;
        float newZ = posZ;
        bool moved = false;
        
        if (keyStates['w'] || keyStates['W']) {
            newX = posX + speed * sin(angleY * M_PI / 180.0);
            newZ = posZ + speed * cos(angleY * M_PI / 180.0);
            moved = true;
            targetTiltZ = maxTilt; 
        }
        
        if (keyStates['s'] || keyStates['S']) {
            newX = posX - speed * sin(angleY * M_PI / 180.0);
            newZ = posZ - speed * cos(angleY * M_PI / 180.0);
            moved = true;
            targetTiltZ = -maxTilt;
        }
        
        if (keyStates['a'] || keyStates['A']) {
            angleY += 3.0f;
            targetTiltX = -maxTilt; 
        }
        
        if (keyStates['d'] || keyStates['D']) {
            angleY -= 3.0;
            targetTiltX = maxTilt; 
        }
        
        if (keyStates['q'] || keyStates['Q']) {
            newY = posY + speed;
            moved = true;
        }
        
        if (keyStates['e'] || keyStates['E']) {
            newY = posY - speed;
            moved = true;
        }
        
        if (moved) {
            float responseX = 0.0f;
            float responseY = 0.0f;
            float responseZ = 0.0f;
            
            bool horizontalCollision = checkCollision(newX, posY, newZ, collisionRadius, 
                                                   &responseX, &responseY, &responseZ);
            
            posX = newX + responseX;
            posZ = newZ + responseZ;
            
            responseX = 0.0f;
            responseY = 0.0f;
            responseZ = 0.0f;
            
            bool verticalCollision = checkCollision(posX, newY, posZ, collisionRadius,
                                                 &responseX, &responseY, &responseZ);
            
            posY = newY + responseY;
            
            responseX = 0.0f;
            responseY = 0.0f;
            responseZ = 0.0f;
            
            checkCollision(posX, posY, posZ, collisionRadius,
                          &responseX, &responseY, &responseZ);
            
            posX += responseX;
            posY += responseY;
            posZ += responseZ;
        }
        
        if (keyStates['r'] || keyStates['R']) {
            posX = 10.0f;
            posY = 10.0f;
            posZ = 0.0f;
            angleY = 0.0f;
        }
    }
}

// Fungsi untuk memperbarui posisi dan orientasi Doraemon
void updateDoraemon() {
    propellerRotation += 10.0f;
    if (propellerRotation > 360.0f) {
        propellerRotation -= 360.0f;
    }
    
    processKeys();
    
    smoothTilt();
}

// Fungsi untuk mengatur keadaan tombol keyboard
void setKeyState(unsigned char key, bool state) {
    keyStates[key] = state;
}

// Fungsi untuk mengatur mode kamera
void setCameraMode(int mode) {
    cameraMode = mode;
}

// Fungsi untuk mendapatkan mode kamera
int getCameraMode() {
    return cameraMode;
}

// Fungsi untuk memeriksa tabrakan kamera dengan objek di arena
bool checkCameraCollision(float cameraX, float cameraY, float cameraZ, float* responseX, float* responseY, float* responseZ) {
    float cameraRadius = 1.0f;
    
    bool collision = false;
    *responseX = 0.0f;
    *responseY = 0.0f;
    *responseZ = 0.0f;

    float boundarySize = arenaSize / 2;
    
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

// Fungsi untuk mendapatkan posisi kamera X 
float getCameraX() {
    if (cameraMode == 1) {
        float responseX = 0.0f, responseY = 0.0f, responseZ = 0.0f;
        float cameraX = posX - cameraDistance * sin(angleY * M_PI / 180.0);
        if (checkCameraCollision(cameraX, posY, posZ, &responseX, &responseY, &responseZ)) {
            cameraX += responseX;  
        }
        return cameraX;
    } else {
        return freeCamX;
    }
}

// Fungsi untuk mendapatkan posisi kamera Y
float getCameraY() {
    if (cameraMode == 1) {
        return posY + cameraHeight;
    } else {
        return freeCamY;
    }
}

// Fungsi untuk mendapatkan posisi kamera Z
float getCameraZ() {
    if (cameraMode == 1) {
        float responseX = 0.0f, responseY = 0.0f, responseZ = 0.0f;
        float cameraZ = posZ - cameraDistance * cos(angleY * M_PI / 180.0);
        
        if (checkCameraCollision(posX, posY, cameraZ, &responseX, &responseY, &responseZ)) {
            cameraZ += responseZ; 
        }
        return cameraZ;
    } else {
        return freeCamZ;
    }
}

// Fungsi untuk mendapatkan orientasi kamera X
float getLookX() {
    if (cameraMode == 1) {
        return posX;
    } else {
        return freeCamX + camLookX;
    }
}

// Fungsi untuk mendapatkan orientasi kamera Y
float getLookY() {
    if (cameraMode == 1) {
        return posY;
    } else {
        return freeCamY;
    }
}

// Fungsi untuk mendapatkan orientasi kamera Z
float getLookZ() {
    if (cameraMode == 1) {
        return posZ;
    } else {
        return freeCamZ + camLookZ;
    }
}

// Fungsi untuk menginisialisasi Doraemon
void initDoraemon() {
    for (int i = 0; i < 256; i++) {
        keyStates[i] = false;
    }
    
    posX = 10.0;
    posY = 10.0;
    posZ = 0.0;
    angleY = 0.0;
    tiltX = 0.0;
    tiltZ = 0.0;
    targetTiltX = 0.0;
    targetTiltZ = 0.0;
    propellerRotation = 0.0;
    
    camAngle = 0.0f;
    camLookX = 0.0f;
    camLookZ = -1.0f;
    orientCamera(camAngle);
}

// Fungsi resetDoraemonState 
void resetDoraemonState() {
    initDoraemon(); 
    printf("Doraemon state reset.\n");
}

// Fungsi untuk mendapatkan posisi Doraemon X
float getDoraemonX() {
    return posX;
}

// Fungsi untuk mendapatkan posisi Doraemon Y
float getDoraemonY() {
    return posY;
}

// Fungsi untuk mendapatkan posisi Doraemon Z
float getDoraemonZ() {
    return posZ;
}



#endif // DORAEMON_C