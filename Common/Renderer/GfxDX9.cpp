#include "GfxDX9.h"

//	Class constructor.  
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
GfxDX9::GfxDX9(HWND hwnd)
{
	this->hwnd			= hwnd;
	this->D3D			= NULL;		// Initialises the Direct3D interface as blank.  
	this->Device		= NULL;		// Initialises the Direct3D device as blank.  
	this->HLSL			= NULL;

	this->y				= 0.5f;		// DEBUG
}

//	Class destructor.  
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
GfxDX9::~GfxDX9()
{
	delete this->MainCam;
	this->MainCam	= NULL;

	delete this->FPS;
	this->FPS		= NULL;

	Crowd->close();
	delete this->Crowd;
	this->Crowd		= NULL;

	delete this->Club;
	this->Club		= NULL;

	if (this->HLSL)
		HLSL->Release();
	this->HLSL = NULL;

	// Releases the Direct3D device.  
    if (Device)
        Device->Release();
	this->Device	= NULL;

	// Releases the Direct3D interface.  
    if (D3D)
        D3D->Release();
	this->D3D		= NULL;

	this->hwnd		= NULL;
}

//	Function to process the rendering of each frame.  This overrides the base class.  
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void GfxDX9::Render(float TimeDelta)
{
	this->ReadX360Pad(TimeDelta * 5.0f);
	FPS->Update(TimeDelta);

	HLSL->SetMatrix("mVP", &MainCam->GetMatrix());

	// Clears the screen.  
	Device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f), 1.0f, 0);

	Device->BeginScene();						// Begins drawing the scene.  
		Club->Render();
		Crowd->Render(MainCam->GetMatrix(), TimeDelta);
		FPS->Render();
    Device->EndScene();							// Ends drawing the scene.  

    Device->Present(NULL, NULL, NULL, NULL);	// Presents the next backbuffer. 
}

//	Function to initialise & configure the Direct3D renderer.  This overrides the base class.  
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void GfxDX9::Initialise()
{
	D3DDISPLAYMODE DisplayMode;		// Direct3D display mode.  
	D3DCAPS9 Caps;					// Direct3D device capabilities.  

	// Creates the Direct3D device.  
	if (!(D3D = Direct3DCreate9(D3D_SDK_VERSION)))
		return;

	// Acquires the necessary display mode.  
    if (FAILED(D3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &DisplayMode)))
		return;

	// Checks the format for the Direct3D device is valid.  
	if (D3D->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, DisplayMode.Format, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D16) == D3DERR_NOTAVAILABLE)
		return;

	// Acquires the capabilities of the device.  
	if (FAILED(D3D->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &Caps)))
		return;

	// Creates the Direct3D device.  
	this->CreateDevice(Caps.VertexProcessingCaps, DisplayMode.Format);

	// Sets lighting to off & no culling to be used.  
	Device->SetRenderState(D3DRS_LIGHTING, FALSE);
	Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

	// If the shader can't be created by the given .fx file, the error is put into the created buffer and is used in a message box.  
	LPD3DXBUFFER Error = NULL;		// Buffer is created in case an error is called.  
	if FAILED(D3DXCreateEffectFromFile(this->Device, "Render.fx", NULL, NULL, 0, NULL, &this->HLSL, &Error))
	{
		MessageBox(NULL, (LPCSTR)Error->GetBufferPointer(), "Fx Compile Error", MB_OK | MB_ICONEXCLAMATION);
		PostQuitMessage(0);
	}

	this->MainCam		= new Camera();
	this->FPS			= new FPS_Stats(Device);
	this->Club			= new Testbed(Device, HLSL);
	this->Crowd			= new OVCCrowd(Device, HLSL);
	Crowd->open();

	//Crowd->update(0.0016f /*clock.elapsedSimulationTime*/);  // Enable only when required to start with boids.

	HLSL->SetValue("Light.Position", &D3DXVECTOR3(0.0f, 5.0f, 0.0f), sizeof(D3DXVECTOR3));
	HLSL->SetVector("Light.Diffuse", &D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f));
}

void GfxDX9::Focus(WPARAM wParam)
{
	XInputEnable((bool)wParam);
}

//	Function to forward mouse messages to the tech demo environment.  This overrides the base class.  
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void GfxDX9::Mouse(UINT message, POINTS Cursor)
{
}

//	Function to forward menu messages to the tech demo environment.  This overrides the base class.  
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void GfxDX9::Menu(WPARAM wParam)
{
}

//	Function to create the Direct3D device.  
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void GfxDX9::CreateDevice(DWORD VPCaps, D3DFORMAT Backbuffer)
{
	ZeroMemory(&Device, sizeof(LPDIRECT3DDEVICE9));

	if (FAILED(D3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, this->hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &this->SetParameters(Backbuffer), &this->Device)))
	{
		MessageBox(this->hwnd, "Device Failed", "DEBUG", MB_OK);
		return;
	}
}

