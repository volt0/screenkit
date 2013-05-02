#include "screen.h"
#include "trss.h"

int __h;
int __w;
trssContext_t *__ctx;

int coreInit()
{
	glewInit();
	if (!GLEW_VERSION_2_0)
	{
		printf("OpenGL 2.0+ is required!\n");
		return 1;
	}

	glClearColor(0.5f, 0.5f, 0, 0);
	glEnable(GL_TEXTURE_RECTANGLE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	trssInit();
	__ctx = trssAlloc();

	return ERR_OK;
}

void coreReshape(int width, int height)
{
	__h = height;
	__w = width;
	trssReshape(__ctx, width, height);

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

	trssRender(__ctx);

	glPopMatrix();

	glFlush();
}
