#include "screen.h"

LONG WINAPI win32WndProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{ 
	static PAINTSTRUCT ps;

	switch(msg)
	{
	case WM_PAINT:
		{
			coreRender();
			BeginPaint(wnd, &ps);
			EndPaint(wnd, &ps);
			SwapBuffers(ps.hdc);
		}
		break;

	case WM_SIZE:
		{
			// int wndDw;
			// int wndDh;
			// wndDw = LOWORD(lParam) % 8;
			// wndDh = HIWORD(lParam) % 16;

			// if (wndDw || wndDh)
			// {
			// 	SetWindowPos(wnd, 0, 0, 0, 250, 250, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
			// 	// SetWindowPos(wnd, 0, 0, 0, LOWORD(lParam) - wndDw, HIWORD(lParam) - wndDh, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
			// }
			// else
			// {
			// }		
			coreReshape(LOWORD(lParam), HIWORD(lParam));
			PostMessage(wnd, WM_PAINT, 0, 0);
		}
		break;

	// case WM_CHAR:
	// switch (wParam) {
	// }
	// break;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(wnd, msg, wParam, lParam); 
} 

HWND win32CreateViewport(char* title, int x, int y, int width, int height)
{
	HWND        wnd;
	WNDCLASS    wc;

	PIXELFORMATDESCRIPTOR pfd;
	int pf;
	HDC hdc;
	static HINSTANCE instance = 0;

	// only register the window class once - use hInstance as a flag.
	if (!instance)
	{
		instance = GetModuleHandle(NULL);
		wc.style         = CS_OWNDC;
		wc.lpfnWndProc   = (WNDPROC)win32WndProc;
		wc.cbClsExtra    = 0;
		wc.cbWndExtra    = 0;
		wc.hInstance     = instance;
		wc.hIcon         = LoadIcon(NULL, IDI_WINLOGO);
		wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = NULL;
		wc.lpszMenuName  = NULL;
		wc.lpszClassName = "ScreenKit";

		if (!RegisterClass(&wc))
		{
			return NULL;
		}
	}

	wnd = CreateWindow(
		"ScreenKit",
		title,
		WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		x,
		y,
		width,
		height,
		NULL,
		NULL,
		instance,
		NULL
		);

	if (!wnd)
	{
		return NULL;
	}

	memset(&pfd, 0, sizeof(pfd));
	pfd.nSize        = sizeof(pfd);
	pfd.nVersion     = 1;
	pfd.dwFlags      = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType   = PFD_TYPE_RGBA;
	pfd.cColorBits   = 32;

	hdc = GetDC(wnd);
	pf = ChoosePixelFormat(hdc, &pfd);
	if (!pf)
	{
		return NULL;
	} 

	if (!SetPixelFormat(hdc, pf, &pfd))
	{
		return NULL;
	} 

	DescribePixelFormat(hdc, pf, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

	ReleaseDC(wnd, hdc);
	return wnd;
}    

static CRITICAL_SECTION win32Lock;

DWORD WINAPI win32MainLoop(LPVOID lParam)
{
	HDC   hdc;
	HGLRC hrc;
	HWND  wnd;
	MSG   msg;

	int result;

	EnterCriticalSection(&win32Lock);

	wnd = win32CreateViewport("ScreenKit P2.1 Demo", 0, 0, 256, 256);
	if (!wnd)
	{
		return ERR_SYSTEM;
	}

	hdc = GetDC(wnd);
	hrc = wglCreateContext(hdc);
	wglMakeCurrent(hdc, hrc);

	result = coreInit();
	if (result != ERR_OK)
	{
		return result;
	}

	ShowWindow(wnd, SW_SHOWDEFAULT);

	LeaveCriticalSection(&win32Lock);
	
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	wglMakeCurrent(NULL, NULL);
	ReleaseDC(wnd, hdc);
	wglDeleteContext(hrc);
	DestroyWindow(wnd);

	return msg.wParam;
}

int platformOpenScreen()
{
	HANDLE thread;
	int result = ERR_OK;

	InitializeCriticalSectionAndSpinCount(&win32Lock, 0x00000400);

	// return win32MainLoop(NULL);
	thread = CreateThread(NULL, 0, win32MainLoop, NULL, 0, NULL);
	if (thread)
	{
		EnterCriticalSection(&win32Lock);

		if (WaitForSingleObject(thread, 0) == WAIT_OBJECT_0)
		{
			if (!GetExitCodeThread(thread, &result))
			{
				result = ERR_SYSTEM;
			}

			CloseHandle(thread);
		}
		
		LeaveCriticalSection(&win32Lock);
	}
	else
	{
		result = ERR_SYSTEM;
	}

	// DeleteCriticalSection(&win32Lock);
	return result;
}