//	Function to set the various Direct3D parameters.  
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
D3DPRESENT_PARAMETERS GfxDX9::SetParameters(D3DFORMAT BackBuffer)
{
	D3DPRESENT_PARAMETERS D3D_PP;

	ZeroMemory(&D3D_PP, sizeof(D3DPRESENT_PARAMETERS));

	D3D_PP.BackBufferCount			= 1;								// One extra backbuffer.  
    D3D_PP.BackBufferFormat			= BackBuffer;						// Format taken from the saved display mode.  
	D3D_PP.SwapEffect				= D3DSWAPEFFECT_DISCARD;			// Discards previously-rendered frames.  
    D3D_PP.EnableAutoDepthStencil	= true;
    D3D_PP.AutoDepthStencilFormat	= D3DFMT_D16;
    D3D_PP.PresentationInterval		= D3DPRESENT_INTERVAL_IMMEDIATE;
	D3D_PP.Windowed					= true;								// Application is windowed.  

	return D3D_PP;
}

void GfxDX9::ReadX360Pad(float TimeDelta)
{
	if (XInputGetState(0, &Pad.state) == ERROR_SUCCESS)
	{
		// Zero value if thumbsticks are within the dead zone
		if (Pad.state.Gamepad.sThumbLX < -INPUT_DEADZONE)
			MainCam->Left(TimeDelta);
		else if (Pad.state.Gamepad.sThumbLX > INPUT_DEADZONE)
			MainCam->Right(TimeDelta);
		else
			Pad.state.Gamepad.sThumbLX = 0;

		if (Pad.state.Gamepad.sThumbLY < -INPUT_DEADZONE)
			MainCam->Down(TimeDelta);
		else if (Pad.state.Gamepad.sThumbLY > INPUT_DEADZONE)
			MainCam->Up(TimeDelta);
		else
            Pad.state.Gamepad.sThumbLY = 0;

        if (	(Pad.state.Gamepad.sThumbRX < INPUT_DEADZONE)
			&&	(Pad.state.Gamepad.sThumbRX > -INPUT_DEADZONE)
			&&	(Pad.state.Gamepad.sThumbRY < INPUT_DEADZONE)
			&&	(Pad.state.Gamepad.sThumbRY > -INPUT_DEADZONE))
        {
            Pad.state.Gamepad.sThumbRX = 0;
            Pad.state.Gamepad.sThumbRY = 0;
        }

		if (Pad.state.Gamepad.bLeftTrigger >= 127)
			Crowd->RemoveInstances(1);

		if (Pad.state.Gamepad.bRightTrigger >= 127)
			Crowd->AddInstances(1);

		WORD wButtons = Pad.state.Gamepad.wButtons;

		if (wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)
		{
			if (!this->Pressed_LeftShoulder)
			{
				Crowd->RemoveInstances(100);
				this->Pressed_LeftShoulder = true;
			}
		}
		else if (this->Pressed_LeftShoulder)
			this->Pressed_LeftShoulder = false;

		if (wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)
		{
			if (!this->Pressed_RightShoulder)
			{
				Crowd->AddInstances(100);
				this->Pressed_RightShoulder = true;
			}
		}
		else if (this->Pressed_RightShoulder)
			this->Pressed_RightShoulder = false;

		// A = Geometry Instancing
		if (wButtons & XINPUT_GAMEPAD_A)
		{
			if (!this->Pressed_A)
			{
				Crowd->SwitchInstancing();
				this->Pressed_A = true;
			}
		}
		else if (this->Pressed_A)
			this->Pressed_A = false;

		// X = Boids Movement
		if (wButtons & XINPUT_GAMEPAD_X)
		{
			if (!this->Pressed_X)
			{
				Crowd->SwitchBoids();
				this->Pressed_X = true;
			}
		}
		else if (this->Pressed_X)
			this->Pressed_X = false;

		// B = Frustum Culling
		if (wButtons & XINPUT_GAMEPAD_B)
		{
			if (!this->Pressed_B)
			{
				Crowd->SwitchFrustum();
				this->Pressed_B = true;
			}
		}
		else if (this->Pressed_B)
			this->Pressed_B = false;

		// Y = Skeletal Animation
		/*	The following block of code is disabled due to issues with skeletal animation

		if (wButtons & XINPUT_GAMEPAD_Y)
		{
			if (!this->Pressed_Y)
			{
				Crowd->SwitchAnimation();
				this->Pressed_Y = true;
			}
		}
		else if (this->Pressed_Y)
			this->Pressed_Y = false;
		*/
	}
}