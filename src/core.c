#include "screen.h"

#include <stdint.h>

int __h;
int __w;

static const char *trssVertSrc =
"void main(){"
"gl_TexCoord[0] = gl_MultiTexCoord0;"
"gl_Position=gl_ModelViewProjectionMatrix*gl_Vertex;"
"}";

static const char *trssFragSrc =
"uniform sampler2DRect font;"
"uniform sampler2DRect fg;"
"uniform sampler2DRect bg;"
"uniform sampler2DRect map;"
"void main(){"
"ivec2 csz=ivec2(8,16);"
"ivec2 pos=ivec2(gl_TexCoord[0].st/csz);"
"ivec2 off=ivec2(mod(gl_TexCoord[0].st,csz));"

"if(texture2DRect(font,texture2DRect(map,vec2(0,0)).rg+off).a==0){"
"gl_FragColor=texture2DRect(bg,pos);"
"}else{"
"float a=texture2DRect(fg,pos).a;"
"gl_FragColor=vec4("
	"(texture2DRect(fg,pos).rgb*a)+(texture2DRect(bg,pos).rgb*(1-a)),"
	"texture2DRect(bg,pos).a"
	");"
"}"
"}";

GLuint trssProgram = 0;

GLuint __fontTexture;
GLuint __fgTexture;
GLuint __bgTexture;
GLuint __mapTexture;

int trssInit()
{
	static const float index[] = {0.0f, 1.0f};

	GLint stat;
	GLuint vertShader;
	GLuint fragShader;

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
	glUniform1i(glGetUniformLocation(trssProgram, "fg"),   1);
	glUniform1i(glGetUniformLocation(trssProgram, "bg"),   2);
	glUniform1i(glGetUniformLocation(trssProgram, "map"),  3);

	glGenTextures(1, &__fontTexture);
	glBindTexture(GL_TEXTURE_RECTANGLE, __fontTexture);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelMapfv(GL_PIXEL_MAP_I_TO_A, 2, index);
	
	// glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	// glTexParameterf(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_REPEAT);
	// glTexParameterf(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_REPEAT);
	
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

	// TMP {{
	{
		const int h = 30;
		const int w = 80;
		int i;
		int s;
		uint8_t *fg;
		uint8_t *bg;
		float *map;
		s = h*w;
		fg = malloc(s*4);
		bg = malloc(s*4);
		map = malloc(s*2*sizeof(float));
		
		for (i = 0; i < s; i++)
		{
			int y = i/w;

			fg[(i*4)]     = (y&1 ) ? ((y&2 ) ? 0xff : 0x7f) : 0x00;
			fg[(i*4) + 1] = (y&4 ) ? ((y&8 ) ? 0xff : 0x7f) : 0x00;
			fg[(i*4) + 2] = (y&16) ? ((y&32) ? 0xff : 0x7f) : 0x00;
			fg[(i*4) + 3] = (i%w)*3;

			bg[(i*4)    ] = 0x00;
			bg[(i*4) + 1] = 0x40;
			bg[(i*4) + 2] = 0x80;
			bg[(i*4) + 3] = 0x7f;

			map[(i*2)    ] = 0.0f;
			map[(i*2) + 1] = 0.0f;
		}

		glGenTextures(1, &__fgTexture);
		glBindTexture(GL_TEXTURE_RECTANGLE, __fgTexture);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexImage2D(GL_TEXTURE_RECTANGLE,0,GL_RGBA,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,(void*)fg);

		glGenTextures(1, &__bgTexture);
		glBindTexture(GL_TEXTURE_RECTANGLE, __bgTexture);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexImage2D(GL_TEXTURE_RECTANGLE,0,GL_RGBA,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,(void*)bg);

		glGenTextures(1, &__mapTexture);
		glBindTexture(GL_TEXTURE_RECTANGLE, __mapTexture);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexImage2D(GL_TEXTURE_RECTANGLE,0,GL_RG16F,w,h,0,GL_RG,GL_FLOAT,(void*)map);

		free(fg);
		free(bg);
		free(map);
	}
	// TMP }}

	return ERR_OK;
}

void trssRender()
{
	glUseProgram(trssProgram);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_RECTANGLE, __fontTexture);
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_RECTANGLE, __fgTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_RECTANGLE, __bgTexture);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_RECTANGLE, __mapTexture);

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

int coreInit()
{
	glewInit();
	if (!GLEW_VERSION_2_0)
	{
		printf("OpenGL 2.0+ is required!\n");
		return 1;
	}

	glClearColor(0, 0, 0, 0);
	glEnable(GL_TEXTURE_RECTANGLE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	trssInit();

	return ERR_OK;
}

void coreReshape(int width, int height)
{
	__h = height;
	__w = width;

	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, width, height, 0);
	glMatrixMode(GL_MODELVIEW);
}

void coreRender()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();

	glColor3f(0.75f, 0.75f, 0.75f);
	glBegin(GL_LINES);
	{
		glVertex2i  (0, 0);
		glVertex2i  (__w, __h);
		glVertex2i  (__w, 0);
		glVertex2i  (0, __h);
	}
	glEnd();

	trssRender();

	glPopMatrix();

	glFlush();
}
