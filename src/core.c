#include "screen.h"

#include <stdio.h>


int __h;
int __w;

void coreReshape(int width, int height)
{
	// printf("coreReshape(int width, int height)\n");
	__h = height;
	__w = width;

	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, width, height, 0);
	glMatrixMode(GL_MODELVIEW);
}

void coreInit()
{
	// printf("coreInit()\n");
	glClearColor(0, 0, 0, 0);
	// glEnable(GL_TEXTURE_RECTANGLE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
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

	glPopMatrix();

	glFlush();
	// platformSwapBuffers();
}
