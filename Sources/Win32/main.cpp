
#include <windows.h>
#include "glee.h"
#include "Common.h"
#include "GameDefines.h"
#include "GameVariables.h"
#include "Vector.h"
#include "Core.h"
#include "Sprites.h"
#include "GUIControls.h"
#include "Game.h"
#include "Menu.h"
#include "main.h"

//#define TC_USE_CONSOLE

HGLRC           hRC = NULL;
HDC             hDC = NULL;

float input_x = 0.0f;
float input_y = 0.0f;
BOOL  input_button_down = FALSE;

const int V_SX = 640;
const int V_SY = 480;
const float PIXEL_SCALE = 2.0f;

BOOL OpenGL_Init(HWND hWnd)
{
	GLuint		PixelFormat;

	static PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),	// Size Of This Pixel Format Descriptor
		1,								// Version Number
		PFD_DRAW_TO_WINDOW |			// Format Must Support Window
		PFD_SUPPORT_OPENGL |			// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,				// Must Support Double Buffering
		PFD_TYPE_RGBA,					// Request An RGBA Format
		32,								// Select Our Color Depth
		0, 0, 0, 0, 0, 0,				// Color Bits Ignored
		0,								// No Alpha Buffer
		0,								// Shift Bit Ignored
		0,								// No Accumulation Buffer
		0, 0, 0, 0,						// Accumulation Bits Ignored
		16,								// 16Bit Z-Buffer (Depth Buffer)
		0,								// No Stencil Buffer
		0,								// No Auxiliary Buffer
		PFD_MAIN_PLANE,					// Main Drawing Layer
		0,								// Reserved
		0, 0, 0							// Layer Masks Ignored
	};		

	if (!(hDC = GetDC(hWnd)))
	{
		OpenGL_Release(hWnd);
		MessageBox(NULL, "Can't Create A GL Device Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return false;
	}		

	if (!(PixelFormat = ChoosePixelFormat(hDC, &pfd)))
	{
		OpenGL_Release(hWnd);
		MessageBox(NULL, "Can't Find A Suitable PixelFormat.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return false;
	}		

	if(!SetPixelFormat(hDC, PixelFormat, &pfd))
	{
		OpenGL_Release(hWnd);
		MessageBox(NULL, "Can't Set The PixelFormat.", "ERROR", MB_OK|MB_ICONEXCLAMATION);
		return false;
	}		
	
	if (!(hRC = wglCreateContext(hDC)))
	{
		OpenGL_Release(hWnd);
		MessageBox(NULL, "Can't Create A GL Rendering Context.", "ERROR", MB_OK|MB_ICONEXCLAMATION);
		return false;
	}		
	
	if(!wglMakeCurrent(hDC, hRC))
	{
		OpenGL_Release(hWnd);
		MessageBox(NULL, "Can't Activate The GL Rendering Context.", "ERROR", MB_OK|MB_ICONEXCLAMATION);
		return false;
	}		
	
	return true;
}

void OpenGL_Release(HWND hWnd)
{
	if (hRC)
	{		
		if (!wglMakeCurrent(NULL,NULL))
		{		
			MessageBox(NULL, "Release Of DC And RC Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		}		

		if (!wglDeleteContext(hRC))
		{		
			MessageBox(NULL, "Release Rendering Context Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		}

		hRC = NULL;
	}		

	ReleaseDC(hWnd, hDC);
}

LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	LRESULT result = 0;

	static POINT cursor_pos;
	static BOOL show_crosshair = TRUE;

	switch (msg)
	{
		case WM_RBUTTONDOWN:
			input_x = LOWORD(lParam);
			input_y = HIWORD(lParam);

			input_x /= PIXEL_SCALE;
			input_y /= PIXEL_SCALE;

			show_crosshair = !show_crosshair;

			if (show_crosshair)
			{
				ShowCursor(TRUE);
				Core_InputTouchEnded(input_x, v_sy - input_y);
			}
			else
			{
				ShowCursor(FALSE);
				GetCursorPos(&cursor_pos);
				Core_InputTouchBegan(input_x, v_sy - input_y);
			}
			break;

		case WM_LBUTTONDOWN:
			if (!show_crosshair)
				break;

			input_x = LOWORD(lParam);
			input_y = HIWORD(lParam);

			input_x /= PIXEL_SCALE;
			input_y /= PIXEL_SCALE;

			if (!input_button_down)
			{
				input_button_down = TRUE;
				Core_InputTouchBegan(input_x, v_sy - input_y);
			}
			break;

		case WM_LBUTTONUP:
			if (!show_crosshair)
				break;

			input_x = LOWORD(lParam);
			input_y = HIWORD(lParam);

			input_x /= PIXEL_SCALE;
			input_y /= PIXEL_SCALE;

			if (input_button_down)
			{
				input_button_down = FALSE;
				Core_InputTouchEnded(input_x, v_sy - input_y);
			}
			break;

		case WM_MOUSEMOVE:
			if (input_button_down)
			{
				input_x = LOWORD(lParam);
				input_y = HIWORD(lParam);

				input_x /= PIXEL_SCALE;
				input_y /= PIXEL_SCALE;

				Core_InputTouchMoved(input_x, v_sy - input_y);
			}

			if (!show_crosshair)
			{
				POINT current_cursor_pos;
				GetCursorPos(&current_cursor_pos);

				float x = (float)(current_cursor_pos.x - cursor_pos.x);
				float y = (float)(current_cursor_pos.y - cursor_pos.y);

				x /= PIXEL_SCALE;
				y /= PIXEL_SCALE;

				input_x += x;
				input_y += y;
				SetCursorPos(cursor_pos.x, cursor_pos.y);

				Core_InputTouchMoved(input_x, v_sy - input_y);
			}
			break;

		case WM_SYSKEYDOWN:
			break;

		case WM_SYSKEYUP:
			break;

		case WM_KEYDOWN:
		{
			if (wParam == VK_LEFT || wParam == 'A')
				left_engine_override = TRUE;

			if (wParam == VK_RIGHT || wParam == 'D')
				right_engine_override = TRUE;

			break;
		}

		case WM_KEYUP:
		{
			if (wParam == VK_LEFT || wParam == 'A')
				left_engine_override = FALSE;

			if (wParam == VK_RIGHT || wParam == 'D')
				right_engine_override = FALSE;

			break;
		}
		
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		default:
			result = DefWindowProc(hWnd, msg, wParam, lParam);
	}

	return result;
}

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
#ifdef TC_USE_CONSOLE
	AllocConsole();

	freopen( "CON", "w", stdout );
	freopen( "CON", "w", stderr );
#endif

	WNDCLASS wc;
	memset( &wc, 0, sizeof( wc ) );

	wc.style         = 0;
	wc.lpfnWndProc   = (WNDPROC) MsgProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = NULL;
	wc.hCursor       = LoadCursor (NULL,IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = "tremor_core";

	if (!RegisterClass (&wc))
		return 0;

	HWND hWnd = CreateWindowEx(0, "tremor_core", GAME_NAME, WS_OVERLAPPED | WS_EX_TOPMOST | WS_CAPTION | WS_BORDER | WS_SYSMENU,
							   100, 100, V_SX, V_SY, NULL, NULL, hInstance, NULL);
    
	RECT window_rect;
	GetWindowRect(hWnd, &window_rect);
	RECT client_rect;
	GetClientRect( hWnd, &client_rect );
	INT w_sx = V_SX + (window_rect.right - window_rect.left) - (client_rect.right - client_rect.left);
	INT w_sy = V_SY + (window_rect.bottom - window_rect.top) - (client_rect.bottom - client_rect.top);
	INT s_sx = GetSystemMetrics(SM_CXSCREEN);
	INT s_sy = GetSystemMetrics(SM_CYSCREEN);
	if (w_sx > s_sx)
		w_sx = s_sx;
	if (w_sx > s_sy)
		w_sy = s_sy;
	INT w_x = (s_sx - w_sx) / 2;
	INT w_y = (s_sy - w_sy) / 2;
	if (w_x < 0) w_x = 0;
	if (w_y < 0) w_y = 0;
	MoveWindow( hWnd, w_x, w_y, w_sx, w_sy, TRUE );

	OpenGL_Init(hWnd);

	Core_Init(V_SX, V_SY, PIXEL_SCALE, SCREEN_SIZE_SMALL);

	gui_enable_touches_reset = FALSE;

	ShowWindow( hWnd, SW_SHOWDEFAULT );
	UpdateWindow( hWnd );
		
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg,NULL, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		Core_Process();

		Core_Render();
		SwapBuffers(hDC);

		Sleep(15);
	}
	
	Core_Release();

	OpenGL_Release(hWnd);

	DestroyWindow( hWnd );

	UnregisterClass( "tremor_core", hInstance );
	
	return 0;
}

