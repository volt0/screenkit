#include "trss.h"

#include <stdio.h>

GLuint trssProgram = 0;
GLuint trssFont = 0;

extern const char *trssVertSrc;
extern const char *trssFragSrc;

int trssInit()
{
    GLint stat;
    GLuint vertShader;
    GLuint fragShader;

    static const float bmpIndex[] = {0.0f, 1.0f};

    vertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertShader, 1, &trssVertSrc, NULL);
    glCompileShader(vertShader);

    fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &trssFragSrc, NULL);
    glCompileShader(fragShader);
    
    // DEBUG {{
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &stat);
    if (stat == GL_FALSE)
    {
        char *log;
        glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &stat);

        log = malloc(stat + 1);
        log[stat] = 0;
        glGetShaderInfoLog(fragShader, stat, &stat, log);
        printf("Error compilling GL_FRAGMENT_SHADER:\n%s\n", log);

        return RES_ERR_SYSTEM;
    }
    // DEBUG }}

    trssProgram = glCreateProgram();
    glAttachShader(trssProgram, vertShader);
    glAttachShader(trssProgram, fragShader);
    glLinkProgram(trssProgram);

    glUseProgram(trssProgram);
    glUniform1i(glGetUniformLocation(trssProgram, "font"), 0);
    glUniform1i(glGetUniformLocation(trssProgram, "map"),  1);
    glUniform1i(glGetUniformLocation(trssProgram, "fg"),   2);
    glUniform1i(glGetUniformLocation(trssProgram, "bg"),   3);

    glGenTextures(1, &trssFont);
    glBindTexture(GL_TEXTURE_RECTANGLE, trssFont);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelMapfv(GL_PIXEL_MAP_I_TO_A, 2, bmpIndex);
    
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glTexImage2D(
        GL_TEXTURE_RECTANGLE,
        0,
        GL_ALPHA,
        fontBitmapWidthPx,
        fontBitmapHeightPx,
        0,
        GL_COLOR_INDEX,
        GL_BITMAP,
        (void *)fontBitmap
        );

    return RES_OK;
}

trssContext_t *trssAlloc()
{
    int result;
    int i;

    trssContext_t *self;
    
    self = (trssContext_t *)malloc(sizeof(trssContext_t));
    self->position = 0;
    self->fgColor = 0xffcccccc;
    self->bgColor = 0xff000000;

    self->width = 0;
    self->height = 0;

    glGenTextures(3, self->textures);
    for (i = 0; i < 3; i++)
    {
        glBindTexture(GL_TEXTURE_RECTANGLE, self->textures[i]);
        glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    self->bufferWidth = 0;
    self->bufferHeight = 0;
    self->mapBuffer = NULL;
    self->fgBuffer = NULL;
    self->bgBuffer = NULL;

    return self;
}

uint32_t utilsUpperPowOf2(uint32_t x)
{
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x++;
    return x;    
}

int trssReshape(trssContext_t *self, int width, int height)
{
    uint32_t widthRq;
    uint32_t heightRq;

    width = (width % 8 > 4) ? (width / 8) + 1 : width / 8;
    height = (height % 16 > 8) ? (height / 16) + 1 : height / 16;

    widthRq = utilsUpperPowOf2(width);
    heightRq = utilsUpperPowOf2(height);

    if ((widthRq > self->bufferWidth) || (heightRq > self->bufferHeight))
    {
        uint32_t size = widthRq * heightRq;
        self->mapBuffer = (float *)realloc(self->mapBuffer, size * sizeof(float));
        self->fgBuffer = (uint32_t *)realloc(self->fgBuffer, size * sizeof(uint32_t));
        self->bgBuffer = (uint32_t *)realloc(self->bgBuffer, size * sizeof(uint32_t));

        if (!(self->width && self->height))
        {
            int i;
            for (i = 0; i < size; i++)
            {
                self->mapBuffer[i] = ((i % widthRq) == 0) ? 2.0f : 0.0f;
                self->fgBuffer[i] = self->fgColor;
                self->bgBuffer[i] = self->bgColor;
            }            
        }
        else
        {
            int i, j;
            for (i = self->bufferHeight - 1; i >= 0; i--)
            {
                int srcOffs;
                int dstOffs;
                srcOffs = i * self->bufferWidth;
                dstOffs = i * widthRq;

                memcpy(self->mapBuffer + dstOffs, self->mapBuffer + srcOffs, self->bufferWidth * sizeof(float));
                memcpy(self->fgBuffer + dstOffs, self->fgBuffer + srcOffs, self->bufferWidth * sizeof(uint32_t));
                memcpy(self->bgBuffer + dstOffs, self->bgBuffer + srcOffs, self->bufferWidth * sizeof(uint32_t));

                for (j = self->bufferWidth; j < widthRq; j++)
                {
                    self->mapBuffer[j + dstOffs] = 3.0f;
                    self->fgBuffer[j + dstOffs] = self->fgColor;
                    self->bgBuffer[j + dstOffs] = self->bgColor;                    
                }
            }

            for (i = self->bufferHeight * widthRq; i < size; i++)
            {
                self->mapBuffer[i] = 4.0f;
                self->fgBuffer[i] = self->fgColor;
                self->bgBuffer[i] = self->bgColor;
            }

            // int srcOffs = self->bufferWidth << 1;
            // int srcPos = size - srcOffs;
            // for (i = self->bufferHeight - 2; i >= 0; i -= 2)
            // {

            // }
        }

        self->bufferWidth = widthRq;
        self->bufferHeight = heightRq;
        trssFlush(self);
    }

    self->width = width;
    self->height = height;

    return RES_OK;
}

// void tmp()
// {
//     const int h = 256;
//     const int w = 256;
//     wchar_t *i;
//     wchar_t *hello = L"Hello world P2";

//     for (i = hello; *i; ++i)
//     {
//         if ((*i >= 0x20) && (*i <= 0x7f))
//         {
//             __ctx.mapBuffer[__ctx.position] = *i - 0x1f;
//         }
//         else
//         {
//             __ctx.mapBuffer[__ctx.position] = 0.0f;
//         }

//         __ctx.position++;
        
//         glBindTexture(GL_TEXTURE_RECTANGLE, __ctx.textures[0]);
//         glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_R32F, w, h, 0, GL_RED, GL_FLOAT, (void*)__ctx.mapBuffer);

//     return S_OK;
// }

const float trssHStep = 8.0f;
const float trssVStep = 16.0f;

int trssRender(trssContext_t *self)
{
    int i;
    float widthTx;
    float heightTx;

    widthTx = (float)self->width * trssHStep;
    heightTx = (float)self->height * trssVStep;

    glUseProgram(trssProgram);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_RECTANGLE, trssFont);
    
    for (i = 0; i < 3; i++)
    {
        glActiveTexture(GL_TEXTURE1 + i);
        glBindTexture(GL_TEXTURE_RECTANGLE, self->textures[i]);
    }

    glPushMatrix();
    glScalef(trssHStep, trssVStep, 1.0f);
    glBegin(GL_QUADS);
    {
        glTexCoord2f(0.0f,    0.0f);     glVertex2i(0, 0);
        glTexCoord2f(widthTx, 0.0f);     glVertex2i(self->width, 0);
        glTexCoord2f(widthTx, heightTx); glVertex2i(self->width, self->height);
        glTexCoord2f(0.0f,    heightTx); glVertex2i(0, self->height);
    }
    glEnd();
    glPopMatrix();  

    glUseProgram(0);

    return RES_OK;
}

