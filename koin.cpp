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

// Function to update coin (called from idle)
void updateKoin() {
    for (size_t i = 0; i < koinAktifDiGame.size(); ++i) {
        if (koinAktifDiGame[i].isActive) {
            // Rotasi koin
            koinAktifDiGame[i].rotation += 10.0f; // Kecepatan rotasi bisa disesuaikan
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
    // Pindah ke posisi koin
    glTranslatef(koin.x, koin.y + koin.yOffset, koin.z);
    
    // Terapkan rotasi pada koin
    glRotatef(koin.rotation, 0.0f, 1.0f, 0.0f);
    
    glColor3f(1.0f, 0.84f, 0.0f);  // Warna emas
    
    GLUquadricObj *cylinder = gluNewQuadric();
    gluQuadricDrawStyle(cylinder, GLU_FILL);
    
    glRotatef(270.0f, 0.0f, 1.0f, 0.0f); 
    
    float coinRadius = 0.5f; // Ukuran bisa disesuaikan
    float coinThickness = 0.1f;
    
    gluCylinder(cylinder, coinRadius, coinRadius, coinThickness, 30, 2); // Sisi
    gluDisk(cylinder, 0.0f, coinRadius, 30, 1); // Tutup 1 (di z=0)
    
    glPushMatrix();
    glColor3f(0.8f, 0.7f, 0.0f); // Warna emas lebih gelap
    glTranslatef(0.0f, 0.0f, -coinThickness/2);
    gluDisk(cylinder, coinRadius-0.1f, coinRadius, 30, 1); // Tutup 2 (di z=coinThickness)
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.8f, 0.7f, 0.0f); // Warna emas lebih gelap
    glTranslatef(0.0f, 0.0f, coinThickness/2);
    gluDisk(cylinder, coinRadius-0.1f, coinRadius, 30, 1); // Tutup 2 (di z=coinThickness)
    glPopMatrix();
    
    gluDeleteQuadric(cylinder);
    glPopMatrix();
}

// Fungsi untuk mengisi daftar semua posisi koin potensial
// Panggil ini sekali saja, mungkin dari init() di main.cpp atau di initKoin() pertama kali.
static void populateMasterCoinList() {
    daftarSemuaPosisiKoinPotensial.clear();
    
    // Dapatkan data gedung dari arena.cpp
    const vector<Building>& semuaGedung = getArenaBuildings();

    float y_dasar_tanah = 0.5f; // Ketinggian koin dari tanah
    float y_offset_diatas_gedung = 0.5f; // Ketinggian koin dari permukaan atas gedung

    // 1. Tambahkan koin di atas setiap gedung
    for (const auto& gedung : semuaGedung) {
        daftarSemuaPosisiKoinPotensial.push_back({
            gedung.x, 
            gedung.height + y_offset_diatas_gedung, 
            gedung.z, 
            false, 0.0f, 0.0f, 1.0f
        });
    }

    // 2. Tambahkan koin di jalan (tanah), tersebar di 4 kuadran
    // Pastikan posisi ini tidak tumpang tindih dengan footprint gedung.
    // Footprint gedung umumnya: x +/- width/2, z +/- depth/2. Gedung Anda memiliki width/depth 5.0.


    // Kuadran 1 (X positif, Z positif)
    // Gedung ada di (5,5), (5,15), (15,5), (15,15)
    daftarSemuaPosisiKoinPotensial.push_back({10.0f, y_dasar_tanah, 10.0f, false, 0.0f, 0.0f, 1.0f}); // Tengah antar gedung Q1
    daftarSemuaPosisiKoinPotensial.push_back({2.0f,  y_dasar_tanah, 2.0f,  false, 0.0f, 0.0f, 1.0f});  // Pojok dekat origin
    daftarSemuaPosisiKoinPotensial.push_back({20.0f, y_dasar_tanah, 8.0f,  false, 0.0f, 0.0f, 1.0f});
    daftarSemuaPosisiKoinPotensial.push_back({8.0f,  y_dasar_tanah, 20.0f, false, 0.0f, 0.0f, 1.0f});
    daftarSemuaPosisiKoinPotensial.push_back({22.0f, y_dasar_tanah, 22.0f, false, 0.0f, 0.0f, 1.0f}); // Pojok luar
    daftarSemuaPosisiKoinPotensial.push_back({1.0f,  y_dasar_tanah, 12.0f, false, 0.0f, 0.0f, 1.0f});
    daftarSemuaPosisiKoinPotensial.push_back({12.0f, y_dasar_tanah, 1.0f,  false, 0.0f, 0.0f, 1.0f});


    // Kuadran 2 (X negatif, Z positif)
    // Gedung ada di (-5,5), (-5,15), (-15,5), (-15,15)
    daftarSemuaPosisiKoinPotensial.push_back({-10.0f, y_dasar_tanah, 10.0f, false, 0.0f, 0.0f, 1.0f});
    daftarSemuaPosisiKoinPotensial.push_back({-2.0f,  y_dasar_tanah, 2.0f,  false, 0.0f, 0.0f, 1.0f});
    daftarSemuaPosisiKoinPotensial.push_back({-20.0f, y_dasar_tanah, 8.0f,  false, 0.0f, 0.0f, 1.0f});
    daftarSemuaPosisiKoinPotensial.push_back({-8.0f,  y_dasar_tanah, 20.0f, false, 0.0f, 0.0f, 1.0f});
    daftarSemuaPosisiKoinPotensial.push_back({-22.0f, y_dasar_tanah, 22.0f, false, 0.0f, 0.0f, 1.0f});
    daftarSemuaPosisiKoinPotensial.push_back({-1.0f,  y_dasar_tanah, 12.0f, false, 0.0f, 0.0f, 1.0f});
    daftarSemuaPosisiKoinPotensial.push_back({-12.0f, y_dasar_tanah, 1.0f,  false, 0.0f, 0.0f, 1.0f});

    // Kuadran 3 (X negatif, Z negatif)
    // Gedung ada di (-5,-5), (-5,-15), (-15,-5), (-15,-15)
    daftarSemuaPosisiKoinPotensial.push_back({-10.0f, y_dasar_tanah, -10.0f, false, 0.0f, 0.0f, 1.0f});
    daftarSemuaPosisiKoinPotensial.push_back({-2.0f,  y_dasar_tanah, -2.0f,  false, 0.0f, 0.0f, 1.0f});
    daftarSemuaPosisiKoinPotensial.push_back({-20.0f, y_dasar_tanah, -8.0f,  false, 0.0f, 0.0f, 1.0f});
    daftarSemuaPosisiKoinPotensial.push_back({-8.0f,  y_dasar_tanah, -20.0f, false, 0.0f, 0.0f, 1.0f});
    daftarSemuaPosisiKoinPotensial.push_back({-22.0f, y_dasar_tanah, -22.0f, false, 0.0f, 0.0f, 1.0f});
    daftarSemuaPosisiKoinPotensial.push_back({-1.0f,  y_dasar_tanah, -12.0f, false, 0.0f, 0.0f, 1.0f});
    daftarSemuaPosisiKoinPotensial.push_back({-12.0f, y_dasar_tanah, -1.0f,  false, 0.0f, 0.0f, 1.0f});

    // Kuadran 4 (X positif, Z negatif)
    // Gedung ada di (5,-5), (5,-15), (15,-5), (15,-15)
    daftarSemuaPosisiKoinPotensial.push_back({10.0f, y_dasar_tanah, -10.0f, false, 0.0f, 0.0f, 1.0f});
    daftarSemuaPosisiKoinPotensial.push_back({2.0f,  y_dasar_tanah, -2.0f,  false, 0.0f, 0.0f, 1.0f});
    daftarSemuaPosisiKoinPotensial.push_back({20.0f, y_dasar_tanah, -8.0f,  false, 0.0f, 0.0f, 1.0f});
    daftarSemuaPosisiKoinPotensial.push_back({8.0f,  y_dasar_tanah, -20.0f, false, 0.0f, 0.0f, 1.0f});
    daftarSemuaPosisiKoinPotensial.push_back({22.0f, y_dasar_tanah, -22.0f, false, 0.0f, 0.0f, 1.0f});
    daftarSemuaPosisiKoinPotensial.push_back({1.0f,  y_dasar_tanah, -12.0f, false, 0.0f, 0.0f, 1.0f});
    daftarSemuaPosisiKoinPotensial.push_back({12.0f, y_dasar_tanah, -1.0f,  false, 0.0f, 0.0f, 1.0f});

    // Tambahan beberapa posisi lagi di tengah atau area lain untuk variasi
    daftarSemuaPosisiKoinPotensial.push_back({0.0f,  y_dasar_tanah, 0.0f, false, 0.0f, 0.0f, 1.0f}); // Tepat di tengah arena
    daftarSemuaPosisiKoinPotensial.push_back({0.0f,  y_dasar_tanah, 22.0f, false, 0.0f, 0.0f, 1.0f}); // Tengah sisi utara
    daftarSemuaPosisiKoinPotensial.push_back({0.0f,  y_dasar_tanah, -22.0f, false, 0.0f, 0.0f, 1.0f});// Tengah sisi selatan
    daftarSemuaPosisiKoinPotensial.push_back({22.0f,  y_dasar_tanah, 0.0f, false, 0.0f, 0.0f, 1.0f}); // Tengah sisi timur
    daftarSemuaPosisiKoinPotensial.push_back({-22.0f,  y_dasar_tanah, 0.0f, false, 0.0f, 0.0f, 1.0f});// Tengah sisi barat
}

// Fungsi untuk memilih 5 koin acak untuk game saat ini
static void pilihKoinUntukGame() {
    koinAktifDiGame.clear();
    if (daftarSemuaPosisiKoinPotensial.empty()) {
        populateMasterCoinList(); // Pastikan ada isinya
    }

    if (daftarSemuaPosisiKoinPotensial.size() < JUMLAH_KOIN_PER_GAME) {
        // Handle error: tidak cukup koin potensial
        printf("Error: Koin potensial kurang dari jumlah koin per game!\n");
        // Untuk sementara, pakai semua yang ada
        for(size_t i = 0; i < daftarSemuaPosisiKoinPotensial.size(); ++i) {
            KoinData koin = daftarSemuaPosisiKoinPotensial[i];
            koin.isActive = true; // Aktifkan
            koin.rotation = (float)(rand() % 360); // Rotasi awal acak
            koin.yOffset = 0.0f;
            koin.yDirection = 1.0f;
            koinAktifDiGame.push_back(koin);
        }
        return;
    }

    // Logika untuk memilih secara acak tanpa duplikat
    vector<int> indices;
    for (size_t i = 0; i < daftarSemuaPosisiKoinPotensial.size(); ++i) {
        indices.push_back(i);
    }

    // Acak indices (Fisher-Yates shuffle sederhana)
    for (size_t i = indices.size() - 1; i > 0; --i) {
        int j = rand() % (i + 1);
        std::swap(indices[i], indices[j]);
    }

    // Ambil JUMLAH_KOIN_PER_GAME pertama dari indices yang sudah diacak
    for (int i = 0; i < JUMLAH_KOIN_PER_GAME; ++i) {
        KoinData koinTerpilih = daftarSemuaPosisiKoinPotensial[indices[i]];
        koinTerpilih.isActive = true; // Koin ini aktif untuk game
        koinTerpilih.rotation = (float)(rand() % 360); // Rotasi awal acak
        koinTerpilih.yOffset = 0.0f; // Reset animasi
        koinTerpilih.yDirection = 1.0f;
        koinAktifDiGame.push_back(koinTerpilih);
    }
    printf("%d koin aktif dipilih.\n", (int)koinAktifDiGame.size());
}

// Function to initialize coin
void initKoin() {        
    if (daftarSemuaPosisiKoinPotensial.empty()) { // Hanya isi sekali
        populateMasterCoinList();
    }
    pilihKoinUntukGame(); // Pilih 5 koin untuk sesi game ini
}

// Fungsi untuk menggambar koin
void drawKoin() {
    for (size_t i = 0; i < koinAktifDiGame.size(); ++i) {
        if (koinAktifDiGame[i].isActive) {
            drawSingleKoin(koinAktifDiGame[i]);
        }
    }
}

const vector<KoinData>& getAllKoinData() {
    return koinAktifDiGame;
}

void setKoinCollected(int index) {
    if (index >= 0 && index < koinAktifDiGame.size()) {
        koinAktifDiGame[index].isActive = false;
        // Tambahkan logika skor di sini atau di game logic utama
        printf("Koin %d dikumpulkan!\n", index);
    }
}

// FUNGSI IMPLEMENTASI resetKoin()
void resetKoin() {
    printf("Resetting koin...\n");
    pilihKoinUntukGame(); // Panggil kembali fungsi untuk memilih 5 koin baru
}
#endif