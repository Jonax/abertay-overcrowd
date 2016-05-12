#include "Win32.h"

//	Class constructor.  
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Win32::Win32()
{
	__int64 cntsPerSec = 0;

	QueryPerformanceFrequency((LARGE_INTEGER*)&cntsPerSec);
	this->secsPerCnt = 1.0f / (float)cntsPerSec;

	this->prevTimeStamp = 0;
	QueryPerformanceCounter((LARGE_INTEGER*)&this->prevTimeStamp);
}

//	Class constructor.  
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Win32::~Win32()
{
}

//	Function to register the window.  
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Win32::Register(HINSTANCE hInstance, WNDPROC WndProc)
{
	// Sets up the window class for registration
	windowClass.cbSize			= sizeof(WNDCLASSEX);				// Size of the WNDCLASSEX structure.
	windowClass.style			= CS_HREDRAW | CS_VREDRAW;			// Redraws window if resized.  
	windowClass.lpfnWndProc		= WndProc;							// Address to the message handler above.  
	windowClass.cbClsExtra		= 0;								// No extra class information.
	windowClass.cbWndExtra		= 0;								// No extra window information.
	windowClass.hInstance		= hInstance;						// Handle for the application's instance.
	windowClass.hIcon			= LoadIcon(NULL, IDI_APPLICATION);	// Standard application icon.
	windowClass.hCursor			= LoadCursor(NULL, IDC_ARROW);		// Standard mouse cursor.
	windowClass.hbrBackground	= NULL;								// No background is drawn (due to Direct3D doing it).  
	windowClass.lpszMenuName	= NULL;//(LPCSTR) IDR_MENU1;				// No menu used.  
	windowClass.lpszClassName	= "OverCrowd";						// Internal name for application.  
	windowClass.hIconSm			= LoadIcon(NULL, IDI_APPLICATION);	// Standard window icon when minimalised.  

	// Registers the window class.  If it fails, the application wraps up.  
	return (bool)RegisterClassEx(&this->windowClass);
}

//	Function to create the Win32 window based on the created window class.  
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Win32::Create(HINSTANCE hInstance)
{
	// Creates the actual window as well as the handle to the window.  
	this->hwnd = CreateWindowEx(NULL,								// No extended styles needed.  
								"OverCrowd",						// Links to created window class.  
								"OverCrowd 0.1.5 - Honours Project",// Caption for the window.
								WS_OVERLAPPEDWINDOW |				// Style for the window.  Creates a standard window...
								WS_CLIPCHILDREN | WS_CLIPSIBLINGS,	// ...And clips areas overlapped by other windows.  
								0,									// Position of the window, should default to (0, 0).   
								0,
								1024,								// Side of the window, should default to as large as possible.  
								768, 
								NULL,								// No parent windows used.  
								NULL,								// No menu used.
								hInstance,							// Handle to the application instance.
								NULL);								// Pointer to a window, not required.

	// Checks that the window was actually created.  If it wasn't (i.e. there was an error creating it), the application wraps up.  
	if (!hwnd)
	{
		this->Unregister();
		return false;
	}
	else
		return true;
}

//	Function to unregister the window class as part of shutdown.  
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Win32::Unregister()
{
	UnregisterClass("OverCrowd", windowClass.hInstance);		// Unregisters the previous-created window class.  
}

//	Main rendering loop.  
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Win32::Render()
{
	// Checks whether a quit message exists.  If it does, the application will handle cleanup at the end of the current frame.  
	if (PeekMessage(&this->msg, NULL, NULL, NULL, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
			return false;

		TranslateMessage(&this->msg);
		DispatchMessage(&this->msg);
	}
	else
	{
		__int64 currTimeStamp = 0;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTimeStamp);
		float dt = (currTimeStamp - this->prevTimeStamp) * this->secsPerCnt;

		// For the remainder of the application though, the application draws via the renderer.
		Renderer->Render(dt);		// Renders based on time.  

		this->prevTimeStamp = currTimeStamp;
	}

	return true;
}

//	Main message handler for the application.  
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Win32::MsgProcess(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		// On event of a menu option being selected, the menu ID is sent to the renderer.  
		case WM_COMMAND:
			Renderer->Menu(wParam);
			break;

		// On event of any mouse-related event, message type & Cursor position is sent to the renderer.  
		case WM_LBUTTONDOWN:		// Left mouse button is pressed.  
		case WM_LBUTTONUP:			// Left mouse button is released.  
		case WM_RBUTTONDOWN:		// Right mouse button is pressed.  
		case WM_RBUTTONUP:			// Right mouse button is released.  
		case WM_MOUSEMOVE:			// Mouse is moved.  
			Renderer->Mouse(message, MAKEPOINTS(lParam));
			break;

		// On event of the window focus changing between applications, send to all relevant areas.
		// Case is used to activate/deactivate XInput.  
		case WM_ACTIVATEAPP:
			Renderer->Focus(wParam);
			break;

		// Sends quit message if forced to close.  
		case WM_DESTROY:						
		case WM_CLOSE:
			PostQuitMessage(0);
			break;
	}
}

//	Function to show the window.  
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Win32::Show()
{
	ShowWindow(this->hwnd, SW_SHOW);
}

//	Function to initialise & attach the required Direct3D renderer.  
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Win32::AttachRenderer()
{
	this->Renderer = new GfxDX9(this->hwnd);	// Creates the renderer for Direct3D 9.  
	Renderer->Initialise();
}

//	Function to send a return to the Win32 handler for shutdown, based on the wParam of the last message.  
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int Win32::GetReturn()
{
	return (int)msg.wParam;
}