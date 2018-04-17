#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform PushConstants
{
	uint modelMatrixIndex;
	float red;
	float green;
	float blue;
} pushConstants;

layout(set = 0, binding = 0) uniform ViewProjection
{
	mat4 view;
	mat4 projection;
} vp;
layout(set = 0, binding = 1) uniform ModelMatrices
{
	mat4 matrices[1];
} modelMatrices;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;

layout(location = 0) out vec2 outUV;
layout(location = 1) out vec3 outColor;
layout(location = 2) out vec3 outNormal;

layout(location = 5) out vec3 outFragPos;
layout(location = 8) out vec3 outViewForward;

void main()
{
	gl_Position = vp.projection * vp.view * modelMatrices.matrices[pushConstants.modelMatrixIndex] * vec4(inPosition, 1.0);
	outUV = vec2(inUV);
	outColor = vec3(pushConstants.red, pushConstants.blue, pushConstants.green);

	outNormal = inNormal;
	outFragPos = vec3(modelMatrices.matrices[pushConstants.modelMatrixIndex] * vec4(inPosition, 1.0f));
	
	outViewForward = vec3(vp.view[0][2], vp.view[1][2], vp.view[2][2]);
}