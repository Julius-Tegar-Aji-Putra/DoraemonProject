#ifndef KOIN_C
#define KOIN_C

#include "koin.h"
#include <math.h>
#include <GL/glut.h>
#include <stdio.h>
#include <vector>   
#include <stdlib.h>
#include <time.h>  
#include "arena.h" 

using namespace std;

static vector<KoinData> daftarSemuaPosisiKoinPotensial;
static vector<KoinData> koinAktifDiGame; 
static const int JUMLAH_KOIN_PER_GAME = 10; 

// Variabel untuk koin
static float coinX = 2.0, coinY = 0.0, coinZ = 0.0;  // Posisi koin (di sebelah Doraemon)
static float coinRotation = 0.0;                     // Rotasi koin
static float coinYOffset = 0.0;                      // Offset Y untuk animasi naik-turun
static float coinYDirection = 1.0f;                  // Arah gerakan naik-turun
static float coinYSpeed = 0.00055;                  // Kecepatan naik-turun
static float coinYMax = 0.25;                         // Ketinggian maksimum

// Fungsi untuk memperbarui koin
void updateKoin() {
    for (size_t i = 0; i < koinAktifDiGame.size(); ++i) {
        if (koinAktifDiGame[i].isActive) {
            // Rotasi koin
            koinAktifDiGame[i].rotation += 10.0f; 
            if (koinAktifDiGame[i].rotation > 360.0f) {
                koinAktifDiGame[i].rotation -= 360.0f;
            }

            // Animasi naik-turun koin
            koinAktifDiGame[i].yOffset += coinYSpeed * koinAktifDiGame[i].yDirection;
            if (koinAktifDiGame[i].yOffset > coinYMax || koinAktifDiGame[i].yOffset < 0.0f) {
                koinAktifDiGame[i].yDirection *= -1.0f; // Balik arah
            }
        }
    }
}

// Fungsi untuk menggambar SATU koin (helper, bisa dipanggil berulang)
static void drawSingleKoin(const KoinData& koin) {
    glPushMatrix();

    glTranslatef(koin.x, koin.y + koin.yOffset, koin.z);
    
 
    glRotatef(koin.rotation, 0.0f, 1.0f, 0.0f);
    
    glColor3f(1.0f, 0.84f, 0.0f);  // Warna emas
    
    GLUquadricObj *cylinder = gluNewQuadric();
    gluQuadricDrawStyle(cylinder, GLU_FILL);
    
    glRotatef(270.0f, 0.0f, 1.0f, 0.0f); 
    
    float coinRadius = 0.5f; 
    float coinThickness = 0.1f;
    
    gluCylinder(cylinder, coinRadius, coinRadius, coinThickness, 30, 2); 
    gluDisk(cylinder, 0.0f, coinRadius, 30, 1); 
    
    glPushMatrix();
    glColor3f(0.8f, 0.7f, 0.0f); // Warna emas lebih gelap
    glTranslatef(0.0f, 0.0f, -coinThickness/2);
    gluDisk(cylinder, coinRadius-0.1f, coinRadius, 30, 1); 
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.8f, 0.7f, 0.0f); // Warna emas lebih gelap
    glTranslatef(0.0f, 0.0f, coinThickness/2);
    gluDisk(cylinder, coinRadius-0.1f, coinRadius, 30, 1);
    glPopMatrix();
    
    gluDeleteQuadric(cylinder);
    glPopMatrix();
}

