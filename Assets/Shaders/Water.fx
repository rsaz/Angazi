//Description: Simple shader that does applies transformation and lighting

cbuffer TransformBuffer : register(b0)
{
	matrix World;
	matrix WVP; // world view projection - all three matricies combined . local - camera 
	float3 ViewPosition;
}

cbuffer LightBuffer : register(b1)
{
	float3 LightDirection;
	float4 LightAmbient;
	float4 LightDiffuse;
	float4 LightSpecular;
}

cbuffer MaterialBuffer : register(b2)
{
	float3 padding;
	float  MaterialPower; //shininess
	float4 MaterialAmbient;
	float4 MaterialDiffuse;
	float4 MaterialSpecular;
}

cbuffer SettingsBuffer : register(b3)
{
	float specularMapWeight : packoffset(c0.x);
	float bumpMapWeight : packoffset(c0.y);
	float normalMapWeight : packoffset(c0.z);
	float brightness : packoffset(c0.w);
	float movement : packoffset(c1.x);
	float movementSpeed : packoffset(c1.y);
	float reflectivePower : packoffset(c1.z);
}

Texture2D diffuseMap : register(t0);
Texture2D specularMap : register(t1);
Texture2D displacementMap : register(t2);
Texture2D normalMap : register(t3);
Texture2D refractionMap : register(t4);
Texture2D reflectionMap : register(t5);
Texture2D depthMap : register(t6);

SamplerState textureSampler : register(s0);

struct VS_INPUT
{
	float3 position : POSITION;
	float4 normal : NORMAL;
	float4 tangent : TANGENT;
	float2 texCoord : TEXCOORD;
};

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float3 worldNormal : NORMAL;
	float3 worldTangent : TEXCOORD0;
	float3 dirToLight : TEXCOORD1;
	float3 dirToView : TEXCOORD2;
	float2 texCoord : TEXCOORD3;
	float4 positionNDC : TEXCOORD4;
	float4 positionScreen : TEXCOORD5;
};

VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output;

	float xCoord = input.texCoord.x + movement;
	if (xCoord > 1.0f)
		xCoord -= 1.0f;
	float displacement = displacementMap.SampleLevel(textureSampler, float2(xCoord, input.texCoord.y), 1).rg * 2.0f - 1.0f;
	float3 localPosition = input.position + (input.normal * displacement * bumpMapWeight);
	float3 worldPosition = mul(float4(localPosition, 1.0f), World).xyz;
	float3 worldNormal = mul(input.normal, (float3x3) World);
	float3 worldTangent = mul(input.tangent, (float3x3) World);
	
	output.position = mul(float4(localPosition, 1.0f), WVP);
	output.positionScreen = mul(float4(input.position, 1.0f), WVP);
	
	output.worldNormal = worldNormal;
	output.worldTangent = worldTangent;
	output.dirToLight = -LightDirection;
	output.dirToView = normalize(ViewPosition - worldPosition);
	output.texCoord = input.texCoord;

	return output;
}

//   |
//   v
// Rasterizer
//   |
//   v

float4 PS(VS_OUTPUT input) : SV_Target
{
	float3 worldNormal = normalize(input.worldNormal);
	float3 worldTangent = normalize(input.worldTangent);
	float3 worldBinormal = normalize(cross(worldNormal, worldTangent));
	float3 dirToLight = normalize(input.dirToLight);
	float3 dirToView = normalize(input.dirToView);
	
	float2 ndc = input.positionScreen.xy / input.positionScreen.w;
	float2 UVRefraction = (ndc + 1.0f) * 0.5f;
	float2 UVReflect = UVRefraction;
	UVRefraction.y = 1.0f - UVRefraction.y;
	
	// Water Depth
	float near = 0.1f;
	float far = 10000.0f;
	float depth = depthMap.Sample(textureSampler, UVRefraction).r;
	//float depth = 1.0f - input.positionScreen.z / input.positionScreen.w;
	float floorDistance = 2.0 * near * far / (far + near - (2.0 * depth - 1.0) * (far - near));
	depth = input.position.z;
	float waterDistance = 2.0 * near * far / (far + near - (2.0 * depth - 1.0) * (far - near));
	float waterDepth = floorDistance - waterDistance;
	
	float xCoord = input.texCoord.x + movement;
	if (xCoord > 1.0f)
		xCoord -= 1.0f;
	float yCoord = input.texCoord.y + movement;
	if (yCoord > 1.0f)
		yCoord -= 1.0f;

	float2 displacementCoords = (displacementMap.SampleLevel(textureSampler, float2(xCoord, input.texCoord.y), 1).rg * 2.0f - 1.0f) * movementSpeed;
	displacementCoords += (displacementMap.SampleLevel(textureSampler, float2(xCoord, yCoord), 1).rg * 2.0f - 1.0f) * movementSpeed;
	displacementCoords *= saturate(waterDepth);

	float2 displacementCoords2 = (displacementMap.SampleLevel(textureSampler, float2(xCoord, input.texCoord.y), 1).rg * 2.0f - 1.0f) * normalMapWeight;
	displacementCoords2 += (displacementMap.SampleLevel(textureSampler, float2(xCoord, yCoord), 1).rg * 2.0f - 1.0f) * normalMapWeight;
	displacementCoords2 *= saturate(waterDepth);
	
	float3 normal = worldNormal;
	if (normalMapWeight != 0.0f)
	{
		float3x3 TBNW = { worldTangent, worldBinormal, worldNormal};
		float4 normalColor = normalMap.Sample(textureSampler, displacementCoords2* 0.02 +0.09);
		float3 normalSampled = (normalColor.xyz * 2.0f) - 1.0f;
		normal = mul(normalSampled, TBNW);
	}

	float4 ambient = LightAmbient * MaterialAmbient;
	//if (aoWeight == 1.0f)
	//	ambient = aoMap.Sample(textureSampler, input.texCoord);

	float diffuseIntensity = saturate(dot(dirToLight, normal));
	float4 diffuse = diffuseIntensity * LightDiffuse * MaterialDiffuse;

	float3 halfAngle = normalize(dirToLight + dirToView);
	float specularBase = saturate(dot(halfAngle, normal));
	float specularIntensity = pow(specularBase, MaterialPower);
	float4 specular = specularIntensity * LightSpecular * MaterialSpecular * saturate(waterDepth);;

	UVRefraction = saturate(UVRefraction + displacementCoords);
	UVReflect = saturate(UVReflect+ displacementCoords);

	float4 texColor = diffuseMap.Sample(textureSampler, input.texCoord+ displacementCoords);
	float4 texColorRefraction = refractionMap.Sample(textureSampler, UVRefraction);
	float4 texColorReflection = reflectionMap.Sample(textureSampler, UVReflect);

	float specularFactor = specularMap.Sample(textureSampler, input.texCoord).r;
	
	float reflectionFactor = saturate(dot(dirToView, worldNormal));
	reflectionFactor = pow(reflectionFactor, reflectivePower);

	float4 waterColor = lerp(texColorReflection, texColorRefraction, reflectionFactor);
	texColor = lerp(texColor, waterColor, 0.85f);

	float4 color = (ambient + diffuse) * texColor * brightness + specular * (specularMapWeight != 0.0f ? specularFactor : 1.0f);
	color.a = saturate(waterDepth);
	//color = float4(color.a, color.a, color.a, color.a);

	return color;
}