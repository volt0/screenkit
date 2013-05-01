#include "screen.h"

GLuint trssProgram = 0;
GLuint trssFont = 0;

extern const char *trssVertSrc;
extern const char *trssFragSrc;


typedef struct
{
    GLuint textures[3];
    float *mapBuffer;
    uint32_t *fgBuffer;
    uint32_t *bgBuffer;

    int position;
    uint32_t fgColor;
    uint32_t bgColor;
} trssContext_t;

trssContext_t __ctx;
extern int __h;
extern int __w;

int trssInit()
{
    GLint stat;
    GLuint vertShader;
    GLuint fragShader;

    if (!trssProgram)
    {
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
    }

    // TMP {{
    {
        const int h = 256;
        const int w = 256;
        const int s = h*w;
        int i;

        __ctx.position = 0;
        __ctx.fgColor = 0xffcccccc;
        __ctx.bgColor = 0xff000000;

        glGenTextures(3, __ctx.textures);
        __ctx.mapBuffer = (float *)malloc(s*sizeof(float));
        __ctx.fgBuffer = (uint32_t *)malloc(s*sizeof(uint32_t));
        __ctx.bgBuffer = (uint32_t *)malloc(s*sizeof(uint32_t));

        for (i = 0; i < s; i++)
        {
            __ctx.mapBuffer[i] = 1.0f;
            __ctx.fgBuffer[i] = __ctx.fgColor;
            __ctx.bgBuffer[i] = __ctx.bgColor;
        }

        glBindTexture(GL_TEXTURE_RECTANGLE, __ctx.textures[0]);
        glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_R32F, w, h, 0, GL_RED, GL_FLOAT, (void*)__ctx.mapBuffer);

        glBindTexture(GL_TEXTURE_RECTANGLE, __ctx.textures[1]);
        glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void*)__ctx.fgBuffer);

        glBindTexture(GL_TEXTURE_RECTANGLE, __ctx.textures[2]);
        glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void*)__ctx.bgBuffer);
    }
    {
        const int h = 256;
        const int w = 256;
        wchar_t *i;
        wchar_t *hello = L"Hello world P2";

        for (i = hello; *i; ++i)
        {
            if ((*i >= 0x20) && (*i <= 0x7f))
            {
                __ctx.mapBuffer[__ctx.position] = *i - 0x1f;
            }
            else
            {
                __ctx.mapBuffer[__ctx.position] = 0.0f;
            }

            __ctx.position++;
            
            glBindTexture(GL_TEXTURE_RECTANGLE, __ctx.textures[0]);
            glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_R32F, w, h, 0, GL_RED, GL_FLOAT, (void*)__ctx.mapBuffer);
        }
    }
    // TMP }}

    return ERR_OK;
}

void trssRender()
{
    int i;

    glUseProgram(trssProgram);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_RECTANGLE, trssFont);
    
    for (i = 0; i < 3; i++)
    {
        glActiveTexture(GL_TEXTURE1 + i);
        glBindTexture(GL_TEXTURE_RECTANGLE, __ctx.textures[i]);
    }

    glPushMatrix();
    glBegin(GL_QUADS);
    {
        glTexCoord2i(0, 0);
        glVertex2i  (0, 0);
        glTexCoord2i(__w, 0);
        glVertex2i  (__w, 0);
        glTexCoord2i(__w, __h);
        glVertex2i  (__w, __h);
        glTexCoord2i(0, __h);
        glVertex2i  (0, __h);
    }
    glEnd();
    glPopMatrix();  
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