int trssFree(trssContext_t *self)
{
    return RES_OK;
}

int trssFlush(trssContext_t *self)
{
    glBindTexture(GL_TEXTURE_RECTANGLE, self->textures[0]);
    glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_R32F, self->bufferWidth, self->bufferHeight, 0, GL_RED, GL_FLOAT, (void*)self->mapBuffer);

    glBindTexture(GL_TEXTURE_RECTANGLE, self->textures[1]);
    glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, self->bufferWidth, self->bufferHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void*)self->fgBuffer);

    glBindTexture(GL_TEXTURE_RECTANGLE, self->textures[2]);
    glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, self->bufferWidth, self->bufferHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void*)self->bgBuffer);

    return RES_OK;
}

int trssWrite(trssContext_t *self, wchar_t *data)
{
    return RES_OK;
}

const char *trssVertSrc =
"void main(){"
    "gl_TexCoord[0] = gl_MultiTexCoord0;"
    "gl_Position=gl_ModelViewProjectionMatrix*gl_Vertex;"
"}";

const char *trssFragSrc =
"uniform sampler2DRect font;"
"uniform sampler2DRect map;"
"uniform sampler2DRect fg;"
"uniform sampler2DRect bg;"

"void main(){"
    "ivec2 csz=ivec2(8,16);"
    "ivec2 pos=ivec2(gl_TexCoord[0].st/csz);"
    "ivec2 off=ivec2(mod(gl_TexCoord[0].st,csz));"
    "float ch=texture2DRect(map,pos).r;"

    "if(texture2DRect(font,(vec2(mod(ch,128),floor(ch/128))*csz)+off).a==0){"
        "gl_FragColor=texture2DRect(bg,pos);"
    "}else{"
        "float a=max(texture2DRect(fg,pos).a,texture2DRect(bg,pos).a);"
        "gl_FragColor=vec4(texture2DRect(fg,pos).rgb,a);"
        // "float a=texture2DRect(fg,pos).a;"
        // "gl_FragColor=vec4("
        //     "(texture2DRect(fg,pos).rgb*a)+(texture2DRect(bg,pos).rgb*(1-a)),"
        //     "texture2DRect(bg,pos).a"
        //     ");"
    "}"
"}";
