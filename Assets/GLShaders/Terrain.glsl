#shader VS
#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aNormal;
layout (location = 2) in vec4 aTangent;
layout (location = 3) in vec2 aTexCoord;

out vec3 outWorldNormal;
out vec3 outWorldTangent;
out vec3 outDirToView;
out vec2 outTexCoord;
out float clip;

out gl_PerVertex
{
	vec4 gl_Position;
};

layout(std140, binding = 0) uniform TransformBuffer
{
	mat4 World;
	mat4 WVP;
	vec3 ViewPosition;
	vec3 LightDirection;
	vec4 LightAmbient;
	vec4 LightDiffuse;
	vec4 LightSpecular;
	vec4 clippingPlane;
};

layout(binding = 0) uniform sampler2D diffuseMap;

void main()
{
	vec4 outPos = vec4(aPos, 1.0f) * WVP;
	outWorldNormal = aNormal.xyz * mat3x3(World);
	outWorldTangent = aTangent.xyz * mat3x3(World);
	outDirToView = ViewPosition - (aPos.xyz * mat3x3(World));
	outTexCoord = aTexCoord * 10.0f;
	clip = dot(vec4(aPos, 1.0f) * World, clippingPlane);

	gl_Position = outPos;
}

#shader PS
#version 450 core

in vec3 outWorldNormal;
in vec3 outWorldTangent;
in vec3 outDirToView;
in vec2 outTexCoord;
in float clip;

out vec4 FragColor;

layout(std140, binding = 0) uniform TransformBuffer
{
	mat4 World;
	mat4 WVP;
	vec3 ViewPosition;
	vec3 LightDirection;
	vec4 LightAmbient;
	vec4 LightDiffuse;
	vec4 LightSpecular;
	vec4 clippingPlane;
};

layout(binding = 0) uniform sampler2D diffuseMap;

void main()
{
	vec3 normal = normalize(outWorldNormal);
	vec3 dirToLight = normalize(-LightDirection.xyz);
	vec3 dirToView = normalize(outDirToView);

	vec4 ambient = LightAmbient;
	vec4 diffuse = LightDiffuse * clamp(dot(normal,dirToLight),0.0,1.0);

	vec4 textureColor = texture(diffuseMap, outTexCoord);

	FragColor = (ambient + diffuse) * textureColor;
}