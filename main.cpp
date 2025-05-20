#include <GL/glut.h>
#include <stdio.h>
#include <math.h>

#include "doraemon.h"
#include "koin.h"
#include "arena.h"

// Fungsi untuk inisialisasi
void init() {
    glClearColor(0.5f, 0.7f, 1.0f, 1.0f);  // Warna langit
    glEnable(GL_DEPTH_TEST);
    
    // Atur pencahayaan
    GLfloat ambient[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    GLfloat diffuse[] = { 0.7f, 0.7f, 0.7f, 1.0f };
    GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat position[] = { 5.0f, 10.0f, 5.0f, 0.0f };
    
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    glLightfv(GL_LIGHT0, GL_POSITION, position);
    
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    
    // Inisialisasi Doraemon dan Koin
    initDoraemon();
    initKoin();
    initArena();
}

// Fungsi untuk menampilkan scene
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    
    // Atur kamera berdasarkan data dari modul Doraemon
    gluLookAt(getCameraX(), getCameraY(), getCameraZ(),    // Posisi kamera
              getLookX(), getLookY(), getLookZ(),          // Titik yang dilihat
              0.0, 1.0, 0.0);                             // Vektor up
    
    // Gambar bidang datar
    drawArena();
    
    // Gambar Doraemon di posisi yang ditentukan
    glPushMatrix();
    glTranslatef(getDoraemonX(), getDoraemonY(), getDoraemonZ());
    drawDoraemon();
    glPopMatrix();

    // Gambar koin di posisi yang ditentukan
    glPushMatrix();
    glTranslatef(getKoinX(), getKoinY(), getKoinZ());
    drawKoin();
    glPopMatrix();
    
    // Setup untuk rendering 2D teks
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 800, 0, 600, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // Nonaktifkan lighting dan depth test untuk teks
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    
    // Tampilkan informasi kontrol
    displayControlInfo();
    
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

// Fungsi untuk menangani input keyboard (saat ditekan)
void keyboard(unsigned char key, int x, int y) {
    // Set key state in Doraemon module
    setKeyState(key, true);
    
    // Toggle mode kamera dengan tombol 0 dan 1
    if (key == '0') {
        setCameraMode(0); // Free camera mode
        printf("Camera Mode: Free Camera\n");
    } else if (key == '1') {
        setCameraMode(1); // Third person view
        printf("Camera Mode: Third Person\n");
    }
}

// Fungsi untuk menangani input keyboard (saat dilepas)
void keyboardUp(unsigned char key, int x, int y) {
    setKeyState(key, false);
}

// Fungsi idle untuk animasi
void idle() {
    // Update Doraemon
    updateDoraemon();
    
    // Update Koin
    updateKoin();
    
    // Minta redisplay
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