// Fungsi untuk mengisi daftar semua posisi koin potensial
static void populateMasterCoinList() {
    daftarSemuaPosisiKoinPotensial.clear();
    
    const vector<Building>& semuaGedung = getArenaBuildings();

    float y_dasar_tanah = 0.5f; 
    float y_offset_diatas_gedung = 0.5f; 

    for (const auto& gedung : semuaGedung) {
        daftarSemuaPosisiKoinPotensial.push_back({
            gedung.x, 
            gedung.height + y_offset_diatas_gedung, 
            gedung.z, 
            false, 0.0f, 0.0f, 1.0f
        });
    }


    // Kuadran 1 (X positif, Z positif)
    daftarSemuaPosisiKoinPotensial.push_back({10.0f, y_dasar_tanah, 10.0f, false, 0.0f, 0.0f, 1.0f}); 
    daftarSemuaPosisiKoinPotensial.push_back({2.0f,  y_dasar_tanah, 2.0f,  false, 0.0f, 0.0f, 1.0f});  
    daftarSemuaPosisiKoinPotensial.push_back({20.0f, y_dasar_tanah, 8.0f,  false, 0.0f, 0.0f, 1.0f});
    daftarSemuaPosisiKoinPotensial.push_back({8.0f,  y_dasar_tanah, 20.0f, false, 0.0f, 0.0f, 1.0f});
    daftarSemuaPosisiKoinPotensial.push_back({22.0f, y_dasar_tanah, 22.0f, false, 0.0f, 0.0f, 1.0f}); 
    daftarSemuaPosisiKoinPotensial.push_back({1.0f,  y_dasar_tanah, 12.0f, false, 0.0f, 0.0f, 1.0f});
    daftarSemuaPosisiKoinPotensial.push_back({12.0f, y_dasar_tanah, 1.0f,  false, 0.0f, 0.0f, 1.0f});


    // Kuadran 2 (X negatif, Z positif)
    daftarSemuaPosisiKoinPotensial.push_back({-10.0f, y_dasar_tanah, 10.0f, false, 0.0f, 0.0f, 1.0f});
    daftarSemuaPosisiKoinPotensial.push_back({-2.0f,  y_dasar_tanah, 2.0f,  false, 0.0f, 0.0f, 1.0f});
    daftarSemuaPosisiKoinPotensial.push_back({-20.0f, y_dasar_tanah, 8.0f,  false, 0.0f, 0.0f, 1.0f});
    daftarSemuaPosisiKoinPotensial.push_back({-8.0f,  y_dasar_tanah, 20.0f, false, 0.0f, 0.0f, 1.0f});
    daftarSemuaPosisiKoinPotensial.push_back({-22.0f, y_dasar_tanah, 22.0f, false, 0.0f, 0.0f, 1.0f});
    daftarSemuaPosisiKoinPotensial.push_back({-1.0f,  y_dasar_tanah, 12.0f, false, 0.0f, 0.0f, 1.0f});
    daftarSemuaPosisiKoinPotensial.push_back({-12.0f, y_dasar_tanah, 1.0f,  false, 0.0f, 0.0f, 1.0f});

    // Kuadran 3 (X negatif, Z negatif)
    daftarSemuaPosisiKoinPotensial.push_back({-10.0f, y_dasar_tanah, -10.0f, false, 0.0f, 0.0f, 1.0f});
    daftarSemuaPosisiKoinPotensial.push_back({-2.0f,  y_dasar_tanah, -2.0f,  false, 0.0f, 0.0f, 1.0f});
    daftarSemuaPosisiKoinPotensial.push_back({-20.0f, y_dasar_tanah, -8.0f,  false, 0.0f, 0.0f, 1.0f});
    daftarSemuaPosisiKoinPotensial.push_back({-8.0f,  y_dasar_tanah, -20.0f, false, 0.0f, 0.0f, 1.0f});
    daftarSemuaPosisiKoinPotensial.push_back({-22.0f, y_dasar_tanah, -22.0f, false, 0.0f, 0.0f, 1.0f});
    daftarSemuaPosisiKoinPotensial.push_back({-1.0f,  y_dasar_tanah, -12.0f, false, 0.0f, 0.0f, 1.0f});
    daftarSemuaPosisiKoinPotensial.push_back({-12.0f, y_dasar_tanah, -1.0f,  false, 0.0f, 0.0f, 1.0f});

    // Kuadran 4 (X positif, Z negatif)
    daftarSemuaPosisiKoinPotensial.push_back({10.0f, y_dasar_tanah, -10.0f, false, 0.0f, 0.0f, 1.0f});
    daftarSemuaPosisiKoinPotensial.push_back({2.0f,  y_dasar_tanah, -2.0f,  false, 0.0f, 0.0f, 1.0f});
    daftarSemuaPosisiKoinPotensial.push_back({20.0f, y_dasar_tanah, -8.0f,  false, 0.0f, 0.0f, 1.0f});
    daftarSemuaPosisiKoinPotensial.push_back({8.0f,  y_dasar_tanah, -20.0f, false, 0.0f, 0.0f, 1.0f});
    daftarSemuaPosisiKoinPotensial.push_back({22.0f, y_dasar_tanah, -22.0f, false, 0.0f, 0.0f, 1.0f});
    daftarSemuaPosisiKoinPotensial.push_back({1.0f,  y_dasar_tanah, -12.0f, false, 0.0f, 0.0f, 1.0f});
    daftarSemuaPosisiKoinPotensial.push_back({12.0f, y_dasar_tanah, -1.0f,  false, 0.0f, 0.0f, 1.0f});

    // Tambahan beberapa posisi lagi di tengah atau area lain untuk variasi
    daftarSemuaPosisiKoinPotensial.push_back({0.0f,  y_dasar_tanah, 0.0f, false, 0.0f, 0.0f, 1.0f}); 
    daftarSemuaPosisiKoinPotensial.push_back({0.0f,  y_dasar_tanah, 22.0f, false, 0.0f, 0.0f, 1.0f}); 
    daftarSemuaPosisiKoinPotensial.push_back({0.0f,  y_dasar_tanah, -22.0f, false, 0.0f, 0.0f, 1.0f});
    daftarSemuaPosisiKoinPotensial.push_back({22.0f,  y_dasar_tanah, 0.0f, false, 0.0f, 0.0f, 1.0f}); 
    daftarSemuaPosisiKoinPotensial.push_back({-22.0f,  y_dasar_tanah, 0.0f, false, 0.0f, 0.0f, 1.0f});
}

