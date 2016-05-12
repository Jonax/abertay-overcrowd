//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	DEFINES
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define WIN32_LEAN_AND_MEAN		// Excludes rarely-used Win32 files to reduce compilation time.  

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	LIBRARY INCLUDES
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Win32.h"				// Win32 handler class.  

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	GLOBAL OBJECT
//	The only global object in the application.  All other objects & variables are kept within others classes.  
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Win32 OS;						// Win32 handler.  

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	WIN32 MESSAGE HANDLER
//	Receives & handles all messages sent via Win32.  Includes keyboard input, mouse input, and asynchronous network input.  
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	OS.MsgProcess(message, wParam, lParam);					// Sends the message to the Win32 handler.  

	return DefWindowProc(hwnd, message, wParam, lParam);	// Handles the rest of the message.  
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	MAIN FUNCTION
//	The top-level function of the application, used to run the Win32 handler.  
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	// Checks whether the WinProc function above has been registered with the Win32 handler.  
	if (!OS.Register(hInstance, WndProc))
		return NULL;

	// Checks whether a Win32 window has been created for the application.  
	if (!OS.Create(hInstance))
		return NULL;

	OS.AttachRenderer();	// Attaches the Direct3D renderer to the Win32 handler.  
	OS.Show();				// Shows the Win32 window on the screen.  

	// Main render loop.  Finishes until a message is sent to quit.  
	bool Continue = true;
	while (Continue)
		Continue = OS.Render();

	OS.Unregister();		// Unregisters the Win32 window as part of shutdown.  

	return OS.GetReturn();	// Ends with a return code from the Win32 handler.  
}