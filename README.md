# Proyek GKV C1 Kelompok 2

Proyek ini merupakan sebuah aplikasi grafika komputer 3D yang menampilkan karakter Doraemon yang dapat terbang di sebuah arena perkotaan. Pemain mengontrol Doraemon untuk mengumpulkan koin-koin yang tersebar di arena. Aplikasi ini dibangun menggunakan C++ dengan library GLUT untuk rendering grafis.

Proyek ini dikembangkan dan dikonfigurasi untuk Visual Studio Code, namun dapat dikompilasi menggunakan g++ pada environment yang mendukung OpenGL dan GLUT.

## Daftar Isi
1.  [Pengaturan Lingkungan VSCode](#1-pengaturan-lingkungan-vscode-setup-environment)
2.  [Struktur File Proyek](#2-struktur-file-proyek)
3.  [Kompilasi Proyek](#3-kompilasi-proyek-compilation)
4.  [Menjalankan Aplikasi](#4-menjalankan-aplikasi-running-the-executable)
5.  [Opsi Tanpa Kompilasi](#5-opsi-tanpa-kompilasi-pre-compiled-option)
6.  [Kontrol Permainan](#6-kontrol-permainan)
7.  [Tujuan Permainan](#7-tujuan-permainan)
8.  [Fitur Utama](#8-fitur-utama)
9.  [Referensi Repositori GitHub](#9-referensi-repositori-github-github-repository)

## 1. Pengaturan Lingkungan VSCode (Setup Environment)
Proyek ini dibuat dan dikonfigurasi untuk dijalankan pada Visual Studio Code. Untuk detail pengaturan environment GLUT di VSCode, Anda dapat merujuk pada panduan berikut:
* **[Tutorial Setup GLUT di VSCode](https://sugar-script-430.notion.site/Tutorial-Setup-GLUT-di-VSCode-1b1bded9d4fb80249cb1d771dc524f8b?pvs=4)**

File konfigurasi VSCode (`.vscode/c_cpp_properties.json`, `.vscode/launch.json`, `.vscode/settings.json`, `.vscode/tasks.json`) sudah disertakan dalam proyek ini.

## 2. Struktur File Proyek
Pastikan file-file berikut berada dalam satu direktori utama untuk kompilasi dan menjalankan proyek:

* **Source Code (.cpp dan .h):**
    * `main.cpp` (File utama, logika game, rendering loop)
    * `doraemon.cpp` & `doraemon.h` (Logika dan model Doraemon, kontrol kamera)
    * `koin.cpp` & `koin.h` (Logika dan model Koin)
    * `arena.cpp` & `arena.h` (Logika dan model Arena, gedung, tembok)
    * `imageloader.cpp` & `imageloader.h` (Fungsi untuk memuat gambar BMP sebagai tekstur)
* **File Tekstur (.bmp):**
    * `tembok.bmp`
    * `paving.bmp`
    * `gedung1.bmp`
    * `gedung2.bmp`
    * `gedung3.bmp`
    * `gedung4.bmp`
* **Folder Konfigurasi VSCode (opsional jika tidak menggunakan VSCode):**
    * `.vscode/` (berisi `c_cpp_properties.json`, `launch.json`, `settings.json`, `tasks.json`)

## 3. Kompilasi Proyek (Compilation)
Jika Anda ingin melakukan kompilasi source code secara manual, ikuti langkah-langkah berikut:

1.  **Pastikan semua file** yang disebutkan di atas (terutama source code dan file header) berada dalam satu folder/direktori.
2.  **Buka terminal** atau command prompt (misalnya, MinGW 64-bit jika menggunakan g++ dari MSYS2).
3.  **Arahkan direktori terminal** ke lokasi folder proyek Anda menggunakan perintah `cd`. Contoh:
    ```bash
    cd path/to/your/DoraemonProject-main
    ```
4.  Setelah berada di direktori yang benar, **kompilasi proyek** dengan menjalankan perintah berikut di terminal:
    ```bash
    g++ main.cpp doraemon.cpp koin.cpp arena.cpp imageloader.cpp -o main.exe -std=c++17 -lfreeglut -lglu32 -lopengl32 -lglfw3 -lglew32
    ```
    *Catatan: Perintah di atas menggunakan `g++` dan menyertakan library yang umum digunakan. Pastikan environment Anda memiliki library-library tersebut (freeglut, glu32, opengl32, dan jika diperlukan glfw3, glew32).*

## 4. Menjalankan Aplikasi (Running the Executable)
Setelah proses kompilasi berhasil dan file `main.exe` telah dibuat:

1.  Anda dapat menjalankan aplikasi langsung dari terminal yang masih terbuka di direktori proyek:
    ```bash
    ./main.exe
    ```
2.  Atau, Anda juga bisa menjalankan file `main.exe` dengan melakukan *double-click* pada file tersebut melalui file explorer.

## 5. Opsi Tanpa Kompilasi (Pre-compiled Option)
Jika Anda tidak ingin melakukan proses kompilasi sendiri, Anda dapat:
* Menggunakan file `main.exe` yang mungkin sudah disertakan dalam folder rilis proyek ini.
* Mengunduh versi `main.exe` yang sudah dikompilasi dari bagian "Releases" pada repositori GitHub proyek (jika tersedia).

## 6. Kontrol Permainan
Berikut adalah kontrol untuk memainkan game:

* **ENTER**: Memulai permainan dari layar awal.
* **R**: Memulai ulang permainan (ketika permainan telah berakhir/Game Over).
* **B**: Mengaktifkan/Menonaktifkan bayangan (Shadows).

* **Mode Kamera:**
    * **0**: Mode Kamera Bebas
        * `W` / `S`: Maju / Mundur (kamera)
        * `A` / `D`: Rotasi Kamera Kiri / Kanan
        * `Q` / `E`: Naik / Turun (kamera)
    * **1**: Mode Third Person (mengikuti Doraemon)
        * `W` / `S`: Doraemon Maju / Mundur
        * `A` / `D`: Doraemon Putar Kiri / Kanan
        * `Q` / `E`: Doraemon Terbang Naik / Turun

## 7. Tujuan Permainan
Objektif dari permainan ini adalah untuk mengumpulkan **10 koin** yang tersebar di seluruh arena. Setelah semua koin terkumpul, permainan akan berakhir dan menampilkan waktu yang Anda butuhkan.

## 8. Fitur Utama
* Model 3D Doraemon lengkap dengan baling-baling bambu.
* Kemampuan terbang bebas di dalam arena perkotaan yang berisi gedung-gedung.
* Sistem pengumpulan koin sebagai objektif permainan.
* Dua mode kamera: Third-person yang mengikuti Doraemon dan mode kamera bebas.
* Implementasi bayangan dinamis sederhana (Projected Shadows) yang dapat diaktifkan atau dinonaktifkan.
* Informasi skor dan timer yang ditampilkan selama permainan.
* User Interface (UI) untuk pesan awal permainan, panduan kontrol, dan pesan akhir permainan.
* Tekstur pada arena (tembok, jalan, gedung) untuk visual yang lebih menarik.
* Deteksi kolisi sederhana antara Doraemon dengan gedung dan batas arena.

## 9. Referensi Repositori GitHub (GitHub Repository)
Source code lengkap dan update terbaru dari proyek ini dapat ditemukan pada repositori GitHub berikut:
* **[Julius-Tegar-Aji-Putra/DoraemonProject](https://github.com/Julius-Tegar-Aji-Putra/DoraemonProject/tree/main)**