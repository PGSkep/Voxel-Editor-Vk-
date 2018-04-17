#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;

layout(location = 0) out vec2 UV;

void main()
{
	gl_Position = vec4(vertexPosition_modelspace, 0.0, 1.0);
	UV = vec2(vertexUV);
}