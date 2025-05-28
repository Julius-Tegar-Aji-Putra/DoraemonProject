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
bool gameIsActive = false; // Mulai tidak aktif sampai game benar-benar dimulai/di-reset
bool gameOver = false;
chrono::steady_clock::time_point gameStartTime;
long long gameDurationSeconds = 0; // Untuk menyimpan durasi total saat game over

const float COLLECTION_RADIUS = 1.5f; // Sesuaikan radius ini! (Doraemon radius + Koin radius)
                                      // Anda mungkin perlu getter untuk radius Doraemon atau koin
const int TARGET_COINS_TO_COLLECT = 5; // Sesuai dengan JUMLAH_KOIN_PER_GAME di koin.cpp

bool shadowsEnabled = false;

// Variabel untuk bayangan
GLfloat light_position_shadow[4]; 
GLfloat shadow_plane_normal[3] = {0.0f, 1.0f, 0.0f}; 
GLfloat shadow_plane_point[3] = {0.0f, -0.05f, 0.0f};

GLuint createOpenGLTexture(Image* image) {
    if (image == NULL || image->pixels == NULL) {
        printf("Gagal membuat tekstur OpenGL: data gambar tidak valid.\n");
        return 0;
    }

    GLuint textureId;
    glGenTextures(1, &textureId); 
    glBindTexture(GL_TEXTURE_2D, textureId);
    
    // Menggunakan data piksel dari objek Image*
    // imageloader.cpp dari PDF Anda memuat gambar sebagai 24-bit RGB
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->width, image->height, 
                   0, GL_RGB, GL_UNSIGNED_BYTE, image->pixels); // [cite: 42]
    
    // Set parameter wrapping (bagaimana tekstur diulang)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    // Set parameter filtering (bagaimana tekstur diperbesar/diperkecil)
    // Sesuai dengan mainContohPenggunaan.cpp[cite: 59], kita tidak pakai mipmap di sini
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // [cite: 59]
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // [cite: 59]
    
    // Hapus atau komentari glGenerateMipmap jika tidak terdefinisi:
    // glGenerateMipmap(GL_TEXTURE_2D); 

    glBindTexture(GL_TEXTURE_2D, 0); 
    
    if (textureId == 0) {
        printf("Gagal menghasilkan ID Tekstur OpenGL.\n");
    } else {
        // Modifikasi pesan agar lebih umum, karena nama file bisa berbeda
        printf("Tekstur OpenGL berhasil dibuat dari gambar BMP, ID: %u (Lebar: %d, Tinggi: %d)\n", 
               textureId, image->width, image->height);
    }
    return textureId;
}

void glShadowProjection(const GLfloat light_pos[3], const GLfloat plane_point[3], const GLfloat plane_normal[3]) {
    GLfloat d, c;
    GLfloat mat[16];

    // d = N dot L (N adalah normal bidang, L adalah posisi cahaya)
    d = plane_normal[0] * light_pos[0] + 
        plane_normal[1] * light_pos[1] + 
        plane_normal[2] * light_pos[2];
    
    // c = (E dot N) - d (E adalah titik pada bidang)
    c = plane_point[0] * plane_normal[0] + 
        plane_point[1] * plane_normal[1] + 
        plane_point[2] * plane_normal[2] - d;

    // Kolom-mayor untuk OpenGL
    // Kolom 1
    mat[0]  = light_pos[0] * plane_normal[0] + c;  // Ini berbeda dari standar, mengikuti PDF
    mat[1]  = plane_normal[0] * light_pos[1];      // Perhatikan urutan pengisian untuk kolom-mayor
    mat[2]  = plane_normal[0] * light_pos[2];      // PDF mungkin menuliskan dlm notasi baris-mayor
    mat[3]  = plane_normal[0];                     // atau formulanya spesifik

    // Kolom 2
    mat[4]  = plane_normal[1] * light_pos[0];
    mat[5]  = light_pos[1] * plane_normal[1] + c;
    mat[6]  = plane_normal[1] * light_pos[2];
    mat[7]  = plane_normal[1];

    // Kolom 3
    mat[8]  = plane_normal[2] * light_pos[0];
    mat[9]  = plane_normal[2] * light_pos[1];
    mat[10] = light_pos[2] * plane_normal[2] + c;
    mat[11] = plane_normal[2];

    // Kolom 4
    mat[12] = -light_pos[0] * c - light_pos[0] * d;
    mat[13] = -light_pos[1] * c - light_pos[1] * d;
    mat[14] = -light_pos[2] * c - light_pos[2] * d;
    mat[15] = -d;

    // Setelah diperiksa ulang dengan PDF halaman 11 (mat[0]=l[0]*n[0]+c menjadi mat[0]=dot - L.x*N.x etc.)
    // Sepertinya implementasi PDF yang diketik di "Tugas 5 No 1.cpp" mengikuti pola tertentu.
    // Mari kita gunakan persis seperti di "Tugas 5 No 1.cpp" untuk konsistensi dengan contoh kerja:
    // (elemen mat[] adalah kolom-mayor: mat[0], mat[1], mat[2], mat[3] adalah kolom pertama dst.)
    // Mengacu pada susunan di Tugas 5 No 1.cpp:
    mat[0] = light_pos[0] * plane_normal[0] + c;    mat[4] = plane_normal[1] * light_pos[0];      mat[8] = plane_normal[2] * light_pos[0];      mat[12] = -light_pos[0] * c - light_pos[0] * d;
    mat[1] = plane_normal[0] * light_pos[1];        mat[5] = light_pos[1] * plane_normal[1] + c;  mat[9] = plane_normal[2] * light_pos[1];      mat[13] = -light_pos[1] * c - light_pos[1] * d;
    mat[2] = plane_normal[0] * light_pos[2];        mat[6] = plane_normal[1] * light_pos[2];      mat[10] = light_pos[2] * plane_normal[2] + c; mat[14] = -light_pos[2] * c - light_pos[2] * d;
    mat[3] = plane_normal[0];                       mat[7] = plane_normal[1];                     mat[11] = plane_normal[2];                    mat[15] = -d;


    glMultMatrixf(mat);
}

