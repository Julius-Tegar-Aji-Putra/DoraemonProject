#ifndef GL_STENCIL_BIT
#define GL_STENCIL_BIT 0x00000100
#endif

#include <GL/glut.h>
#include <stdio.h>
#include <math.h>
#include <time.h> 
#include <vector>   
#include <string>   
#include <iomanip>   
#include <sstream>   
#include <chrono>   
#include "doraemon.h"
#include "koin.h"
#include "arena.h"
#include "imageloader.h"

using namespace std;

GLuint texture_tembok_pembatas;
GLuint texture_jalan_paving; 

// ============== Variabel Global untuk Game State ==============
int score = 0;
int collectedCoinsCount = 0;
bool gameIsActive = false; 
bool gameOver = false;
chrono::steady_clock::time_point gameStartTime;
long long gameDurationSeconds = 0; 

const float COLLECTION_RADIUS = 1.5f; 
const int TARGET_COINS_TO_COLLECT = 5; 

bool shadowsEnabled = true;

// Variabel untuk bayangan
GLfloat light_position_shadow[4]; 
GLfloat shadow_plane_normal[3] = {0.0f, 1.0f, 0.0f}; 
GLfloat shadow_plane_point[3] = {0.0f, 0.01f, 0.0f};

GLuint createOpenGLTexture(Image* image) {
    if (image == NULL || image->pixels == NULL) {
        printf("Gagal membuat tekstur OpenGL: data gambar tidak valid.\n");
        return 0;
    }

    GLuint textureId;
    glGenTextures(1, &textureId); 
    glBindTexture(GL_TEXTURE_2D, textureId);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->width, image->height, 
                   0, GL_RGB, GL_UNSIGNED_BYTE, image->pixels); 
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 

    glBindTexture(GL_TEXTURE_2D, 0); 
    
    if (textureId == 0) {
        printf("Gagal menghasilkan ID Tekstur OpenGL.\n");
    } else {
        printf("Tekstur OpenGL berhasil dibuat dari gambar BMP, ID: %u (Lebar: %d, Tinggi: %d)\n", 
               textureId, image->width, image->height);
    }
    return textureId;
}

// Fungsi matriks proyeksi bayangan
void glShadowProjection(const GLfloat light_pos[4], const GLfloat plane_point[3], const GLfloat plane_normal[3]) {
    GLfloat mat[16];
    GLfloat plane_eq[4];
    plane_eq[0] = plane_normal[0];
    plane_eq[1] = plane_normal[1];
    plane_eq[2] = plane_normal[2];
    plane_eq[3] = -(plane_normal[0] * plane_point[0] +
                    plane_normal[1] * plane_point[1] +
                    plane_normal[2] * plane_point[2]);

    GLfloat dot = plane_eq[0] * light_pos[0] +
                  plane_eq[1] * light_pos[1] +
                  plane_eq[2] * light_pos[2] +
                  plane_eq[3] * light_pos[3];

    // Matriks kolom-mayor
    mat[0]  = dot - light_pos[0] * plane_eq[0];
    mat[4]  = 0.0f - light_pos[0] * plane_eq[1];
    mat[8]  = 0.0f - light_pos[0] * plane_eq[2];
    mat[12] = 0.0f - light_pos[0] * plane_eq[3];

    mat[1]  = 0.0f - light_pos[1] * plane_eq[0];
    mat[5]  = dot - light_pos[1] * plane_eq[1];
    mat[9]  = 0.0f - light_pos[1] * plane_eq[2];
    mat[13] = 0.0f - light_pos[1] * plane_eq[3];

    mat[2]  = 0.0f - light_pos[2] * plane_eq[0];
    mat[6]  = 0.0f - light_pos[2] * plane_eq[1];
    mat[10] = dot - light_pos[2] * plane_eq[2];
    mat[14] = 0.0f - light_pos[2] * plane_eq[3];

    mat[3]  = 0.0f - light_pos[3] * plane_eq[0];
    mat[7]  = 0.0f - light_pos[3] * plane_eq[1];
    mat[11] = 0.0f - light_pos[3] * plane_eq[2];
    mat[15] = dot - light_pos[3] * plane_eq[3];

    glMultMatrixf(mat); //
}

