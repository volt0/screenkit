#ifndef SCREEN_H
#define SCREEN_H

#ifdef _WIN32
#include <Windows.h>
// #include <gl/GL.h>
#endif

#include <gl/glew.h>
#include <stdint.h>

#define ERR_OK 0
#define ERR_SYSTEM -1

int platformOpenScreen();

int coreInit();
void coreReshape(int width, int height);
void coreRender();

#endif