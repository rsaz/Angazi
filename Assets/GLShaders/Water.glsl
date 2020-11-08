#shader VS
#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aNormal;
layout (location = 2) in vec4 aTangent;
layout (location = 3) in vec2 aTexCoord;

out vec3 outWorldNormal;
out vec3 outWorldTangent;
out vec3 outDirToLight;
out vec3 outDirToView;
out vec2 outTexCoord;
out vec4 outPositionNDC;
out vec4 outPositionScreen;

out gl_PerVertex
{
	vec4 gl_Position;
};

layout(std140, binding = 0) uniform TransformBuffer
{
	mat4 World;
	mat4 WVP;
	vec3 ViewPosition;
};

layout(std140, binding = 1) uniform LightBuffer 
{
	vec3 LightDirection;
	vec4 LightAmbient;
	vec4 LightDiffuse;
	vec4 LightSpecular;
};

layout(std140, binding = 2) uniform MaterialBuffer 
{
	vec3 padding;
	float MaterialPower;
	vec4 MaterialAmbient;
	vec4 MaterialDiffuse;
	vec4 MaterialSpecular;
};

layout(std140, binding = 3) uniform SettingsBuffer 
{
	float specularMapWeight;
	float bumpMapWeight;
	float normalMapWeight;
	float brightness;
	float movement;
	float movementSpeed;
	float reflectivePower;
};

layout(binding = 0) uniform sampler2D diffuseMap;
layout(binding = 1) uniform sampler2D specularMap;
layout(binding = 2) uniform sampler2D displacementMap;
layout(binding = 3) uniform sampler2D normalMap;
layout(binding = 4) uniform sampler2D refractionMap;
layout(binding = 5) uniform sampler2D reflectionMap;
layout(binding = 6) uniform sampler2D depthMap;

void main()
{
	float xCoord = aTexCoord.x + movement;
	if (xCoord > 1.0f)
		xCoord -= 1.0f;

	float displacement =  texture(displacementMap, vec2(xCoord, aTexCoord.y), 1).r * 2.0f - 1.0f;
	vec3 localPosition = aPos + (aNormal.xyz * displacement * bumpMapWeight);
	vec3 worldPosition = (vec4(localPosition, 1.0f)* World).xyz;
	vec3 worldNormal = aNormal.xyz* mat3x3(World);
	vec3 worldTangent = aTangent.xyz* mat3x3(World);

	vec4 outPos = vec4(localPosition, 1.0f) * WVP;
	outPositionScreen= vec4(aPos, 1.0f) * WVP;

	outWorldNormal = worldNormal;
	outWorldTangent = worldTangent;
	outDirToLight = -LightDirection;
	outDirToView = normalize(ViewPosition - worldPosition);
	outTexCoord = aTexCoord;
	gl_Position = outPos; 
}

#shader PS
#version 450 core

in vec3 outWorldNormal;
in vec3 outWorldTangent;
in vec3 outDirToLight;
in vec3 outDirToView;
in vec2 outTexCoord;
in vec4 outPositionNDC;
in vec4 outPositionScreen;

out vec4 FragColor;

layout(std140, binding = 0) uniform TransformBuffer
{
	mat4 World;
	mat4 WVP;
	vec3 ViewPosition;
};

layout(std140, binding = 1) uniform LightBuffer 
{
	vec3 LightDirection;
	vec4 LightAmbient;
	vec4 LightDiffuse;
	vec4 LightSpecular;
};

layout(std140, binding = 2) uniform MaterialBuffer 
{
	vec3 padding;
	float MaterialPower;
	vec4 MaterialAmbient;
	vec4 MaterialDiffuse;
	vec4 MaterialSpecular;
};

layout(std140, binding = 3) uniform SettingsBuffer 
{
	float specularMapWeight;
	float bumpMapWeight;
	float normalMapWeight;
	float brightness;
	float movement;
	float movementSpeed;
	float reflectivePower;
};

layout(binding = 0) uniform sampler2D diffuseMap;
layout(binding = 1) uniform sampler2D specularMap;
layout(binding = 2) uniform sampler2D displacementMap;
layout(binding = 3) uniform sampler2D normalMap;
layout(binding = 4) uniform sampler2D refractionMap;
layout(binding = 5) uniform sampler2D reflectionMap;
layout(binding = 6) uniform sampler2D depthMap;

