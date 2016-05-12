#ifndef _OVC_CROWD_H_
#define _OVC_CROWD_H_

#include <d3dx9.h>
#include <sstream>
#include <vector>
#include "Presence.h"
#include "OpenSteer/Boids.h"

#include <iostream>
#include <fstream>
using namespace std;

#ifdef TIGER
	#define RADIUS 1.0f
#else
	#define RADIUS 1.8f
#endif

struct INSTANCE
{
	D3DXMATRIX World;
};

class OVCCrowd : public BoidsPlugIn
{
	public:
		OVCCrowd(LPDIRECT3DDEVICE9 Device, LPD3DXEFFECT HLSL);
		~OVCCrowd();

		void Update(D3DXMATRIX &VP, float dt);
		void Render(D3DXMATRIX &VP, float dt);

		void AddInstances(int n);
		void RemoveInstances(int n);

		void SwitchInstancing();
		void SwitchBoids();
		void SwitchFrustum();
		// void SwitchAnimation();				// Disabled due to issues with skeletal animation

	private:
		void Initialise();

		void RenderRegular(D3DXMATRIX &VP);
		void RenderInstancing(D3DXMATRIX &VP);

		void ReadyBatch(D3DXMATRIX &VP);
		void LoadXFile(char* filename);

		bool UseInstancing, UseBoids, UseFrustum, UseAnimation;

		D3DXPLANE Plane[6];

		LPDIRECT3DDEVICE9				Device;
		LPD3DXEFFECT					HLSL;		// Handle to a loaded HLSL shader.  
		LPDIRECT3DVERTEXDECLARATION9	VD_Geometry, VD_Combined;
		LPDIRECT3DVERTEXBUFFER9			GeometryPacket, CrowdInstances;
		LPDIRECT3DINDEXBUFFER9			IndexPacket;
		UINT							VDG_Size, VDI_Size;

		LPD3DXMESH						Mesh;	// define the mesh pointe
		LPDIRECT3DTEXTURE9*				Texture;    // a pointer to a texture
		D3DCOLORVALUE*					Diffuse;		// define the material object
		DWORD							numMaterials;	// stores the number of materials in the mesh

		LPD3DXFONT Font;    // the pointer to the font object
		RECT TextInstances, TextInstancing, TextBoids, TextFrustum, TextAnimation;
		string		LabelInstances, LabelInstancing, LabelBoids, LabelFrustum, LabelAnimation;

		vector<Presence*>::iterator MemberList;

		ofstream myfile;

		unsigned int batch_size;

};

#endif