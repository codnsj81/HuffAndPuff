struct MATERIAL
{
	float4					m_cAmbient;
	float4					m_cDiffuse;
	float4					m_cSpecular; //a = power
	float4					m_cEmissive;
};

cbuffer cbCameraInfo : register(b1)
{
	matrix					gmtxView : packoffset(c0);
	matrix					gmtxProjection : packoffset(c4);
	float3					gvCameraPosition : packoffset(c8);
};

cbuffer cbGameObjectInfo : register(b2)
{
	matrix					gmtxGameObject : packoffset(c0);
	MATERIAL				gMaterial : packoffset(c4);
	uint					gnTexturesMask : packoffset(c8);
};

cbuffer Texbuffer : register(b9)
{
	float2 translation = 0;
};

#include "Light.hlsl"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//#define _WITH_VERTEX_LIGHTING

#define MATERIAL_ALBEDO_MAP			0x01
#define MATERIAL_SPECULAR_MAP		0x02
#define MATERIAL_NORMAL_MAP			0x04
#define MATERIAL_METALLIC_MAP		0x08
#define MATERIAL_EMISSION_MAP		0x10
#define MATERIAL_DETAIL_ALBEDO_MAP	0x20
#define MATERIAL_DETAIL_NORMAL_MAP	0x40

Texture2D gtxtAlbedoTexture : register(t6);
Texture2D gtxtSpecularTexture : register(t7);
Texture2D gtxtNormalTexture : register(t8);
Texture2D gtxtMetallicTexture : register(t9);
Texture2D gtxtEmissionTexture : register(t10);
Texture2D gtxtDetailAlbedoTexture : register(t11);
Texture2D gtxtDetailNormalTexture : register(t12);

SamplerState gssWrap : register(s0);

struct VS_STANDARD_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
	uint	colornum : COLORNUM;
};

struct VS_STANDARD_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float3 tangentW : TANGENT;
	float3 bitangentW : BITANGENT;
	float2 uv : TEXCOORD;
	uint	colornum : COLORNUM;
};

