#ifndef _FPS_STATS_H_
#define _FPS_STATS_H_

#include <string>
#include <sstream>
#include <d3dx9.h>

class FPS_Stats
{
	public:
		FPS_Stats(LPDIRECT3DDEVICE9 Device);
		~FPS_Stats();

		void Update(float dt);
		void Render();

		void StartCPUClock();
		void StopCPUClock(float TimeRate);

	private:
		LPDIRECT3DDEVICE9 Device;
		LPD3DXFONT Font;    // the pointer to the font object
		RECT Dimensions;
		std::string		Label;

		float numFrames;
		float timeElapsed;

		float mFPS;
		float mMilliSecPerFrame;
};

#endif