// ============== Fungsi Baru untuk Mereset Game ==============
void resetGame() {
    printf("Starting new game / Resetting game...\n");
    score = 0;
    collectedCoinsCount = 0;
    gameIsActive = true;
    gameOver = false;

    resetKoin();
    initDoraemon();


    gameStartTime = std::chrono::steady_clock::now();
    gameDurationSeconds = 0;
}



// Fungsi untuk inisialisasi
void init() {
    srand(time(NULL));

    glClearColor(0.5f, 0.7f, 1.0f, 1.0f);  // Warna langit
    glEnable(GL_DEPTH_TEST);
    
    // Atur pencahayaan
    GLfloat ambient[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    GLfloat diffuse[] = { 0.7f, 0.7f, 0.7f, 1.0f };
    GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat light0_position_from_setup[] = { 5.0f, 10.0f, 5.0f,0.0f};
    
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light0_position_from_setup);
    
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);

    light_position_shadow[0] = light0_position_from_setup[0];
    light_position_shadow[1] = 30.0f;
    light_position_shadow[2] = light0_position_from_setup[2];
    light_position_shadow[3] = 1.0f;
    
    Image* img_tembok = loadBMP("tembok.bmp"); // PASTIKAN INI FILE .BMP 24-BIT Anda [cite: 44]
    if (img_tembok != NULL) {
        texture_tembok_pembatas = createOpenGLTexture(img_tembok);
        delete img_tembok; // Penting: Hapus objek Image setelah data di-upload ke OpenGL [cite: 44]
    } else {
        printf("PERINGATAN UTAMA: Gagal memuat tembok.bmp menggunakan loadBMP!\n");
        texture_tembok_pembatas = 0;
    }

    Image* img_jalan = loadBMP("paving.bmp"); // PASTIKAN INI FILE .BMP 24-BIT Anda
    if (img_jalan != NULL) {
        texture_jalan_paving = createOpenGLTexture(img_jalan);
        delete img_jalan; 
    } else {
        printf("PERINGATAN UTAMA: Gagal memuat paving.bmp menggunakan loadBMP!\n");
        texture_jalan_paving = 0;
    }

    // Inisialisasi Doraemon dan Koin
    initArena();
    resetGame();
}

