#shader vertex
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aTangent;
layout (location = 3) in vec2 aTextureCoords;

out vec4 outPos;
out vec3 outNormal;
out vec3 outDirToLight;
out vec3 outDirToView;
out vec2 outTexCoord;

struct TransformBuffer
{
	mat4 World;
	mat4 WVP;
	vec3 ViewPosition;
};

struct LightBuffer 
{
	vec3 direction;
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
};

uniform sampler2D displacementMap;
uniform TransformBuffer transform;
uniform LightBuffer light;

void main()
{
	float displacement =  texture(displacementMap, aTextureCoords).x;
	vec3 localPosition = aPos+ (aNormal* displacement * 0.0f);
	vec3 worldPosition = (vec4(localPosition, 1.0)* transform.World).xyz;
	vec3 worldNormal = aNormal * mat3x3(transform.World);

	outPos = vec4(localPosition, 1.0)* transform.World;
	outNormal = worldNormal;
	outDirToLight = light.direction;
	outDirToView = normalize(transform.ViewPosition - worldPosition);
	outTexCoord = aTextureCoords;
	gl_Position = outPos;
}

#shader fragment
#version 330 core

struct TransformBuffer
{
	mat4 WVP;
	mat4 World;
	vec3 ViewPosition;
};

struct LightBuffer 
{
	vec3 direction;
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
};

struct MaterialBuffer 
{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float power;
}; 

uniform MaterialBuffer material;
uniform LightBuffer light;
uniform TransformBuffer transform;

in vec4 outPos;
in vec3 outNormal;
in vec3 outDirToLight;
in vec3 outDirToView;
in vec2 outTexCoord;

out vec4 FragColor;

uniform sampler2D diffuseMap;
uniform sampler2D specularMap;

void main()
{
	vec3 worldNormal = normalize(outNormal);
	vec3 dirToLight = normalize(outDirToLight);
	vec3 dirToView = normalize(outDirToView);

	//ambient
	vec4 ambient = light.ambient + material.ambient;

	//diffuse
	float diffuseIntensity = clamp(dot(dirToLight, worldNormal),0.0,1.0);
	vec4 diffuse = diffuseIntensity * light.diffuse * material.diffuse;

	//specular
	vec3 halfAngle = normalize(dirToLight + dirToView);
	float specularBase = clamp(dot(halfAngle, worldNormal), 0.0,1.0);
	float specularIntensity = pow(specularBase, material.power);
	vec4 specular = specularIntensity * light.specular * material.specular;

	vec4 mainTexture = texture(diffuseMap, outTexCoord);
	float specularFactor = texture(specularMap, outTexCoord).x;

	vec4 color = (ambient + diffuse) *mainTexture + specular * specularFactor;

	FragColor = vec4(0.0f,0.0f,0.0f,0.0f);
}