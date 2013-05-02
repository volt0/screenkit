#ifndef TRSS_H
#define TRSS_H

#include <stdlib.h>
#include <stdint.h>
#include <gl/glew.h>

#define RES_OK 0
#define RES_ERR_SYSTEM -1

typedef struct
{
    uint32_t width;
    uint32_t height;

    GLuint textures[3];
    uint32_t bufferWidth;
    uint32_t bufferHeight;
    float *mapBuffer;
    uint32_t *fgBuffer;
    uint32_t *bgBuffer;

    int position;
    uint32_t fgColor;
    uint32_t bgColor;
} trssContext_t;

int trssInit();

trssContext_t *trssAlloc();
int trssReshape(trssContext_t *self, int width, int height);
int trssRender(trssContext_t *self);
int trssFree(trssContext_t *self);

int trssWrite(trssContext_t *self, wchar_t *data);
int trssFlush(trssContext_t *self);

extern const int fontBitmapWidthEm;
extern const int fontBitmapHeightEm;
extern const int fontBitmapWidthPx;
extern const int fontBitmapHeightPx;
extern const unsigned char fontBitmap[];

#endif