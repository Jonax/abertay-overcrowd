#ifndef _TESTBED_H_
#define _TESTBED_H_

#include <d3dx9.h>

class Testbed
{
	public:
		Testbed(LPDIRECT3DDEVICE9 Device, LPD3DXEFFECT HLSL);
		~Testbed();

		void Render();

	private:
		void Initialise();
		void LoadXFile(char* filename, const D3DVERTEXELEMENT9* VertexElements);

		LPDIRECT3DDEVICE9				Device;
		LPD3DXEFFECT					HLSL;		// Handle to a loaded HLSL shader.  
		LPDIRECT3DVERTEXDECLARATION9	VD;

		LPD3DXMESH						Mesh;	// define the mesh pointe
		LPDIRECT3DTEXTURE9*				Texture;    // a pointer to a texture
		D3DCOLORVALUE*					Diffuse;		// define the material object
		DWORD							numMaterials;	// stores the number of materials in the mesh

		D3DXMATRIX						World;
};

#endif