// ============== Fungsi Baru untuk Mereset Game ==============
void resetGame() {
    printf("Starting new game / Resetting game...\n");
    score = 0;
    collectedCoinsCount = 0;
    gameIsActive = true;
    gameOver = false;

    initDoraemon();
    resetKoin();


    gameStartTime = chrono::steady_clock::now();
    gameDurationSeconds = 0;
}



// Fungsi untuk inisialisasi
void init() {
    srand(time(NULL)); 

    glClearColor(0.5f, 0.7f, 1.0f, 1.0f); 
    glEnable(GL_DEPTH_TEST); 

    GLfloat ambient[] = { 0.5f, 0.5f, 0.5f, 1.0f }; 
    GLfloat diffuse[] = { 0.7f, 0.7f, 0.7f, 1.0f }; 
    GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f }; 
    GLfloat light0_position_from_setup[] = { 15.0f, 30.0f, 15.0f, 1.0f}; 

    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient); 
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular); 
    glLightfv(GL_LIGHT0, GL_POSITION, light0_position_from_setup); 

    glEnable(GL_LIGHTING); 
    glEnable(GL_LIGHT0); 
    glEnable(GL_COLOR_MATERIAL); 

    light_position_shadow[0] = light0_position_from_setup[0]; 
    light_position_shadow[1] = light0_position_from_setup[1]; 
    light_position_shadow[2] = light0_position_from_setup[2]; 
    light_position_shadow[3] = light0_position_from_setup[3]; 
    
    Image* img_tembok = loadBMP("tembok.bmp"); 
    if (img_tembok != NULL) {
        texture_tembok_pembatas = createOpenGLTexture(img_tembok);
        delete img_tembok;
    } else {
        printf("PERINGATAN UTAMA: Gagal memuat tembok.bmp menggunakan loadBMP!\n");
        texture_tembok_pembatas = 0;
    }

    Image* img_jalan = loadBMP("paving.bmp"); 
    if (img_jalan != NULL) {
        texture_jalan_paving = createOpenGLTexture(img_jalan);
        delete img_jalan; 
    } else {
        printf("PERINGATAN UTAMA: Gagal memuat paving.bmp menggunakan loadBMP!\n");
        texture_jalan_paving = 0;
    }

    resetGame();
    initArena();
}

// Fungsi untuk menggambar scene TANPA bayangan
void drawSceneObjects(float camX, float camY, float camZ) {
    glEnable(GL_LIGHTING); 
    glEnable(GL_DEPTH_TEST); 
    glDisable(GL_BLEND);

    drawArena(camX, camY, camZ); 

    if (gameIsActive || gameOver) { 
        glPushMatrix(); 
        glTranslatef(getDoraemonX(), getDoraemonY(), getDoraemonZ()); 
        drawDoraemon(); 
        glPopMatrix(); 
    }

    if (gameIsActive) { 
        drawKoin(); 
    }
}

// Fungsi untuk menggambar HANYA tanah (untuk Stencil)
void drawGroundForStencil() {
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); 
    glDepthMask(GL_FALSE); 

    float halfArena = getArenaSize(); 
    glBegin(GL_QUADS);
        glVertex3f(-halfArena, shadow_plane_point[1], -halfArena);
        glVertex3f( halfArena, shadow_plane_point[1], -halfArena);
        glVertex3f( halfArena, shadow_plane_point[1],  halfArena);
        glVertex3f(-halfArena, shadow_plane_point[1],  halfArena);
    glEnd();

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); 
    glDepthMask(GL_TRUE); 
}