VS_STANDARD_OUTPUT VSStandard(VS_STANDARD_INPUT input)
{
	VS_STANDARD_OUTPUT output;

	output.positionW = mul(float4(input.position, 1.0f), gmtxGameObject).xyz;
	output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
	output.tangentW = mul(input.tangent, (float3x3)gmtxGameObject);
	output.bitangentW = mul(input.bitangent, (float3x3)gmtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;
	output.colornum = input.colornum;

	return(output);
}

float4 PSStandard(VS_STANDARD_OUTPUT input) : SV_TARGET
{
	float4 cAlbedoColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	if (gnTexturesMask & MATERIAL_ALBEDO_MAP) cAlbedoColor = gtxtAlbedoTexture.Sample(gssWrap, input.uv);
	//else cAlbedoColor = float4(1.0f, 0.0f, 0.0f, 0.0f);
	float4 cSpecularColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	if (gnTexturesMask & MATERIAL_SPECULAR_MAP) cSpecularColor = gtxtSpecularTexture.Sample(gssWrap, input.uv);
	float4 cNormalColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	if (gnTexturesMask & MATERIAL_NORMAL_MAP) cNormalColor = gtxtNormalTexture.Sample(gssWrap, input.uv);
	float4 cMetallicColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	if (gnTexturesMask & MATERIAL_METALLIC_MAP) cMetallicColor = gtxtMetallicTexture.Sample(gssWrap, input.uv);
	float4 cEmissionColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	if (gnTexturesMask & MATERIAL_EMISSION_MAP) cEmissionColor = gtxtEmissionTexture.Sample(gssWrap, input.uv);

	float3 normalW;
	float4 cColor = cAlbedoColor + cSpecularColor + cMetallicColor + cEmissionColor;
	if (gnTexturesMask & MATERIAL_NORMAL_MAP)
	{
		float3x3 TBN = float3x3(normalize(input.tangentW), normalize(input.bitangentW), normalize(input.normalW));
		float3 vNormal = normalize(cNormalColor.rgb * 2.0f - 1.0f); //[0, 1] → [-1, 1]
		normalW = normalize(mul(vNormal, TBN));
	}
	else
	{
		normalW = normalize(input.normalW);
	}
	float4 cIllumination = Lighting(input.positionW, normalW);
	cColor = lerp(cColor, cIllumination, 0.4f);
	if (input.colornum == 1) cColor += float4(0.3f, 0.f, 0.f, 0.f);

	return(cColor);
}


float4 PSEffect(VS_STANDARD_OUTPUT input) : SV_TARGET
{

	float4 cColor;
	if (gnTexturesMask & MATERIAL_ALBEDO_MAP) cColor = gtxtAlbedoTexture.Sample(gssWrap, input.uv);
	cColor.a -= 0.3f;
	return(cColor);
}

float4 PSGrass(VS_STANDARD_OUTPUT input) : SV_TARGET
{
	float4 cAlbedoColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	if (gnTexturesMask & MATERIAL_ALBEDO_MAP) cAlbedoColor = gtxtAlbedoTexture.Sample(gssWrap, input.uv);
	cAlbedoColor = cAlbedoColor - float4(0.2f, 0.2f, 0.2f, 0.f);
		
	return cAlbedoColor;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#define MAX_VERTEX_INFLUENCES			4
#define SKINNED_ANIMATION_BONES			128

cbuffer cbBoneOffsets : register(b7)
{
	float4x4 gpmtxBoneOffsets[SKINNED_ANIMATION_BONES]; // offset 변환행렬 그 bone 월드 변환행렬
};

cbuffer cbBoneTransforms : register(b8)
{
	float4x4 gpmtxBoneTransforms[SKINNED_ANIMATION_BONES];
};

struct VS_SKINNED_STANDARD_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
	uint4 indices : BONEINDEX;
	float4 weights : BONEWEIGHT;
	uint colornum : COLORNUM;
};
// 애니메이션 정보를 받고 모든 뼈의 움직임을 바꾼다
// skin mesh 를 그리기 위해서 필요한 두개의 행려 (  bone transfrom, offset 변환 ) 정점 하나를 넘겨 받으면

VS_STANDARD_OUTPUT VSSkinnedAnimationStandard(VS_SKINNED_STANDARD_INPUT input)
{
	VS_STANDARD_OUTPUT output;

	output.positionW = float3(0.0f, 0.0f, 0.0f);
	output.normalW = float3(0.0f, 0.0f, 0.0f);
	output.tangentW = float3(0.0f, 0.0f, 0.0f);
	output.bitangentW = float3(0.0f, 0.0f, 0.0f);
	output.colornum = input.colornum;
	matrix mtxVertexToBoneWorld;
	for (int i = 0; i < MAX_VERTEX_INFLUENCES; i++) // 매번 루트를 돌면서 
	{
		// offset  * bone transform 본의  인덱스에 해당하는 인덱스와 
		mtxVertexToBoneWorld = mul(gpmtxBoneOffsets[input.indices[i]], gpmtxBoneTransforms[input.indices[i]]); // 월드 변환행렬로 바꾸기
		output.positionW += input.weights[i] * mul(float4(input.position, 1.0f), mtxVertexToBoneWorld).xyz;
		output.normalW += input.weights[i] * mul(input.normal, (float3x3)mtxVertexToBoneWorld);
		output.tangentW += input.weights[i] * mul(input.tangent, (float3x3)mtxVertexToBoneWorld);
		output.bitangentW += input.weights[i] * mul(input.bitangent, (float3x3)mtxVertexToBoneWorld); // 다 더하면 결과
	}

	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
Texture2D gtxtTerrainDetailTexture : register(t2);
Texture2D gtxtTerrainDetailTexture2 : register(t3);

struct VS_TERRAIN_INPUT
{
	float3 position : POSITION;
	float4 color : COLOR;
	float2 uv0 : TEXCOORD0;
	float2 uv1 : TEXCOORD1;
};

struct VS_TERRAIN_OUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 uv0 : TEXCOORD0;
	float2 uv1 : TEXCOORD1;
	float4 shadowPosition : TEXCOORD3;
};

VS_TERRAIN_OUTPUT VSTerrain(VS_TERRAIN_INPUT input)
{
	VS_TERRAIN_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.color = input.color;
	output.uv0 = input.uv0;
	output.uv1 = input.uv1;

	return(output);
}

float4 PSTerrain(VS_TERRAIN_OUTPUT input) : SV_TARGET
{
	float4 cColor;
	float4 cBaseTexColor = gtxtSpecularTexture.Sample(gssWrap, input.uv0);
	float4 cDetailTexColor = gtxtMetallicTexture.Sample(gssWrap, input.uv1);
	cDetailTexColor = saturate(float4(0.f, 0.1f, 0.f, 1) + (cDetailTexColor * 0.6f));
	float4 cDetailTexColor2 = gtxtAlbedoTexture.Sample(gssWrap, input.uv1) ;
	cDetailTexColor2 = saturate(float4(0.1f, 0.09f, 0.f, 1) + (cDetailTexColor2 * 0.6f));
	//cDetailTexColor2 = saturate(float4(0.0f, 0.00f, 0.f, 1) + (cDetailTexColor2 * 0.6f));
	float4 cDetailTexColor3 = gtxtNormalTexture.Sample(gssWrap, input.uv1);

	cColor = input.color * (cDetailTexColor3 * cBaseTexColor.y + cDetailTexColor * cBaseTexColor.x
					+ cDetailTexColor2 * cBaseTexColor.z);
	return(cColor);
}
struct VS_SKYBOX_CUBEMAP_INPUT
{
	float3 position : POSITION;
};

struct VS_SKYBOX_CUBEMAP_OUTPUT
{
	float3	positionL : POSITION;
	float4	position : SV_POSITION;
};

VS_SKYBOX_CUBEMAP_OUTPUT VSSkyBox(VS_SKYBOX_CUBEMAP_INPUT input)
{
	VS_SKYBOX_CUBEMAP_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.positionL = input.position;

	return(output);
}

TextureCube gtxtSkyCubeTexture : register(t13);
SamplerState gssClamp : register(s1);

float4 PSSkyBox(VS_SKYBOX_CUBEMAP_OUTPUT input) : SV_TARGET
{
	float4 cColor = gtxtSkyCubeTexture.Sample(gssClamp, input.positionL);

	return(cColor);
}

////////////////////////////////////////////

cbuffer MatrixBuffer
{
	matrix worldMatrix; matrix viewMatrix; matrix projectionMatrix;
};
cbuffer ReflectionBuffer { matrix reflectionMatrix; };


struct VS_WATER_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
};

