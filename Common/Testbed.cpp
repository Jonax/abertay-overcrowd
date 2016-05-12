#include "Testbed.h"

Testbed::Testbed(LPDIRECT3DDEVICE9 Device, LPD3DXEFFECT HLSL)
{
	this->Device = Device;
	this->HLSL = HLSL;
	this->VD = NULL;

	this->Initialise();
}

Testbed::~Testbed()
{
	this->HLSL = NULL;
	this->Device = NULL;

	Mesh->Release();
	this->Mesh = NULL;
}

void Testbed::LoadXFile(char* filename, const D3DVERTEXELEMENT9* VertexElements)
{
	ID3DXMesh* TempMesh		= NULL;
	ID3DXBuffer* adjBuffer	= NULL;
	ID3DXBuffer* mtrlBuffer	= NULL;

	D3DXLoadMeshFromX(filename, D3DXMESH_SYSTEMMEM, this->Device, &adjBuffer, &mtrlBuffer, 0, &this->numMaterials, &TempMesh);

	TempMesh->CloneMesh(D3DXMESH_SYSTEMMEM, VertexElements, this->Device, &TempMesh);
	TempMesh->Optimize(D3DXMESH_MANAGED|D3DXMESHOPT_COMPACT|D3DXMESHOPT_ATTRSORT|D3DXMESHOPT_VERTEXCACHE, (DWORD*)adjBuffer->GetBufferPointer(), 0, 0, 0, &this->Mesh);
	TempMesh->Release();

	D3DXMATERIAL* tempMaterials = (D3DXMATERIAL*)mtrlBuffer->GetBufferPointer();

	this->Diffuse = new D3DCOLORVALUE[this->numMaterials];
	this->Texture = new LPDIRECT3DTEXTURE9[this->numMaterials];

	for (DWORD i = 0 ; i < this->numMaterials ; i++)		// for each material...
	{
		this->Diffuse[i] = tempMaterials[i].MatD3D.Diffuse;	// get the material info...

		if (FAILED(D3DXCreateTextureFromFile(this->Device, tempMaterials[i].pTextureFilename, &this->Texture[i])))
			this->Texture[i] = NULL;    // if there is no texture, set the texture to NULL
	}

	mtrlBuffer->Release();
}

void Testbed::Initialise()
{
	D3DVERTEXELEMENT9 VertexElements[] = 
	{
		{0,  0, D3DDECLTYPE_FLOAT4,	D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,	0},
		{0, 16, D3DDECLTYPE_FLOAT4,	D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,		0},
		{0, 32, D3DDECLTYPE_FLOAT2,	D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,	0},
		D3DDECL_END()
	};
	
	Device->CreateVertexDeclaration(VertexElements, &VD);

	this->LoadXFile("Testbed.x", VertexElements);

	D3DXMatrixIdentity(&World);
}

void Testbed::Render()
{
	UINT uPasses;

	HLSL->SetTechnique("Render");
	HLSL->SetMatrix("mWorld", &World);

	Device->SetVertexDeclaration(VD);

	HLSL->Begin(&uPasses, 0);
	for (UINT uPass = 0 ; uPass < uPasses ; ++uPass)
	{
		HLSL->BeginPass(uPass);

		for (DWORD i = 0 ; i < this->numMaterials ; i++)    // loop through each subset
		{
			HLSL->SetVector("Material.Diffuse", (D3DXVECTOR4*)&Diffuse[i]);

			if (Texture[i])    // if the subset has a texture (if texture is not NULL)
				HLSL->SetTexture("tTexture", Texture[i]);

			HLSL->CommitChanges();
			Mesh->DrawSubset(i);					// draw the subset
		}

		HLSL->EndPass();
	}
	HLSL->End();
}