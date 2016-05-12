#include "OVCCrowd.h"

OVCCrowd::OVCCrowd(LPDIRECT3DDEVICE9 Device, LPD3DXEFFECT HLSL)
: BoidsPlugIn()
{
	this->Device			= Device;
	this->HLSL				= HLSL;
	this->GeometryPacket	= NULL;
	this->IndexPacket		= NULL;
	this->CrowdInstances	= NULL;
	this->LabelInstances	= "";
	this->LabelInstancing	= "Geometric Instancing: Enabled";
	this->LabelBoids		= "Boids Animation: Enabled";
	this->LabelFrustum		= "Frustum Culling: Enabled";
	//this->LabelAnimation	= "Skeletal Animation: Enabled";			// Disabled due to issues with skeletal animation

	this->Initialise();

	this->UseInstancing	= true;
	this->UseBoids		= true;
	this->UseFrustum	= true;
	this->UseAnimation	= true;

	this->Font		= NULL;

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

	SetRect(&this->TextInstancing, 0, 0, 200, 16);
	SetRect(&this->TextBoids, 0, 16, 200, 32);
	SetRect(&this->TextFrustum, 0, 32, 200, 48);
	SetRect(&this->TextAnimation, 0, 48, 200, 64);

	SetRect(&this->TextInstances, 0, 80, 250, 96);
}

OVCCrowd::~OVCCrowd()
{
	this->Font		= NULL;

	flock.clear();

	Mesh->Release();
	this->Mesh = NULL;

	this->HLSL = NULL;
	this->Device = NULL;
}

