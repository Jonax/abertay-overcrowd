//--------------------------------------------------------------------------------------
// File: BasicHLSL.fx
//
// The effect file for the BasicHLSL sample.  
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

uniform float4x4 mWorld;
uniform float4x4 mVP;

uniform extern struct
{
	float3 Position;
	float4 Diffuse;
} Light;

uniform extern struct
{
	float4 Diffuse;
} Material;

uniform extern texture tTexture;

//--------------------------------------------------------------------------------------
// Texture samplers
//--------------------------------------------------------------------------------------
sampler sTextureSampler = 
sampler_state
{
	Texture		= <tTexture>;
	MipFilter	= LINEAR;
	MinFilter	= LINEAR;
	MagFilter	= LINEAR;
};

//--------------------------------------------------------------------------------------
// Vertex shader output structure
//--------------------------------------------------------------------------------------
struct VS_OUTPUT_T
{
	float4 Position	: POSITION;		// vertex position 
	float4 Normal	: NORMAL;
	float2 UV		: TEXCOORD0;
	float4 ExtraPos : TEXCOORD1;
};

struct VS_OUTPUT_C
{
	float4 Position	: POSITION;		// vertex position 
	float4 Normal	: NORMAL;
	float2 UV		: TEXCOORD0;
};

struct VS_INPUT
{
	float4 Position	: POSITION;		// vertex position 
	float4 Normal	: NORMAL;
	float2 UV		: TEXCOORD0;
};

struct VS_INSTANCE
{
	float4 Position	: POSITION;		// vertex position 
	float4 Normal	: NORMAL;
	float2 UV		: TEXCOORD0;
	
	float4 model_matrix0 : TEXCOORD1;
	float4 model_matrix1 : TEXCOORD2;
	float4 model_matrix2 : TEXCOORD3;
	float4 model_matrix3 : TEXCOORD4;
};

//--------------------------------------------------------------------------------------
// Pixel shader output structure
//--------------------------------------------------------------------------------------
struct PS_OUTPUT
{
    float4 RGBColor : COLOR0;  // Pixel color    
};

//--------------------------------------------------------------------------------------
// This shader computes standard transform and lighting
//--------------------------------------------------------------------------------------
VS_OUTPUT_T RenderUI_VS(VS_INPUT Input)
{
	VS_OUTPUT_T Output;

	float4x4 MVP	= mul(mWorld, mVP);
    
	Output.Position	= mul(Input.Position, MVP);
	Output.ExtraPos = Output.Position;
	
	float3 normalW = mul(Input.Normal, MVP).xyz;
	Output.Normal = float4(normalize(normalW), 1.0f);
    
//	float3 LightDirection = Input.Position.xyz - Light.Position;
//	LightDirection = normalize(LightDirection);
    
//	float s = max(dot(LightDirection, normalW), 0.0f);
//	Output.Diffuse.rgb = s * (Material.Diffuse * Light.Diffuse).rgb;
//  Output.Diffuse.a = Light.Diffuse;
	
	Output.UV		= Input.UV;
    
	return Output;
}

VS_OUTPUT_C RenderBatch_VS(VS_INSTANCE Input)
{
	VS_OUTPUT_C Output;
	
	float4x4 modelMatrix =
	{
		Input.model_matrix0,
		Input.model_matrix1,
		Input.model_matrix2,
		Input.model_matrix3
	};
	
	float4x4 MVP	= mul(modelMatrix, mVP);
    
	Output.Position	= mul(Input.Position, MVP);
	Output.Normal	= mul(Input.Normal, MVP);
	Output.UV		= Input.UV;
    
	return Output;
}

//--------------------------------------------------------------------------------------
// This shader outputs the pixel's color by modulating the texture's
//       color with diffuse material color
//--------------------------------------------------------------------------------------
PS_OUTPUT RenderUI_PS(VS_OUTPUT_T In) 
{ 
	PS_OUTPUT Output;

	//Output.RGBColor = tex2D(sTextureSampler, In.UV) * In.Diffuse;
	
	float3 Normal = normalize(In.Normal.xyz);
	
	float3 LightDirection = normalize(In.ExtraPos - Light.Position);
	
	float s = max(dot(LightDirection, Normal), 0.0f);
	
	float3 Diffuse = s * (Material.Diffuse * Light.Diffuse).rgb;
	
	Output.RGBColor = tex2D(sTextureSampler, In.UV) * float4(Diffuse, Material.Diffuse.a);

	return Output;
}

PS_OUTPUT RenderBatch_PS(VS_OUTPUT_C In) 
{ 
	PS_OUTPUT Output;

	Output.RGBColor = tex2D(sTextureSampler, In.UV) * Material.Diffuse;

	return Output;
}

//--------------------------------------------------------------------------------------
// Renders scene to render target
//--------------------------------------------------------------------------------------
technique Render
{
    pass P0
    {          
        VertexShader = compile vs_3_0 RenderUI_VS();
        PixelShader  = compile ps_3_0 RenderUI_PS();
    }
}

technique RenderCrowd
{
    pass P0
    {          
        VertexShader = compile vs_3_0 RenderBatch_VS();
        PixelShader  = compile ps_3_0 RenderBatch_PS();
    }
}