struct VS_WATER_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float3 tangentW : TANGENT;
	float3 bitangentW : BITANGENT;
	float2 uv : TEXCOORD;
};

VS_WATER_OUTPUT VSWater(VS_WATER_INPUT input)
{
	VS_WATER_OUTPUT output;

	output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxGameObject);
	output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
	output.tangentW = (float3)mul(float4(input.tangent, 1.0f), gmtxGameObject);
	output.bitangentW = (float3)mul(float4(input.bitangent, 1.0f), gmtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;
	output.uv.y += translation.x;

	return(output);
}

float4 PSWater(VS_WATER_OUTPUT input) : SV_TARGET
{

	float4 cColor = float4(0.0f, 1.f, 1.f, 0.2f);
	float3x3 TBN = float3x3(normalize(input.tangentW), normalize(input.bitangentW), normalize(input.normalW));
	float4 cNormal;
	cNormal = gtxtAlbedoTexture.Sample(gssClamp, input.uv);
	float3 vNormal = normalize(cNormal.rgb * 2.0f - 1.0f); //[0, 1] → [-1, 1]

	float4 cIllumination = Lighting(input.positionW, normalize(mul(vNormal, TBN)));

	return(lerp(cColor, cIllumination, 0.5f));
	//return cNormal;

}

//////////////////



struct VS_UI_INPUT
{
	float3 position : POSITION;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
};

struct VS_UI_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
};

