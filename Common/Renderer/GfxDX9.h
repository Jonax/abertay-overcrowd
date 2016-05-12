#ifndef _GFX_DX9_H_
#define _GFX_DX9_H_

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	LIBRARY INCLUDES
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <d3dx9.h>
#include <XInput.h>
#include "../Testbed.h"
#include "../Camera.h"
#include "../FPS_Stats.h"
#include "../OVCCrowd.h"

#define INPUT_DEADZONE		(0.24f * FLOAT(0x7FFF))  // Default to 24% of the +/- 32767 range.   This is a reasonable default value but can be altered if needed.

struct CONTROLLER_STATE
{
    XINPUT_STATE    state;       
    bool            bConnected;
};

class GfxDX9
{
	public:
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//	CLASS MEMBERS
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		GfxDX9(HWND hwnd);											// Class constructor.  
		~GfxDX9();													// Class destructor.  

		void Render(float TimeDelta);				// Handles frame rendering.  

		void Initialise();							// Handles the renderer's initialisation & configuration.  

		void Focus(WPARAM wParam);
		void Mouse(UINT message, POINTS Cursor);	// Handles signals from the mouse.  
		void Menu(WPARAM wParam);					// Handles signals from the application menu.  

	private:
		void					CreateDevice(DWORD VPCaps, D3DFORMAT Backbuffer);		// Creates the Direct3D device.
		D3DPRESENT_PARAMETERS	SetParameters(D3DFORMAT BackBuffer);					// Sets various Direct3D parameters. 

		void ReadX360Pad(float TimeDelta);

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//	CLASS OBJECTS
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		HWND				hwnd;			// Handle for the application's handle.
		LPDIRECT3D9			D3D;			// Direct3D interface.  
		LPDIRECT3DDEVICE9	Device;			// Direct3D device.  

		ID3DXEffect*		HLSL;		// Handle to a loaded HLSL shader.  

		Testbed*			Club;
		OVCCrowd*			Crowd;

		FPS_Stats*			FPS;

		Camera*				MainCam;

		CONTROLLER_STATE	Pad;
		bool				Pressed_LeftShoulder, Pressed_RightShoulder, Pressed_A, Pressed_B, Pressed_X, Pressed_Y;

		float y; // DEBUG CODE;
};

#endif