void drawShadows() {
    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_POLYGON_BIT | GL_STENCIL_BIT); 

    glDisable(GL_LIGHTING);  
    glEnable(GL_DEPTH_TEST); 
    glDepthMask(GL_FALSE);   
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //
    glColor4f(0.15f, 0.15f, 0.15f, 0.6f); 

    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 1, 0xFF); 
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP); 

    // ---- Bayangan Gedung-gedung ----
    const vector<Building>& allBuildings = getArenaBuildings(); 
    for (const auto& b : allBuildings) {
        glPushMatrix(); 
        glShadowProjection(light_position_shadow, shadow_plane_point, shadow_plane_normal); 
        glTranslatef(b.x, b.height / 2.0f, b.z); 
        glScalef(b.width, b.height, b.depth); 
        glutSolidCube(1.0); 
        glPopMatrix(); 
    }

    // ---- Bayangan Doraemon ----
    if (gameIsActive || gameOver) { 
        glPushMatrix(); 
        glShadowProjection(light_position_shadow, shadow_plane_point, shadow_plane_normal); 
        glTranslatef(getDoraemonX(), getDoraemonY(), getDoraemonZ()); 
        drawDoraemon(); 
        glPopMatrix(); 
    }

    // ---- Bayangan Koin-koin ----
    if (gameIsActive) { //
        const vector<KoinData>& allCoins = getAllKoinData(); 
        for (const auto& koin : allCoins) {
            if (koin.isActive) { 
                glPushMatrix(); 
                glShadowProjection(light_position_shadow, shadow_plane_point, shadow_plane_normal); 
                glTranslatef(koin.x, koin.y + koin.yOffset, koin.z); 
                glRotatef(koin.rotation, 0.0f, 1.0f, 0.0f); 
                glRotatef(270.0f, 0.0f, 1.0f, 0.0f); 

                GLUquadricObj *cylinderShadow = gluNewQuadric(); 
                gluQuadricDrawStyle(cylinderShadow, GLU_FILL); 
                float coinRadius = 0.5f; 
                float coinThickness = 0.1f; 
                gluCylinder(cylinderShadow, coinRadius, coinRadius, coinThickness, 20, 1); 
                gluDisk(cylinderShadow, 0.0f, coinRadius, 20, 1); 
                glPushMatrix(); 
                glTranslatef(0.0f, 0.0f, coinThickness); 
                gluDisk(cylinderShadow, 0.0f, coinRadius, 20, 1); 
                glPopMatrix(); 
                gluDeleteQuadric(cylinderShadow); 
                glPopMatrix(); 
            }
        }
    }

    glDepthMask(GL_TRUE); 
    glPopAttrib(); 
}

void drawBuildingShadow(const Building& b) {
    glPushMatrix();
    glTranslatef(b.x, b.height / 2.0f, b.z);
    glScalef(b.width, b.height, b.depth);
    glutSolidCube(1.0);
    glPopMatrix();
}

void drawCoinShadow() {
    GLUquadricObj *cylinderShadow = gluNewQuadric();
    gluQuadricDrawStyle(cylinderShadow, GLU_FILL); 
    float coinRadius = 0.35f;
    float coinThickness = 0.1f;
    gluCylinder(cylinderShadow, coinRadius, coinRadius, coinThickness, 20, 1);
    gluDisk(cylinderShadow, 0.0f, coinRadius, 20, 1);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, coinThickness);
    gluDisk(cylinderShadow, 0.0f, coinRadius, 20, 1);
    glPopMatrix();
    gluDeleteQuadric(cylinderShadow);
}

