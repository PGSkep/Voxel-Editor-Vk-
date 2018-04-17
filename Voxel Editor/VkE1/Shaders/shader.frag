#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 2) uniform sampler2D texSampler;

layout(location = 0) in vec2 inUV;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;

layout(location = 5) in vec3 inFragPos;
layout(location = 8) in vec3 inViewForward;

layout(location = 0) out vec4 outColor;

vec4 CalculateLight(vec3 position, vec3 color)
{
// to be external
	float diffuseStrenght = 1.0;
	float specularStrength = 1.5;

	float specularRoughness = 32;
	
	vec3 diffuseColor = color;
	vec3 specularColor = vec3(1.0, 0.0, 0.0);

// precalc
	vec3 norm = normalize(inNormal);
	vec3 lightDir = normalize(position - inFragPos);
	vec3 viewDir = normalize(inViewForward);
	vec3 reflectDir = reflect(-lightDir, norm);

	vec3 diffuse = max(dot(norm, lightDir), 0.0) * diffuseColor * diffuseStrenght;
	vec3 specular = pow(max(dot(viewDir, reflectDir), 0.0), specularRoughness) * color * specularStrength;

	return vec4(diffuse + specular, 1.0f);
}

void main()
{
	//outColor = texture(texSampler, inUV) * 0.0 + vec4(inColor, 1.0) * 1.0;
	
	vec3 position = vec3(15.0f, 15.0f, -15.0f);
	vec4 ambient = vec4(inColor, 0.0f) * 0.1;

	outColor = CalculateLight(position, inColor) + ambient;
}