void OVCCrowd::LoadXFile(char* filename)
{
	ID3DXMesh* TempMesh		= NULL;
	ID3DXBuffer* adjBuffer	= NULL;
	ID3DXBuffer* mtrlBuffer	= NULL;

	D3DVERTEXELEMENT9 VE_Geometry[] = 
	{
		{0,  0, D3DDECLTYPE_FLOAT4,	D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,	0},
		{0, 16, D3DDECLTYPE_FLOAT2,	D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,		0},
		{0, 32, D3DDECLTYPE_FLOAT2,	D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,	0},

		D3DDECL_END()
	};
	Device->CreateVertexDeclaration(VE_Geometry, &VD_Geometry);

	D3DXLoadMeshFromX(filename, D3DXMESH_MANAGED, this->Device, &adjBuffer, &mtrlBuffer, 0, &this->numMaterials, &TempMesh);

	TempMesh->CloneMesh(D3DXMESH_MANAGED, VE_Geometry, this->Device, &this->Mesh);
	Mesh->OptimizeInplace(D3DXMESH_MANAGED|D3DXMESHOPT_COMPACT|D3DXMESHOPT_VERTEXCACHE, (DWORD*)adjBuffer->GetBufferPointer(), NULL, NULL, NULL);
//	TempMesh->Optimize(D3DXMESH_MANAGED/*|D3DXMESHOPT_COMPACT|D3DXMESHOPT_ATTRSORT|D3DXMESHOPT_VERTEXCACHE*/, (DWORD*)adjBuffer->GetBufferPointer(), 0, 0, 0, &this->Mesh);
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

void OVCCrowd::Initialise()
{
	D3DVERTEXELEMENT9 VE_Combined[] = 
	{
		{0,  0, D3DDECLTYPE_FLOAT4,	D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0, 16, D3DDECLTYPE_FLOAT4,	D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,	  0},
		{0, 32, D3DDECLTYPE_FLOAT2,	D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
		{1,  0, D3DDECLTYPE_FLOAT4,	D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},
		{1, 16, D3DDECLTYPE_FLOAT4,	D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2},
		{1, 32, D3DDECLTYPE_FLOAT4,	D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 3},
		{1, 48, D3DDECLTYPE_FLOAT4,	D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 4},

		D3DDECL_END()
	};

	this->VDG_Size = D3DXGetDeclVertexSize(VE_Combined, 0);
	this->VDI_Size = D3DXGetDeclVertexSize(VE_Combined, 1);

	Device->CreateVertexDeclaration(VE_Combined, &VD_Combined);

	#ifdef TIGER
		this->LoadXFile("tiger.x");
	#else
		this->LoadXFile("archmage.x");		// http://www.sharecg.com/v/17943/3d-model/Archmage:-low-poly-RTS-game-character
		//this->LoadXFile("tiny.x");
	#endif

	// Create a VB for the instancing data
	Device->CreateVertexBuffer(MAX_INSTANCES * sizeof(INSTANCE), 0, 0, D3DPOOL_MANAGED, &CrowdInstances, 0);

	this->open();

	this->AddInstances(DEFAULT_INSTANCES);

	Mesh->GetVertexBuffer(&this->GeometryPacket);
	Mesh->GetIndexBuffer(&this->IndexPacket);
}

void OVCCrowd::ReadyBatch(D3DXMATRIX &VP)
{
	stringstream ss;
	bool render;

	int p = 0;				// Used as a test for controlling which are uploaded for batching.  

	INSTANCE* pInstances;
	D3DXMATRIX World;

	// Used for filtering through the members if required.  
	CrowdInstances->Lock(0, NULL, (void**)&pInstances, 0);
	for (MemberList = flock.begin() ; MemberList != flock.end() ; MemberList++)
	{
		World	= (*MemberList)->GetWorld();
		render	= true;

		if (this->UseFrustum)
		{
			for (int i = 0 ; i < 6 ; i++)
			{
				if (D3DXPlaneDotCoord(&Plane[i], &D3DXVECTOR3(World._41, World._42, World._43)) < -RADIUS)
				{
					render = false;

					break;
				}
			}
		}

		if (render)
		{
			pInstances->World = World;
			pInstances++;
			p++;
		}
	}
	CrowdInstances->Unlock();

	ss << "Crowd Size (Visible): " << p;

	LabelInstances.clear();
	LabelInstances = ss.str();

	this->batch_size = p;
}

void OVCCrowd::Update(D3DXMATRIX &VP, float dt)
{
	this->batch_size = 0;

	if (UseBoids)
		this->update(dt);

	if (this->UseInstancing)
		this->ReadyBatch(VP);
}

void OVCCrowd::Render(D3DXMATRIX &VP, float TimeDelta)
{
	if (this->UseFrustum)
	{
		D3DXVECTOR4 col0(VP(0, 0), VP(1, 0), VP(2, 0), VP(3, 0));
		D3DXVECTOR4 col1(VP(0, 1), VP(1, 1), VP(2, 1), VP(3, 1));
		D3DXVECTOR4 col2(VP(0, 2), VP(1, 2), VP(2, 2), VP(3, 2));
		D3DXVECTOR4 col3(VP(0, 3), VP(1, 2), VP(2, 3), VP(3, 3));

		Plane[0] = D3DXPLANE(col2);
		Plane[1] = D3DXPLANE(col3 - col2);
		Plane[2] = D3DXPLANE(col3 + col0);
		Plane[3] = D3DXPLANE(col3 - col0);
		Plane[4] = D3DXPLANE(col3 - col1);
		Plane[5] = D3DXPLANE(col3 + col1);

		for (int i = 0 ; i < 6 ; i++)
			D3DXPlaneNormalize(&Plane[i], &Plane[i]);
	}

	this->Update(VP, TimeDelta * 0.5f);

	if (this->UseInstancing)
	{
		if (this->batch_size != 0)
			this->RenderInstancing(VP);
	}
	else
		this->RenderRegular(VP);

	Font->DrawText(NULL, LabelInstancing.c_str(),	LabelInstancing.length(),	&this->TextInstancing,	DT_LEFT | DT_TOP, D3DCOLOR_ARGB(255, 255, 255, 255));
	Font->DrawText(NULL, LabelBoids.c_str(),		LabelBoids.length(),		&this->TextBoids,		DT_LEFT | DT_TOP, D3DCOLOR_ARGB(255, 255, 255, 255));
	Font->DrawText(NULL, LabelFrustum.c_str(),		LabelFrustum.length(),		&this->TextFrustum,		DT_LEFT | DT_TOP, D3DCOLOR_ARGB(255, 255, 255, 255));
//	Font->DrawText(NULL, LabelAnimation.c_str(),	LabelAnimation.length(),	&this->TextAnimation,	DT_LEFT | DT_TOP, D3DCOLOR_ARGB(255, 255, 255, 255));

	Font->DrawText(NULL, LabelInstances.c_str(),	LabelInstances.length(),	&this->TextInstances,	DT_LEFT | DT_TOP, D3DCOLOR_ARGB(255, 255, 255, 255));
}

void OVCCrowd::RenderRegular(D3DXMATRIX &VP)
{
	stringstream ss;
	D3DXMATRIX World, WIT;
	UINT uPasses;
	bool render;
	UINT p = 0;

	HLSL->SetTechnique("Render");

	Device->SetVertexDeclaration(VD_Geometry);

	for (MemberList = flock.begin() ; MemberList != flock.end() ; MemberList++)
	{
		World	= (*MemberList)->GetWorld();
		render	= true;

		if (this->UseFrustum)
		{
			for (int i = 0 ; i < 6 ; i++)
			{
				if (D3DXPlaneDotCoord(&Plane[i], &D3DXVECTOR3(World._41, World._42, World._43)) < -RADIUS)
				{
					render = false;

					break;
				}
			}
		}

		if (render)
		{
			HLSL->Begin(&uPasses, 0);
			for (UINT uPass = 0 ; uPass < uPasses ; ++uPass)
			{
				HLSL->BeginPass(uPass);

				for (DWORD i = 0 ; i < this->numMaterials ; i++)    // loop through each subset
				{
					HLSL->SetMatrix("mWorld", &(*MemberList)->GetWorld());
					HLSL->SetVector("Material.Diffuse", (D3DXVECTOR4*)&Diffuse[i]);

					if (Texture[i])    // if the subset has a texture (if texture is not NULL)
						HLSL->SetTexture("tTexture", Texture[i]);

					HLSL->CommitChanges();
					Mesh->DrawSubset(i);					// draw the subset
				}

				HLSL->EndPass();
			}
			HLSL->End();

			p++;
		}
	}

	ss << "Crowd Size (Visible): " << p;

	LabelInstances.clear();
	LabelInstances = ss.str();
}

void OVCCrowd::RenderInstancing(D3DXMATRIX &VP)
{
	UINT uPasses;

	Device->SetVertexDeclaration(VD_Combined);

	Device->SetStreamSource(0, this->GeometryPacket, 0, this->VDG_Size);
	Device->SetStreamSourceFreq(0, D3DSTREAMSOURCE_INDEXEDDATA | batch_size);

	Device->SetStreamSource(1, this->CrowdInstances, 0, this->VDI_Size);
	Device->SetStreamSourceFreq(1, D3DSTREAMSOURCE_INSTANCEDATA | 1ul);

	Device->SetIndices(this->IndexPacket);

	HLSL->SetTechnique("RenderCrowd");

	HLSL->Begin(&uPasses, 0);
	HLSL->BeginPass(0);
		HLSL->SetVector("Material.Diffuse", (D3DXVECTOR4*)&Diffuse[0]);
		HLSL->SetTexture("tTexture", Texture[0]);
		HLSL->CommitChanges();

		Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, Mesh->GetNumVertices(), 0, Mesh->GetNumFaces());
	HLSL->EndPass();
	HLSL->End();

	Device->SetStreamSourceFreq(0, 1);
	Device->SetStreamSourceFreq(1, 1);
}

void OVCCrowd::AddInstances(int n)
{
	std::stringstream ss;

	// If the number asked takes it over the instance limit, only add as far as the limit.  Otherwise, add as normal.  
	if (flock.size() + n > MAX_INSTANCES)
	{
		for (int i = flock.size() ; i < MAX_INSTANCES ; i++)
			flock.push_back(new Presence(*pd, this->insideBigBox));
	}
	else
	{
		for (int i = 0 ; i < n ; i++)
			flock.push_back(new Presence(*pd, this->insideBigBox));
	}

	ss << "Crowd Size (Visible): " << flock.size();

	LabelInstances.clear();
	LabelInstances = ss.str();
}

void OVCCrowd::RemoveInstances(int n)
{
	std::stringstream ss;

	if (flock.size() == 0)
		return;

	if ((flock.size() - n) <= 0)
		flock.clear();
	else
	{
		for (int i = 0 ; i < n ; i++)
			flock.pop_back();
	}

	ss << "Crowd Size (Visible): " << flock.size();

	LabelInstances.clear();
	LabelInstances = ss.str();
}

void OVCCrowd::SwitchInstancing()
{
	std::stringstream ss;

	if (this->UseInstancing)
	{
		this->UseInstancing = false;
		ss << "Geometric Instancing: Disabled";
	}
	else
	{
		this->UseInstancing = true;
		ss << "Geometric Instancing: Enabled";
	}

	LabelInstancing.clear();
	LabelInstancing = ss.str();
}

void OVCCrowd::SwitchBoids()
{
	std::stringstream ss;

	if (this->UseBoids)
	{
		this->UseBoids = false;
		ss << "Boids Animation: Disabled";
	}
	else
	{
		this->UseBoids = true;
		ss << "Boids Animation: Enabled";
	}

	LabelBoids.clear();
	LabelBoids = ss.str();
}

void OVCCrowd::SwitchFrustum()
{
	std::stringstream ss;

	if (this->UseFrustum)
	{
		this->UseFrustum = false;
		ss << "Frustum Culling: Disabled";
	}
	else
	{
		this->UseFrustum = true;
		ss << "Frustum Culling: Enabled";
	}

	LabelFrustum.clear();
	LabelFrustum = ss.str();
}

/*	The following function is removed due to issues with skeletal animation.  

void OVCCrowd::SwitchAnimation()
{
	std::stringstream ss;

	if (this->UseAnimation)
	{
		this->UseAnimation = false;
		ss << "Skeletal Animation: Disabled";
	}
	else
	{
		this->UseAnimation = true;
		ss << "Skeletal Animation: Enabled";
	}

	LabelAnimation.clear();
	LabelAnimation = ss.str();
}
*/