void main()
{
	vec3 worldNormal = normalize(outWorldNormal);
	vec3 worldTangent = normalize(outWorldTangent);
	vec3 worldBinormal = normalize(cross(worldNormal,worldTangent));
	vec3 dirToLight = normalize(outDirToLight);
	vec3 dirToView = normalize(outDirToView);

	vec2 ndc = (outPositionScreen.xy / outPositionScreen.w) *0.5f +0.5f;
	vec2 UVReflect = ndc;
	vec2 UVRefraction = ndc;
	UVReflect.y = 1.0f - UVReflect.y;

	// Water Depth
	float near = 0.1f;
	float far = 10000.0f;
	float depth = texture(depthMap,UVRefraction).r;
	float floorDistance = 2.0 * near * far / (far + near - (2.0 * depth - 1.0) * (far - near));
	depth = gl_FragCoord.z;
	float waterDistance = 2.0 * near * far / (far + near - (2.0 * depth - 1.0) * (far - near));
	float waterDepth = floorDistance - waterDistance;

	float xCoord = outTexCoord.x + movement;
	if (xCoord > 1.0f)
		xCoord -= 1.0f;
	float yCoord = outTexCoord.y + movement;
	if (yCoord > 1.0f)
		yCoord -= 1.0f;

	vec2 displacementCoords = (texture(displacementMap, vec2(xCoord, outTexCoord.y), 1).rg * 2.0f - 1.0f) * movementSpeed;
	displacementCoords += (texture(displacementMap, vec2(xCoord, yCoord), 1).rg * 2.0f - 1.0f) * movementSpeed;
	displacementCoords *= clamp(waterDepth,0.0f,1.0f);

	vec2 displacementCoords2 = (texture(displacementMap, vec2(xCoord, outTexCoord.y), 1).rg * 2.0f - 1.0f) * normalMapWeight;
	displacementCoords2 += (texture(displacementMap, vec2(xCoord, yCoord), 1).rg * 2.0f - 1.0f) * normalMapWeight;
	displacementCoords2 *= clamp(waterDepth,0.0f,1.0f);

	vec3 normal = worldNormal;
	if (normalMapWeight != 0.0f)
	{
		mat3x3 TBNW = { worldTangent, worldBinormal, worldNormal};
		vec4 normalColor = texture(normalMap, displacementCoords2 * 0.5f +0.09);
		vec3 normalSampled = (normalColor.xyz * 2.0f) - 1.0f;
		normal = TBNW *normalSampled ;
	}

	// ambient
	vec4 ambient = LightAmbient * MaterialAmbient;

	// diffuse
	float diffuseIntensity = clamp(dot(dirToLight, normal),0.0,1.0);
	vec4 diffuse = diffuseIntensity * LightDiffuse* MaterialDiffuse;

	// specular
	vec3 halfAngle = normalize(dirToLight + dirToView);
	float specularBase = clamp(dot(halfAngle, normal), 0.0,1.0);
	float specularIntensity = pow(specularBase, MaterialPower);
	vec4 specular = specularIntensity * LightSpecular * MaterialSpecular;

	UVRefraction = clamp(UVRefraction + displacementCoords,0.0,1.0);
	UVReflect = clamp(UVReflect + displacementCoords,0.0,1.0);

	vec4 texColor = texture(diffuseMap, outTexCoord + displacementCoords);
	vec4 texColorRefraction = texture(refractionMap, UVRefraction);
	vec4 texColorReflection = texture(reflectionMap, UVReflect);

	float specularFactor = texture(specularMap, outTexCoord).r * clamp(waterDepth,0.0f,1.0f);

	float reflectionFactor = clamp(dot(dirToView, worldNormal),0.0,1.0);
	reflectionFactor = pow(reflectionFactor,reflectivePower);
	
	vec4 waterColor = mix(texColorReflection, texColorRefraction, reflectionFactor);
	texColor = mix(texColor, waterColor, 0.9f);

	vec4 color = (ambient + diffuse) * texColor * brightness + specular * (specularMapWeight != 0.0f ? specularFactor : 1.0f);
	color.a = clamp(waterDepth,0.0f,1.0f);

	FragColor = color;
}