// Fungsi untuk menampilkan scene
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    float currentCamX = getCameraX(); 
    float currentCamY = getCameraY(); 
    float currentCamZ = getCameraZ(); 
  
    gluLookAt(getCameraX(), getCameraY(), getCameraZ(),    
              getLookX(), getLookY(), getLookZ(),          
              0.0, 1.0, 0.0);                            

    glEnable(GL_LIGHTING); 
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);    
    
    drawArena(currentCamX, currentCamY, currentCamZ); 
    
    if (gameIsActive || gameOver) {
        glPushMatrix();
        glTranslatef(getDoraemonX(), getDoraemonY(), getDoraemonZ());
        drawDoraemon();
        glPopMatrix();
    }

    if (gameIsActive) {
        glEnable(GL_LIGHTING);
        drawKoin();
    }

    drawArena(currentCamX, currentCamY, currentCamZ);

    if (shadowsEnabled) {
        glEnable(GL_STENCIL_TEST); 
        glDisable(GL_DEPTH_TEST);  
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); 
        
        glStencilFunc(GL_ALWAYS, 1, 0xFF); 
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); 

        drawGroundAndRoofsForStencil();

        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); 
        glEnable(GL_DEPTH_TEST);
    }
    
    if (shadowsEnabled) {
        glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_POLYGON_BIT | GL_STENCIL_BIT);

        glDisable(GL_LIGHTING);  
        glEnable(GL_DEPTH_TEST);  
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.15f, 0.15f, 0.15f, 0.6f); 

        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(-1.0f, -1.0f); 

        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_EQUAL, 1, 0xFF); 
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

        GLfloat ground_plane_point[3] = {0.0f, 0.01f, 0.0f};
        GLfloat plane_normal[3] = {0.0f, 1.0f, 0.0f};      

        // ---- Bayangan Gedung (Tetap di Tanah) ----
        glPushMatrix();
        glShadowProjection(light_position_shadow, ground_plane_point, plane_normal);
        const std::vector<Building>& allBuildings = getArenaBuildings(); 
        for (const auto& b : allBuildings) {
             drawBuildingShadow(b); 
        }
        glPopMatrix();

        // ---- Bayangan Doraemon (DINAMIS) ---- 
        if (gameIsActive || gameOver) { 
            float doraemonX = getDoraemonX(); 
            float doraemonZ = getDoraemonZ(); 
            
            float plane_height = getBuildingRoofHeight(doraemonX, doraemonZ);
            GLfloat current_plane_point[3] = {0.0f, plane_height + 0.01f, 0.0f}; 

            glPushMatrix();
            glShadowProjection(light_position_shadow, current_plane_point, plane_normal); 
            

            glPushMatrix();
            glTranslatef(doraemonX, getDoraemonY(), doraemonZ); 
            drawDoraemon(); 
            glPopMatrix();

            glPopMatrix();
        }

        // ---- Bayangan Koin-koin (Dinamis) ----
        const vector<KoinData>& allCoins = getAllKoinData(); 
        for (const auto& koin : allCoins) { 
            if (koin.isActive) { 
                float plane_height = getBuildingRoofHeight(koin.x, koin.z); 
                GLfloat current_plane_point[3] = {0.0f, plane_height + 0.01f, 0.0f};

                glPushMatrix(); 
                glShadowProjection(light_position_shadow, current_plane_point, plane_normal); 

                glTranslatef(koin.x, koin.y + koin.yOffset, koin.z); 
                glRotatef(koin.rotation, 0.0f, 1.0f, 0.0f); 
                glRotatef(270.0f, 0.0f, 1.0f, 0.0f); 

                drawCoinShadow(); 

                glPopMatrix(); 
            }
        }

        glDisable(GL_POLYGON_OFFSET_FILL); 
        glPopAttrib(); 
        glDisable(GL_STENCIL_TEST); 
    }

    // ============== Setup untuk rendering UI (Teks) ==============
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, glutGet(GLUT_WINDOW_WIDTH), 0.0, glutGet(GLUT_WINDOW_HEIGHT));
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_LIGHTING); 
    glDisable(GL_DEPTH_TEST); 

    void* uiFont = GLUT_BITMAP_HELVETICA_18; 
    float uiFontHeight = 18.0f; 
    float uiPadding = 5.0f; 
    float blackR = 0.0f, blackG = 0.0f, blackB = 0.0f;    

    // Warna teks default (putih)
    float whiteR = 1.0f, whiteG = 1.0f, whiteB = 1.0f;
    
    // --- Tampilkan Skor ---
    char scoreText[50];
    sprintf(scoreText, "Skor: %d", score);
    drawArenaText(10, glutGet(GLUT_WINDOW_HEIGHT) - (uiFontHeight + uiPadding), scoreText, 
                  whiteR, whiteG, whiteB, uiFont); // Teks hitam

    // --- Tampilkan Timer ---
    char timerText[50]; 
    if (gameIsActive) {
        auto now = std::chrono::steady_clock::now();
        long long elapsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(now - gameStartTime).count();
        int hours = elapsedSeconds / 3600;
        int minutes = (elapsedSeconds % 3600) / 60;
        int seconds = elapsedSeconds % 60;
        char timerText[50];
        sprintf(timerText, "Waktu: %02d:%02d:%02d", hours, minutes, seconds);
        drawArenaText(10, glutGet(GLUT_WINDOW_HEIGHT) - 2 * (uiFontHeight + uiPadding), timerText, whiteR, whiteG, whiteB, uiFont);
    } else if (gameOver) {
        // Tampilkan waktu final jika game over
        int hours = gameDurationSeconds / 3600;
        int minutes = (gameDurationSeconds % 3600) / 60;
        int seconds = gameDurationSeconds % 60;
        char timerText[50];
        sprintf(timerText, "Waktu Selesai: %02d:%02d:%02d", hours, minutes, seconds);
        drawArenaText(10, glutGet(GLUT_WINDOW_HEIGHT) - 2 * (uiFontHeight + uiPadding), timerText, whiteR, whiteG, whiteB, uiFont);
    }

    // --- Tampilkan Status Bayangan (Sudah Anda tambahkan sebelumnya) ---
    char shadowStatusText[50];
    sprintf(shadowStatusText, "Bayangan (B): %s", (shadowsEnabled ? "Aktif" : "Nonaktif"));
    drawArenaText(10, glutGet(GLUT_WINDOW_HEIGHT) - 3 * (uiFontHeight + uiPadding), shadowStatusText, whiteR, whiteG, whiteB, uiFont);


    // --- Tampilkan Pesan Game Over dan Tombol Restart ---
    if (gameOver) {
        void* gameOverFont = GLUT_BITMAP_TIMES_ROMAN_24; 
        float gameOverFontHeight = 24.0f; 
        float bgPaddingX_gameOver = 8.0f; 
        float bgPaddingY_gameOver = 5.0f; 

        float rTextGameOver = 0.0f, gTextGameOver = 0.0f, bTextGameOver = 0.0f;
        float rBgGameOver = 1.0f, gBgGameOver = 1.0f, bBgGameOver = 1.0f, aBgGameOver = 1.0f;

        const char* msg1 = "Permainan Selesai! Anda Menang!";
        const char* msg2 = "Tekan 'R' untuk Mulai Ulang";
        
        float screenWidth = glutGet(GLUT_WINDOW_WIDTH);
        float screenHeight = glutGet(GLUT_WINDOW_HEIGHT);

        int msg1Width = 0;
        for (const char* c = msg1; *c; c++) {
            msg1Width += glutBitmapWidth(gameOverFont, *c);
        }
        int msg2Width = 0;
        for (const char* c = msg2; *c; c++) {
            msg2Width += glutBitmapWidth(gameOverFont, *c);
        }

        float msg1X = (screenWidth - msg1Width) / 2.0f;
        float msg1Y = screenHeight / 2.0f + gameOverFontHeight / 2.0f; // Sedikit di atas tengah

        float msg2X = (screenWidth - msg2Width) / 2.0f;
        float msg2Y = screenHeight / 2.0f - gameOverFontHeight - uiPadding; // Sedikit di bawah tengah

        if (aBgGameOver > 0.0f && msg1Width > 0) {
            glColor4f(rBgGameOver, gBgGameOver, bBgGameOver, aBgGameOver);
            glBegin(GL_QUADS);
                glVertex2f(msg1X - bgPaddingX_gameOver, msg1Y - bgPaddingY_gameOver);
                glVertex2f(msg1X + msg1Width + bgPaddingX_gameOver, msg1Y - bgPaddingY_gameOver);
                glVertex2f(msg1X + msg1Width + bgPaddingX_gameOver, msg1Y + gameOverFontHeight + bgPaddingY_gameOver);
                glVertex2f(msg1X - bgPaddingX_gameOver, msg1Y + gameOverFontHeight + bgPaddingY_gameOver);
            glEnd();
        }

        drawArenaText(msg1X, msg1Y, msg1, rTextGameOver, gTextGameOver, bTextGameOver, gameOverFont);

        if (aBgGameOver > 0.0f && msg2Width > 0) {
            glColor4f(rBgGameOver, gBgGameOver, bBgGameOver, aBgGameOver);
            glBegin(GL_QUADS);
                glVertex2f(msg2X - bgPaddingX_gameOver, msg2Y - bgPaddingY_gameOver);
                glVertex2f(msg2X + msg2Width + bgPaddingX_gameOver, msg2Y - bgPaddingY_gameOver);
                glVertex2f(msg2X + msg2Width + bgPaddingX_gameOver, msg2Y + gameOverFontHeight + bgPaddingY_gameOver);
                glVertex2f(msg2X - bgPaddingX_gameOver, msg2Y + gameOverFontHeight + bgPaddingY_gameOver);
            glEnd();
        }
        // Gambar teks msg2
        drawArenaText(msg2X, msg2Y, msg2, rTextGameOver, gTextGameOver, bTextGameOver, gameOverFont);
    }
    
    // Panggil displayControlInfo yang sudah dimodifikasi (untuk panduan keyboard)
    if (gameIsActive || gameOver) { 
         displayControlInfo(); 
    }

    // Kembalikan settings
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    
    glMatrixMode(GL_PROJECTION); 
    glPopMatrix();              
    
    glMatrixMode(GL_MODELVIEW);  
    glPopMatrix();              
    
    glutSwapBuffers();
}

