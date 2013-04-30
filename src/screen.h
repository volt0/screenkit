#ifndef SCREEN_H
#define SCREEN_H

#ifdef _WIN32
#include <Windows.h>
// #include <gl/GL.h>
#endif

#include <gl/glew.h>

#define ERR_OK 0
#define ERR_SYSTEM -1

int platformOpenScreen();

int coreInit();
void coreReshape(int width, int height);
void coreRender();

extern const int fontBitmapWidthEm;
extern const int fontBitmapHeightEm;
extern const int fontBitmapWidthPx;
extern const int fontBitmapHeightPx;
extern const unsigned char fontBitmap[];

#endif