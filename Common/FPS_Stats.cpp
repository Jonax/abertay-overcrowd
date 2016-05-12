#include "FPS_Stats.h"

FPS_Stats::FPS_Stats(LPDIRECT3DDEVICE9 Device)
{
	this->Device	= Device;
	this->Font		= NULL;
	this->Label		= "Getting FPS";

	this->numFrames		= 0.0f;
	this->timeElapsed	= 0.0f;

	D3DXCreateFont(	this->Device,				// the D3D Device
					16,							// font height of 30
					0,							// default font width
					FW_BOLD,					// font weight
					1,							// not using MipLevels
					false,						// non-italic font
					DEFAULT_CHARSET,			// default character set
					OUT_DEFAULT_PRECIS,			// default OutputPrecision,
					ANTIALIASED_QUALITY,		// default Quality
					DEFAULT_PITCH | FF_DONTCARE,// default pitch and family
					"Arial",					// use Facename Arial
					&this->Font);				// the font object

	SetRect(&this->Dimensions, 0, 112, 300, 128);		// Using parts of each corner to ensure correct dimensions.  
}

FPS_Stats::~FPS_Stats()
{
	this->Font		= NULL;
	this->Device	= NULL;
}

void FPS_Stats::Update(float dt)
{
	std::stringstream ss;

	numFrames += 1.0f;
	timeElapsed += dt;

	if (timeElapsed >= 1.0f)
	{
		mFPS = numFrames;

		this->timeElapsed	= 0.0f;
		this->numFrames		= 0.0f;

		ss << "FPS: " << mFPS;

		Label.clear();
		Label = ss.str();
	}
}

void FPS_Stats::Render()
{
	Font->DrawText(NULL, Label.c_str(), Label.length(), &this->Dimensions, DT_LEFT | DT_TOP, D3DCOLOR_ARGB(255, 255, 255, 255));
}