// Fungsi untuk menampilkan scene
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    float currentCamX = getCameraX(); 
    float currentCamY = getCameraY(); 
    float currentCamZ = getCameraZ(); 
    // Atur kamera berdasarkan data dari modul Doraemon
    gluLookAt(getCameraX(), getCameraY(), getCameraZ(),    // Posisi kamera
              getLookX(), getLookY(), getLookZ(),          // Titik yang dilihat
              0.0, 1.0, 0.0);                             // Vektor up

    glEnable(GL_LIGHTING); 
    glEnable(GL_DEPTH_TEST);
    
    // Gambar bidang datar
    drawArena(currentCamX, currentCamY, currentCamZ); 
    
    // Gambar Doraemon di posisi yang ditentukan
    if (gameIsActive || gameOver) {
        glPushMatrix();
        glTranslatef(getDoraemonX(), getDoraemonY(), getDoraemonZ());
        drawDoraemon();
        glPopMatrix();
    }
    // Gambar koin di posisi yang ditentukan
    if (gameIsActive) {
        glEnable(GL_LIGHTING);
        drawKoin();
    }

    if (shadowsEnabled) {
        glDisable(GL_LIGHTING);  // Bayangan tidak dipengaruhi cahaya
        glEnable(GL_DEPTH_TEST); // Agar bayangan tidak "bertarung" dengan lantai
                                  // Alternatif: glEnable(GL_DEPTH_TEST) dan gambar bayangan sedikit di atas lantai (Y+epsilon)
                                  // atau gunakan glPolygonOffset. Untuk kesederhanaan, kita disable dulu.

        glColor4f(0.2f, 0.2f, 0.2f, 0.65f); // Warna bayangan: abu-abu gelap transparan
                                            // Sesuaikan alpha (nilai ke-4) untuk intensitas bayangan

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Blending standar untuk transparansi

        // Stencil buffer (opsional, untuk mencegah bayangan ganda jika ada beberapa sumber cahaya atau multiple pass)
        // Untuk kasus ini (satu sumber cahaya, satu pass bayangan per objek), mungkin tidak perlu.
        // Jika Anda menggunakan stencil, Anda akan clear stencil buffer, set stencil func/op
        // untuk menandai area lantai, lalu gambar bayangan hanya jika stencil pass.

        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(-1.0f, -1.0f);

        // ---- Bayangan Gedung-gedung ----
        const std::vector<Building>& allBuildings = getArenaBuildings(); // Fungsi dari arena.cpp
        for (const auto& b : allBuildings) {
            glPushMatrix();
            // Terapkan matriks proyeksi bayangan ke matriks ModelView saat ini
            glShadowProjection(light_position_shadow, shadow_plane_point, shadow_plane_normal);
            
            // Lakukan transformasi yang SAMA PERSIS seperti saat menggambar gedung aslinya
            // (dari arena.cpp, fungsi drawBuilding utama, sebelum scaling bentuk dasar)
            glTranslatef(b.x, b.height / 2.0f, b.z);
            glScalef(b.width, b.height, b.depth);
            glutSolidCube(1.0); // Gambar bentuk dasar gedung untuk bayangan
            glPopMatrix();
        }

        // ---- Bayangan Doraemon ----
        if (gameIsActive || gameOver) { // Hanya gambar bayangan jika Doraemon ada
            glPushMatrix();
            glShadowProjection(light_position_shadow, shadow_plane_point, shadow_plane_normal);
            
            // Transformasi Doraemon (SAMA PERSIS seperti di blok gambar Doraemon di atas)
            glTranslatef(getDoraemonX(), getDoraemonY(), getDoraemonZ());
            drawDoraemon(); 
            glPopMatrix();
        }

        // ---- Bayangan Koin-koin ----
        if (gameIsActive) { // Hanya gambar bayangan jika koin aktif
            const std::vector<KoinData>& allCoins = getAllKoinData(); // Fungsi dari koin.cpp
            for (const auto& koin : allCoins) {
                if (koin.isActive) {
                    glPushMatrix();
                    glShadowProjection(light_position_shadow, shadow_plane_point, shadow_plane_normal);

                    // Transformasi Koin (SAMA PERSIS seperti di drawSingleKoin di koin.cpp)
                    glTranslatef(koin.x, koin.y + koin.yOffset, koin.z);
                    glRotatef(koin.rotation, 0.0f, 1.0f, 0.0f);
                    glRotatef(270.0f, 0.0f, 1.0f, 0.0f); 

                    // Gambar bentuk dasar koin untuk bayangan
                    GLUquadricObj *cylinderShadow = gluNewQuadric();
                    float coinRadius = 0.35f; 
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
        
        // Kembalikan state OpenGL
        glDisable(GL_POLYGON_OFFSET_FILL);
        glPopAttrib();
        // glEnable(GL_DEPTH_TEST); // Aktifkan kembali jika sebelumnya dinonaktifkan untuk bayangan
        // glEnable(GL_LIGHTING); // Ini sudah diaktifkan di awal untuk objek utama,
                                 // dan akan diatur lagi oleh UI atau frame berikutnya.
    }
    
    // ============== Setup untuk rendering UI (Teks) ==============
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    // Sesuaikan nilai ortho dengan ukuran window Anda (misal 800x600)
    gluOrtho2D(0.0, glutGet(GLUT_WINDOW_WIDTH), 0.0, glutGet(GLUT_WINDOW_HEIGHT));
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_LIGHTING); // Nonaktifkan lighting untuk teks UI
    glDisable(GL_DEPTH_TEST); // Nonaktifkan depth test untuk teks UI
    
    // --- Tampilkan Skor ---
    char scoreText[50];
    sprintf(scoreText, "Skor: %d", score);
    drawArenaText(10, glutGet(GLUT_WINDOW_HEIGHT) - 20, scoreText); // Gunakan drawArenaText
    
    // --- Tampilkan Timer ---
    if (gameIsActive) {
        auto now = std::chrono::steady_clock::now();
        long long elapsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(now - gameStartTime).count();
        int hours = elapsedSeconds / 3600;
        int minutes = (elapsedSeconds % 3600) / 60;
        int seconds = elapsedSeconds % 60;
        char timerText[50];
        sprintf(timerText, "Waktu: %02d:%02d:%02d", hours, minutes, seconds);
        drawArenaText(10, glutGet(GLUT_WINDOW_HEIGHT) - 40, timerText);
    } else if (gameOver) {
        // Tampilkan waktu final jika game over
        int hours = gameDurationSeconds / 3600;
        int minutes = (gameDurationSeconds % 3600) / 60;
        int seconds = gameDurationSeconds % 60;
        char timerText[50];
        sprintf(timerText, "Waktu Selesai: %02d:%02d:%02d", hours, minutes, seconds);
        drawArenaText(10, glutGet(GLUT_WINDOW_HEIGHT) - 40, timerText);
    }

    // --- Tampilkan Status Bayangan (Sudah Anda tambahkan sebelumnya) ---
    char shadowStatusText[50];
    sprintf(shadowStatusText, "Bayangan (B): %s", (shadowsEnabled ? "Aktif" : "Nonaktif"));
    drawArenaText(10, glutGet(GLUT_WINDOW_HEIGHT) - 60, shadowStatusText); // Pastikan posisi Y sesuai

    // --- Tampilkan Pesan Game Over dan Tombol Restart ---
    if (gameOver) {
        drawArenaText(glutGet(GLUT_WINDOW_WIDTH) / 2 - 100, glutGet(GLUT_WINDOW_HEIGHT) / 2 + 20, "Permainan Selesai! Anda Menang!");
        drawArenaText(glutGet(GLUT_WINDOW_WIDTH) / 2 - 100, glutGet(GLUT_WINDOW_HEIGHT) / 2, "Tekan 'R' untuk Mulai Ulang");
    }
    
    // Tampilkan informasi kontrol (jika masih relevan atau pindahkan ke kondisi gameIsActive)
    if (gameIsActive) { // Hanya tampilkan info kontrol jika game aktif
         displayControlInfo(); // Fungsi dari doraemon.cpp/doraemon.h
    }

    // Kembalikan settings
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
    
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
    if (gameIsActive) { // Hanya proses input game jika aktif
        setKeyState(key, true); // Dari doraemon.cpp
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
        shadowsEnabled = !shadowsEnabled; // Balik nilainya (true jadi false, false jadi true)
        if (shadowsEnabled) {
            printf("Bayangan: AKTIF\n");
        } else {
            printf("Bayangan: NONAKTIF\n");
        }
        glutPostRedisplay(); // Minta OpenGL untuk menggambar ulang scene
    }
}

// Fungsi untuk menangani input keyboard (saat dilepas)
void keyboardUp(unsigned char key, int x, int y) {
    setKeyState(key, false);
}

// ============== Fungsi idle() dimodifikasi ==============
void idle() {
    if (gameIsActive) { // Hanya update jika game sedang aktif
        updateDoraemon();
        updateKoin();

        // --- Logika Deteksi Kolisi Koin ---
        float dX = getDoraemonX();
        float dY = getDoraemonY(); // Perhatikan Y Doraemon, mungkin perlu offset
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
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
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