// Fungsi untuk menangani perubahan ukuran window
void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (float)w/(float)h, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

// ============== Fungsi keyboard() dimodifikasi ==============
void keyboard(unsigned char key, int x, int y) {
    if (gameIsActive) { 
        setKeyState(key, true); 
    }

    // Toggle mode kamera (bisa tetap di luar gameIsActive jika diinginkan)
    if (key == '0') {
        setCameraMode(0);
        printf("Camera Mode: Free Camera\n");
    } else if (key == '1') {
        setCameraMode(1);
        printf("Camera Mode: Third Person\n");
    }

    // Restart game
    if ((key == 'r' || key == 'R') && gameOver) {
        resetGame();
    }

    if (key == 'b' || key == 'B') { // Tombol 'B' untuk toggle bayangan
        shadowsEnabled = !shadowsEnabled; 
        if (shadowsEnabled) {
            printf("Bayangan: AKTIF\n");
        } else {
            printf("Bayangan: NONAKTIF\n");
        }
        glutPostRedisplay(); 
    }
}

// Fungsi untuk menangani input keyboard (saat dilepas)
void keyboardUp(unsigned char key, int x, int y) {
    setKeyState(key, false);
}

// ============== Fungsi idle() dimodifikasi ==============
void idle() {
    if (gameIsActive) { 
        updateDoraemon();
        updateKoin();

        // --- Logika Deteksi Kolisi Koin ---
        float dX = getDoraemonX();
        float dY = getDoraemonY(); 
        float dZ = getDoraemonZ();

        const std::vector<KoinData>& coins = getAllKoinData();
        for (size_t i = 0; i < coins.size(); ++i) {
            if (coins[i].isActive) {
                float distSq = (dX - coins[i].x) * (dX - coins[i].x) +
                               (dY - (coins[i].y + coins[i].yOffset)) * (dY - (coins[i].y + coins[i].yOffset)) + // y koin adalah y_dasar + yOffset
                               (dZ - coins[i].z) * (dZ - coins[i].z);
                
                if (distSq < (COLLECTION_RADIUS * COLLECTION_RADIUS)) {
                    setKoinCollected(i); // Fungsi dari koin.cpp
                    score += 10;
                    collectedCoinsCount++;
                    printf("Koin dikumpulkan! Skor: %d, Terkumpul: %d\n", score, collectedCoinsCount);
                }
            }
        }

        // --- Cek Kondisi Menang ---
        if (collectedCoinsCount >= TARGET_COINS_TO_COLLECT) {
            gameIsActive = false;
            gameOver = true;
            auto now = std::chrono::steady_clock::now();
            gameDurationSeconds = std::chrono::duration_cast<std::chrono::seconds>(now - gameStartTime).count();
            printf("Semua koin terkumpul! Permainan Selesai!\n");
            printf("Waktu: %lld detik\n", gameDurationSeconds);
        }
    }
    glutPostRedisplay();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
    int windowWidth = 1000;
    int windowHeight = 600;
    glutInitWindowSize(windowWidth, windowHeight);
    int screenWidth = glutGet(GLUT_SCREEN_WIDTH);
    int screenHeight = glutGet(GLUT_SCREEN_HEIGHT);
    int posX = (screenWidth - windowWidth) / 2;
    int posY = (screenHeight - windowHeight) / 2;
    glutInitWindowPosition(posX, posY); 
    glutCreateWindow("Doraemon 3D dengan Baling-Baling Bambu - Terbang");
    
    init();
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(specialKeyPressed);
    glutIdleFunc(idle);
    glutMainLoop();
    return 0;
}