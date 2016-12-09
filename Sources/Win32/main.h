
#ifndef MAIN_H
#define MAIN_H


BOOL OpenGL_Init(HWND hWnd);
void OpenGL_Release(HWND hWnd);

LRESULT WINAPI	 MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
INT WINAPI		 WinMain( HINSTANCE hInst, HINSTANCE, LPSTR, INT );


#endif /* MAIN_H */