// Fungsi untuk memilih 5 koin acak untuk game saat ini
static void pilihKoinUntukGame() {
    koinAktifDiGame.clear();
    if (daftarSemuaPosisiKoinPotensial.empty()) {
        populateMasterCoinList(); 
    }

    if (daftarSemuaPosisiKoinPotensial.size() < JUMLAH_KOIN_PER_GAME) {
        printf("Error: Koin potensial kurang dari jumlah koin per game!\n");
        for(size_t i = 0; i < daftarSemuaPosisiKoinPotensial.size(); ++i) {
            KoinData koin = daftarSemuaPosisiKoinPotensial[i];
            koin.isActive = true; 
            koin.rotation = (float)(rand() % 360); 
            koin.yOffset = 0.0f;
            koin.yDirection = 1.0f;
            koinAktifDiGame.push_back(koin);
        }
        return;
    }

    vector<int> indices;
    for (size_t i = 0; i < daftarSemuaPosisiKoinPotensial.size(); ++i) {
        indices.push_back(i);
    }

    for (size_t i = indices.size() - 1; i > 0; --i) {
        int j = rand() % (i + 1);
        std::swap(indices[i], indices[j]);
    }

    for (int i = 0; i < JUMLAH_KOIN_PER_GAME; ++i) {
        KoinData koinTerpilih = daftarSemuaPosisiKoinPotensial[indices[i]];
        koinTerpilih.isActive = true; 
        koinTerpilih.rotation = (float)(rand() % 360); 
        koinTerpilih.yOffset = 0.0f; 
        koinTerpilih.yDirection = 1.0f;
        koinAktifDiGame.push_back(koinTerpilih);
    }
    printf("%d koin aktif dipilih.\n", (int)koinAktifDiGame.size());
}

// Fungsi untuk memilih koin acak
void initKoin() {        
    if (daftarSemuaPosisiKoinPotensial.empty()) { 
        populateMasterCoinList();
    }
    pilihKoinUntukGame(); 
}

// Fungsi untuk menggambar koin
void drawKoin() {
    for (size_t i = 0; i < koinAktifDiGame.size(); ++i) {
        if (koinAktifDiGame[i].isActive) {
            drawSingleKoin(koinAktifDiGame[i]);
        }
    }
}

// Fungsi untuk mendapatkan semua data koin
const vector<KoinData>& getAllKoinData() {
    return koinAktifDiGame;
}

// Fungsi untuk mengumpulkan koin
void setKoinCollected(int index) {
    if (index >= 0 && index < koinAktifDiGame.size()) {
        koinAktifDiGame[index].isActive = false;
        printf("Koin %d dikumpulkan!\n", index);
    }
}

// FUNGSI IMPLEMENTASI resetKoin()
void resetKoin() {
    printf("Resetting koin...\n");
    pilihKoinUntukGame(); 
}
#endif