VS_UI_OUTPUT VSUI(VS_UI_INPUT input)
{
	VS_UI_OUTPUT output;
	//output.positionW = mul(float4(input.position, 1.0f), gmtxGameObject).xyz;
	//output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	
	
	output.positionW = mul(float4(input.position, 1.0f), gmtxGameObject).xyz;
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);

	output.color = input.color;
	output.uv = input.uv;
	return(output);
}


VS_UI_OUTPUT VSFontUI(VS_UI_INPUT input)
{
	VS_UI_OUTPUT output;
	//output.positionW = mul(float4(input.position, 1.0f), gmtxGameObject).xyz;
	//output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);


	output.positionW = mul(float4(input.position, 1.0f), gmtxGameObject).xyz;
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);

	output.color = input.color;
	output.uv = input.uv;
	output.uv.y = input.uv.y + translation.y;
	output.uv.x = input.uv.x + translation.x;
	return(output);
}


float4 PSUI(VS_UI_OUTPUT input) : SV_TARGET
{
	VS_UI_OUTPUT output;
	output.color = input.color;
	if (gnTexturesMask & MATERIAL_ALBEDO_MAP) output.color = gtxtAlbedoTexture.Sample(gssWrap, input.uv);

	return(output.color);
}



///////////////////////////////////////////////////////////////

//원형 그림자

Texture2D gtxCircularShadow : register(t14);
SamplerState gssCircularShadow : register(s2);

struct VS_INPUT {
	float3 position : POSITION;
	float4 shadow : INSTANCEPOS;
};

struct VS_OUTPUT {
	float3 position : POSITION;
	float size : SIZE;
};
struct VS_SHADOW_INPUT {
	float3 position: POSITION;
	float2 texCoord : TEXCOORD;
	float4 shadow : INSTANCEPOS;
};

struct VS_SHADOW_OUTPUT {
	float4 position: SV_POSITION;
	float2 texCoord : TEXCOORD0;
};

VS_OUTPUT VSShadow(VS_INPUT input) {
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.size = 0.3f * (input.shadow.y - input.shadow.w);
	output.position = input.shadow.xyz - float3(0.0f, output.size * 2.5f, 0);
	return (output);
}

struct GS_OUTPUT {
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 texCoord : TEXCOORD0;
};

float4 PSShadow(GS_OUTPUT input) : SV_Target{
	return(gtxCircularShadow.Sample(gssCircularShadow, input.texCoord) * input.color);
}


[maxvertexcount(4)]
void GSShadow(point VS_OUTPUT input[1], inout TriangleStream<GS_OUTPUT> outStream) {
	GS_OUTPUT output;
	float3 vCorners[4];
	vCorners[0] = float3(input[0].position.x - input[0].size, input[0].position.y, input[0].position.z - input[0].size);
	vCorners[1] = float3(input[0].position.x - input[0].size, input[0].position.y, input[0].position.z + input[0].size);
	vCorners[2] = float3(input[0].position.x + input[0].size, input[0].position.y, input[0].position.z - input[0].size);
	vCorners[3] = float3(input[0].position.x + input[0].size, input[0].position.y, input[0].position.z + input[0].size);
	float2 vTexCoords[4] = { float2(0,1), float2(0,0), float2(1,1), float2(1,0) };
	float fColor = min(1, max(0, (100 + 155 * (90 - input[0].size)) / 255.0f));
	for (int i = 0; i < 4; i++)
	{
		output.position = mul(mul(float4(vCorners[i], 1), gmtxView), gmtxProjection);
		output.texCoord = vTexCoords[i];
		output.color = float4(fColor, fColor, fColor, 0);
		outStream.Append(output);
	}
}
VS_SHADOW_OUTPUT VSCircularShadow(VS_SHADOW_INPUT input)
{
	VS_SHADOW_OUTPUT output = (VS_SHADOW_OUTPUT)0;
	output.texCoord = input.texCoord;
	float3 position = input.position + input.shadow.xyz;
	position.y -= (input.shadow.y - input.shadow.w) * 0.7f;
	output.position = mul(mul(float4(position, 1.0f), gmtxView), gmtxProjection);
	return (output);
}

float4 PSCircularShadow(VS_SHADOW_OUTPUT input) : SV_Target{
	return (gtxCircularShadow.Sample(gssCircularShadow, input.texCoord));
}